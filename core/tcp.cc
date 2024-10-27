#include "tcp.h"
#include "pandio/tcp.h"
#include <buffer.h>
#include <errors.h>
#include <memory>
#include <writer.h>

namespace pand::core {

void TcpStream::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, TcpStream::constructor);

  t->SetClassName(Pand::symbol(isolate, "TcpStream"));
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
  pand->setTcpStreamConstructor(func);
  exports->Set(context, Pand::symbol(isolate, "TcpStream"), func).ToChecked();
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
  if (!stream)
    return;
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

  if (args.Length() < 1 || !args[0]->IsArrayBuffer()) {
    Errors::throwTypeException(isolate, "Expected <ArrayBuffer>");
  }
  
  auto ab = args[0].As<v8::ArrayBuffer>();
  char *payload = static_cast<char*>(ab->Data());
  Writer *writer = new Writer(payload, ab->ByteLength());
  writer->setBuffer(isolate,  ab);
  pd_tcp_write(&stream->handle, &writer->op);
}

// callback handlers:
void TcpStream::onConnect(pd_tcp_t *handle, int status) {
  Pand *pand = Pand::get();
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> obj = stream->obj.Get(isolate);

  if (status < 0) {
    v8::Local<v8::Value> error = Pand::makeSystemError(isolate, status);
    v8::Local<v8::Value> argv[1] = {error};
    pd_tcp_close(handle);
    Pand::makeCallback(obj, isolate, "onError", argv, 1);
    return;
  }

  Pand::makeCallback(obj, isolate, "onConnect", {}, 0);
}

void *TcpStream::readAllocator(pd_tcp_t *handle, size_t size) {
  Pand *pand = Pand::get();
  auto bs = v8::ArrayBuffer::NewBackingStore(
      pand->isolate, size, v8::BackingStoreInitializationMode::kUninitialized);
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  stream->read_bs = std::move(bs);

  return stream->read_bs->Data();
}

void TcpStream::onData(pd_tcp_t *handle, char *buffer, size_t size) {
  Pand *pand = Pand::get();
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, std::move(stream->read_bs));
  v8::Local<v8::Value> argv[2] = {ab, Pand::integer(isolate, size)};
  v8::Local<v8::Object> obj = stream->obj.Get(isolate);
  Pand::makeCallback(obj, isolate, "onData", argv, 2);
}

void TcpStream::onClose(pd_tcp_t *handle) {
  Pand *pand = Pand::get();
  TcpStream *stream = static_cast<TcpStream *>(handle->data);
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> obj = stream->obj.Get(isolate);
  Pand::makeCallback(obj, isolate, "onClose", {}, 0);
  obj->SetAlignedPointerInInternalField(0, nullptr);

  delete stream;
}

} // namespace pand::core
