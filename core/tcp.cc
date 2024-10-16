#include "tcp.h"
#include "pandio/tcp.h"
#include "v8_utils.cc"

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
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

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
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  pd_tcp_shutdown(&stream->handle);
}

void TcpStream::destroy(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  pd_tcp_close(&stream->handle);
}

void TcpStream::pause(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  pd_tcp_pause(&stream->handle);
}

void TcpStream::resume(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpStream *stream = static_cast<TcpStream *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  pd_tcp_resume(&stream->handle);
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

void TcpStream::onData(pd_tcp_t *handle, char *buf, size_t size) {
  printf("Received data with len: %zu\n", size);
  printf("%.*s\n", (int)size, buf);
  free(buf);
  pd_tcp_close(handle);
}

void TcpStream::onClose(pd_tcp_t *handle) {
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  delete stream;
}

} // namespace pand::core
