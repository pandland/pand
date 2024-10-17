#include "tcp_server.h"
#include <cstdio>
#include <v8-value.h>

namespace pand::core {

void TcpServer::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, TcpServer::constructor);

  t->SetClassName(Pand::symbol(isolate, "TcpServer"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  v8::Local<v8::FunctionTemplate> listenT =
      v8::FunctionTemplate::New(isolate, TcpServer::listen);
  t->PrototypeTemplate()->Set(isolate, "listen", listenT);

  v8::Local<v8::FunctionTemplate> closeT =
      v8::FunctionTemplate::New(isolate, TcpServer::close);
  t->PrototypeTemplate()->Set(isolate, "close", closeT);

  v8::Local<v8::Function> func = t->GetFunction(context).ToLocalChecked();
  exports->Set(context, Pand::symbol(isolate, "TcpServer"), func).ToChecked();
}

void TcpServer::constructor(const v8::FunctionCallbackInfo<v8::Value> &args) {
  assert(args.IsConstructCall());
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  TcpServer *server = new TcpServer(args.This());
  args.This()->SetAlignedPointerInInternalField(0, server);
}

void TcpServer::listen(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpServer *server = static_cast<TcpServer *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  if (!server)
    return;

  int port = args[0]->Int32Value(context).FromMaybe(0);
  // TODO: it would be cool to handle pandio errrors...
  pd_tcp_listen(&server->handle, port, TcpServer::onConnection);
}

void TcpServer::close(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  TcpServer *server = static_cast<TcpServer *>(
      args.This()->GetAlignedPointerFromInternalField(0));
  if (!server)
    return;

  int status = pd_tcp_server_close(&server->handle);
  printf("status: %d\n", status);
  server->obj.Get(isolate)->SetAlignedPointerInInternalField(0, nullptr);
  delete server;
}

void TcpServer::onConnection(pd_tcp_server_t *handle, pd_socket_t socket,
                             int status) {
  Pand *pand = Pand::get();
  TcpServer *server = static_cast<TcpServer *>(handle->data);
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  auto constructor = pand->getTcpStreamConstructor();
  v8::Local<v8::Object> streamInstance =
      constructor->NewInstance(context).ToLocalChecked();
  TcpStream *stream = static_cast<TcpStream *>(
      streamInstance->GetAlignedPointerFromInternalField(0));
  // TODO: make checks
  pd_tcp_accept(&stream->handle, socket);

  v8::Local<v8::Object> obj = server->obj.Get(isolate);
  v8::Local<v8::Value> argv[1] = {streamInstance};
  Pand::makeCallback(obj, isolate, "onConnection", argv, 1);
}

} // namespace pand::core
