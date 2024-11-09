#include "fs.h"
#include "pand.h"
#include "errors.h"
#include "buffer.h"
#include <iostream>

namespace pand::core {

struct FileOperation {
  File *file;
  v8::Persistent<v8::Promise::Resolver> resolver;
  pd_fs_t op{};

  FileOperation(pd_io_t *ctx, File *file): file(file) {
    pd_fs_init(ctx, &op);
    op.udata = this;
  }

  virtual ~FileOperation() {
      resolver.Reset();
  }

  void setResolver(v8::Local<v8::Promise::Resolver> value) {
    this->resolver.Reset(value->GetIsolate(), value);
  }
};

struct ReadFileOperation : FileOperation {
  v8::Persistent<v8::Value> buffer;

  using FileOperation::FileOperation;

  ~ReadFileOperation() override {
    buffer.Reset();
  }
};

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

  int32_t flags = args[1]->Int32Value(context).ToChecked();
  v8::String::Utf8Value path(isolate, args[0]);
  pd_io_t *ctx = Pand::get()->ctx;

  auto file = static_cast<File *>(args.This()->GetAlignedPointerFromInternalField(0));
  auto *openOp = new FileOperation(ctx, file);
  openOp->setResolver(resolver);

  // pd_fs_open copies data from path - we DO NOT transfer ownership here
  pd_fs_open(&openOp->op, *path, flags, File::onOpen);

  args.GetReturnValue().Set(resolver->GetPromise());
}

void File::onOpen(pd_fs_t *op) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  auto *openOp = static_cast<FileOperation *>(op->udata);
  auto resolver = openOp->resolver.Get(isolate);
  if (op->status < 0) {
    auto err = Pand::makeSystemError(isolate, op->status);
    resolver->Reject(context, err).ToChecked();
    delete openOp;

    return;
  }

  openOp->file->isClosed = false;
  openOp->file->fd = op->result.fd;
  resolver->Resolve(context, Pand::value(isolate, op->result.fd)).ToChecked();
  delete openOp;
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
  auto *readOp = new ReadFileOperation(ctx, file);
  readOp->buffer.Reset(isolate, buffer);
  readOp->setResolver(resolver);

  char *data = Buffer::getBytes(buffer);
  size_t size = Buffer::getSize(buffer);

  pd_fs_read(&readOp->op, file->fd, data, size, File::onRead);

  args.GetReturnValue().Set(resolver->GetPromise());
}

void File::onRead(pd_fs_t *op) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  auto *readOp = static_cast<ReadFileOperation *>(op->udata);
  auto resolver = readOp->resolver.Get(isolate);
  if (op->status < 0) {
    auto err = Pand::makeSystemError(isolate, op->status);
    resolver->Reject(context, err).ToChecked();
    delete readOp;

    return;
  }

  resolver->Resolve(context, Pand::integer(isolate, op->result.size)).ToChecked();
  delete readOp;
}

void File::close(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
  pd_io_t *ctx = Pand::get()->ctx;

  auto file = static_cast<File *>(args.This()->GetAlignedPointerFromInternalField(0));
  auto *closeOp = new FileOperation(ctx, file);
  closeOp->setResolver(resolver);

  pd_fs_close(&closeOp->op, file->fd, File::onClose);

  args.GetReturnValue().Set(resolver->GetPromise());
}

void File::onClose(pd_fs_t *op) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  auto *closeOp = static_cast<FileOperation *>(op->udata);
  auto resolver = closeOp->resolver.Get(isolate);
  if (op->status < 0) {
    auto err = Pand::makeSystemError(isolate, op->status);
    resolver->Reject(context, err).ToChecked();
    delete closeOp;

    return;
  }
  closeOp->file->isClosed = true;
  resolver->Resolve(context, v8::Undefined(isolate)).ToChecked();
  delete closeOp;
}

}
