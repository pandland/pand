#include "tcp.h"
#include "errors.h"
#include "v8_utils.cc"
#include <v8-isolate.h>
#include <v8-local-handle.h>
#include <v8-object.h>
#include <v8-value.h>

namespace pand::core {

void TcpStream::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, TcpStream::constructor);

  t->SetClassName(v8_symbol(isolate, "TcpStream"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  // set proto functions:
  v8::Local<v8::FunctionTemplate> setKeepAliveT =
      v8::FunctionTemplate::New(isolate, TcpStream::setKeepAlive);
  t->PrototypeTemplate()->Set(isolate, "setKeepAlive", setKeepAliveT);

  v8::Local<v8::FunctionTemplate> setNoDelayT =
      v8::FunctionTemplate::New(isolate, TcpStream::setNoDelay);
  t->PrototypeTemplate()->Set(isolate, "setNoDelay", setNoDelayT);

  v8::Local<v8::FunctionTemplate> connectT =
      v8::FunctionTemplate::New(isolate, TcpStream::connect);
  t->PrototypeTemplate()->Set(isolate, "connect", connectT);

  v8::Local<v8::FunctionTemplate> shutdownT =
      v8::FunctionTemplate::New(isolate, TcpStream::shutdown);
  t->PrototypeTemplate()->Set(isolate, "shutdown", shutdownT);

  v8::Local<v8::FunctionTemplate> destroyT =
      v8::FunctionTemplate::New(isolate, TcpStream::destroy);
  t->PrototypeTemplate()->Set(isolate, "destroy", destroyT);

  v8::Local<v8::FunctionTemplate> pauseT =
      v8::FunctionTemplate::New(isolate, TcpStream::pause);
  t->PrototypeTemplate()->Set(isolate, "pause", pauseT);

  v8::Local<v8::FunctionTemplate> resumeT =
      v8::FunctionTemplate::New(isolate, TcpStream::resume);
  t->PrototypeTemplate()->Set(isolate, "resume", resumeT);

  v8::Local<v8::FunctionTemplate> writeT =
      v8::FunctionTemplate::New(isolate, TcpStream::write);
  t->PrototypeTemplate()->Set(isolate, "write", writeT);

  v8::Local<v8::Function> func = t->GetFunction(context).ToLocalChecked();
  exports
      ->Set(context,
            v8::String::NewFromUtf8(isolate, "TcpStream").ToLocalChecked(),
            func)
      .ToChecked();
}

void TcpStream::constructor(const v8::FunctionCallbackInfo<v8::Value> &args) {
  assert(args.IsConstructCall());
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  TcpStream *stream = new TcpStream(args.This());
  args.This()->SetAlignedPointerInInternalField(0, stream);
}

// JS land API:
void TcpStream::setKeepAlive(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));

  bool enable = args[0]->BooleanValue(isolate);
  int64_t delay = args[1]->IntegerValue(context).FromMaybe(0);
  pd_tcp_keepalive(&stream->handle, enable, delay);
}

void TcpStream::setNoDelay(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));

  bool enable = args[0]->BooleanValue(isolate);
  pd_tcp_nodelay(&stream->handle, enable);
}

void TcpStream::connect(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));

  v8::String::Utf8Value hostname(isolate, args[0]);
  int port = args[1]->Int32Value(context).ToChecked();

  pd_tcp_connect(&stream->handle, *hostname, port, TcpStream::onConnect);
}

void TcpStream::shutdown(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  if (!stream)
    return;
  pd_tcp_shutdown(&stream->handle);
}

void TcpStream::destroy(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  if (!stream)
    return;

  pd_tcp_close(&stream->handle);
}

void TcpStream::pause(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  if (!stream)
    return;

  pd_tcp_pause(&stream->handle);
}

void TcpStream::resume(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  if (!stream)
    return;

  pd_tcp_resume(&stream->handle);
}

void TcpStream::write(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  if (!stream)
    return;

  // TODO: we do not have Buffer class yet, so we operate on strings...
  v8::Local<v8::String> data = args[0]->ToString(context).ToLocalChecked();
  size_t size = data->Utf8Length(isolate);
  char *buffer = new char[size];
  data->WriteUtf8(isolate, buffer, size, nullptr,
                  v8::String::NO_NULL_TERMINATION);

  pd_write_t *op = new pd_write_t;
  pd_write_init(op, buffer, size, TcpStream::onWrite);
  pd_tcp_write(&stream->handle, op);
}

void TcpStream::makeCallback(v8::Local<v8::Object> &obj, v8::Isolate *isolate,
                             const char *funcName, v8::Local<v8::Value> *argv,
                             size_t argc) {
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Function> callback =
      obj->Get(context, v8_symbol(isolate, funcName))
          .ToLocalChecked()
          .As<v8::Function>();

  if (callback.IsEmpty()) {
    return;
  }

  v8::TryCatch try_catch(isolate);
  auto result = callback->Call(context, v8::Undefined(isolate), argc, argv);
  if (try_catch.HasCaught()) {
    Errors::throwCritical(try_catch.Exception());
  }
}

// callback handlers:
void TcpStream::onConnect(pd_tcp_t *handle, int status) {
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  if (status < 0) {
    printf("Error code: %d.\n", status);
    printf("Name: %s\n", pd_errname(status));
    printf("Message: %s\n", pd_errstr(status));
    pd_tcp_close(handle);
  } else {
    printf("Connected.\n");
  }
}

void TcpStream::onData(pd_tcp_t *handle, char *buffer, size_t size) {
  Pand *pand = Pand::get();
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::String> data =
      v8::String::NewFromUtf8(isolate, buffer, v8::NewStringType::kNormal, size)
          .ToLocalChecked();
  v8::Local<v8::Value> argv = {data};
  v8::Local<v8::Object> obj = stream->obj.Get(isolate);
  TcpStream::makeCallback(obj, isolate, "onData", &argv, 1);

  // TODO: make ability to create own allocator in pandiolib
  free(buffer); // our pandio uses C allocators
}

void TcpStream::onWrite(pd_write_t *op, int status) {
  delete op->data.buf;
  delete op;
}

void TcpStream::onClose(pd_tcp_t *handle) {
  Pand *pand = Pand::get();
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> obj = stream->obj.Get(isolate);
  TcpStream::makeCallback(obj, isolate, "onClose", {}, 0);
  obj->SetAlignedPointerInInternalField(0, nullptr);

  delete stream;
}

} // namespace pand::core
