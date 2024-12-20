#include "fs.h"
#include "pand.h"
#include "errors.h"
#include "buffer.h"
#include <iostream>
#include <filesystem>

namespace pand::core {

struct FileOperation {
  File *file;
  v8::Persistent<v8::Promise::Resolver> resolver;
  v8::Persistent<v8::Object> handle;
  pd_fs_t op{};

  FileOperation(pd_io_t *ctx, File *file): file(file) {
    pd_fs_init(ctx, &op);
    op.udata = this;
  }

  virtual ~FileOperation() {
    handle.Reset();
    resolver.Reset();
  }

  void setObjectHandle(v8::Local<v8::Object> obj) {
      this->handle.Reset(obj->GetIsolate(), obj);
  }

  void setResolver(v8::Local<v8::Promise::Resolver> value) {
    this->resolver.Reset(value->GetIsolate(), value);
  }

  static void onDone(pd_fs_t *op) {
    Pand *pand = Pand::get();
    v8::Isolate *isolate = pand->isolate;
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    auto *wrap = static_cast<FileOperation *>(op->udata);
    if (op->type == pd_close_op)
      wrap->file->isClosing = false;

    auto resolver = wrap->resolver.Get(isolate);
    if (op->status < 0) {

      auto err = Pand::makeSystemError(isolate, op->status);
      resolver->Reject(context, err).ToChecked();
      delete wrap;
      return;
    }

    v8::Local<v8::Value> result;
    pd_fs_type_t type = op->type;

    switch (type) {
    case pd_open_op:
      wrap->file->fd = op->result.fd;
      result = v8::Integer::New(isolate, op->result.fd);
      break;
    case pd_read_op:
    case pd_write_op:
      result = v8::Integer::New(isolate, int(op->result.size));
      break;
    case pd_close_op:
      wrap->file->isClosed = true;
    case pd_unknown_op:
      result = v8::Undefined(isolate);
      break;
    }

    resolver->Resolve(context, result).ToChecked();
    delete wrap;
  }
};

struct BufferedFileOperation : FileOperation {
  v8::Persistent<v8::Value> buffer;

  using FileOperation::FileOperation;

  ~BufferedFileOperation() override {
    buffer.Reset();
  }

  void setBuffer(v8::Local<v8::Uint8Array> buf) {
    this->buffer.Reset(buf->GetIsolate(), buf);
  }
};

void File::onCleanup(const v8::WeakCallbackInfo<File> &param) {
  File *file = param.GetParameter();
  // TODO: close open fd
  delete file;
}

void File::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, File::constructor);

  t->SetClassName(Pand::symbol(isolate, "File"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  v8::Local<v8::FunctionTemplate> openT =
      v8::FunctionTemplate::New(isolate, File::open);
  t->PrototypeTemplate()->Set(isolate, "open", openT);

  v8::Local<v8::FunctionTemplate> readT =
      v8::FunctionTemplate::New(isolate, File::read);
  t->PrototypeTemplate()->Set(isolate, "read", readT);

  v8::Local<v8::FunctionTemplate> writeT =
      v8::FunctionTemplate::New(isolate, File::write);
  t->PrototypeTemplate()->Set(isolate, "write", writeT);

  v8::Local<v8::FunctionTemplate> closeT =
      v8::FunctionTemplate::New(isolate, File::close);
  t->PrototypeTemplate()->Set(isolate, "close", closeT);

  v8::Local<v8::Function> func = t->GetFunction(context).ToLocalChecked();
  exports
      ->Set(context, v8::String::NewFromUtf8(isolate, "File").ToLocalChecked(),
            func)
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "O_RDONLY"),
            Pand::integer(isolate, PD_FS_O_RDONLY))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "O_WRONLY"),
            Pand::integer(isolate, PD_FS_O_WRONLY))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "O_CREAT"),
            Pand::integer(isolate, PD_FS_O_CREAT))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "O_APPEND"),
            Pand::integer(isolate, PD_FS_O_APPEND))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "O_EXCL"),
            Pand::integer(isolate, PD_FS_O_EXCL))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "O_RDWR"),
            Pand::integer(isolate, PD_FS_O_RDWR))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "O_TRUNC"),
            Pand::integer(isolate, PD_FS_O_TRUNC))
      .ToChecked();
}

void File::constructor(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  new File(args.This());
}

void File::open(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  if (args.Length() < 2 ) {
    auto err = Errors::TypeError(isolate, "Expected 2 arguments");
    resolver->Reject(context, err).ToChecked();
    args.GetReturnValue().Set(resolver->GetPromise());
    return;
  }

  if (!args[0]->IsString()) {
    auto err = Errors::TypeError(isolate, "Expected path to be string");
    resolver->Reject(context, err).ToChecked();
    args.GetReturnValue().Set(resolver->GetPromise());
    return;
  }

  if (!args[1]->IsInt32()) {
    auto err = Errors::TypeError(isolate, "Open flags must be 32 bit integer");
    resolver->Reject(context, err).ToChecked();
    args.GetReturnValue().Set(resolver->GetPromise());
    return;
  }

  int mode = 0666;
  if (args.Length() == 3 && args[2]->IsInt32()) {
    mode = args[2]->Int32Value(context).ToChecked();
  }

  int32_t flags = args[1]->Int32Value(context).ToChecked();
  v8::String::Utf8Value pathstr(isolate, args[0]);
  pd_io_t *ctx = Pand::get()->ctx;

  auto file = static_cast<File *>(args.This()->GetAlignedPointerFromInternalField(0));
  auto *openOp = new FileOperation(ctx, file);
  openOp->setResolver(resolver);

  std::filesystem::path path(*pathstr);
  if (path.is_relative()) {
    path = std::filesystem::current_path() / path;
    path = path.lexically_normal();
  }
  // pd_fs_open copies data from path - we DO NOT transfer ownership here
  pd_fs_open(&openOp->op, path.c_str(), flags, mode, FileOperation::onDone);

  args.GetReturnValue().Set(resolver->GetPromise());
}

void File::read(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  if (args.Length() < 1 && Buffer::isBuffer(args[0])) {
    auto err = Errors::TypeError(isolate, "Expected buffer to be <Buffer> or <Uint8Array>");
    resolver->Reject(context, err).ToChecked();
    args.GetReturnValue().Set(resolver->GetPromise());
  }

  pd_io_t *ctx = Pand::get()->ctx;

  v8::Local<v8::Uint8Array> buffer = args[0].As<v8::Uint8Array>();
  auto file = static_cast<File *>(args.This()->GetAlignedPointerFromInternalField(0));
  auto *readOp = new BufferedFileOperation(ctx, file);
  readOp->setBuffer(buffer);;
  readOp->setObjectHandle(args.This());
  readOp->setResolver(resolver);

  char *data = Buffer::getBytes(buffer);
  size_t size = Buffer::getSize(buffer);

  pd_fs_read(&readOp->op, file->fd, data, size, BufferedFileOperation::onDone);

  args.GetReturnValue().Set(resolver->GetPromise());
}

void File::write(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  if (args.Length() < 1 && Buffer::isBuffer(args[0])) {
    auto err = Errors::TypeError(isolate, "Expected buffer to be <Buffer> or <Uint8Array>");
    resolver->Reject(context, err).ToChecked();
    args.GetReturnValue().Set(resolver->GetPromise());
  }

  pd_io_t *ctx = Pand::get()->ctx;

  v8::Local<v8::Uint8Array> buffer = args[0].As<v8::Uint8Array>();
  auto file = static_cast<File *>(args.This()->GetAlignedPointerFromInternalField(0));
  auto *writeOp = new BufferedFileOperation(ctx, file);
  writeOp->setBuffer(buffer);
  writeOp->setObjectHandle(args.This());
  writeOp->setResolver(resolver);

  char *data = Buffer::getBytes(buffer);
  size_t size = Buffer::getSize(buffer);

  pd_fs_write(&writeOp->op, file->fd, data, size, BufferedFileOperation::onDone);

  args.GetReturnValue().Set(resolver->GetPromise());
}

void File::close(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  pd_io_t *ctx = Pand::get()->ctx;

  auto file = static_cast<File *>(args.This()->GetAlignedPointerFromInternalField(0));
  auto *closeOp = new FileOperation(ctx, file);
  closeOp->setObjectHandle(args.This());
  closeOp->setResolver(resolver);

  file->isClosing = true;
  pd_fs_close(&closeOp->op, file->fd, FileOperation::onDone);

  args.GetReturnValue().Set(resolver->GetPromise());
}

}
