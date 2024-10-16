#include "tcp_server.h"
#include "v8_utils.cc"
#include <cstdio>

namespace pand::core {

void TcpServer::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, TcpServer::constructor);

  t->SetClassName(v8_symbol(isolate, "TcpServer"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  v8::Local<v8::FunctionTemplate> listenT =
      v8::FunctionTemplate::New(isolate, TcpServer::listen);
  t->PrototypeTemplate()->Set(isolate, "listen", listenT);

  v8::Local<v8::Function> func = t->GetFunction(context).ToLocalChecked();
  exports->Set(context, v8_symbol(isolate, "TcpServer"), func).ToChecked();
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

void TcpServer::onConnection(pd_tcp_server_t *server, pd_socket_t socket,
                             int status) {
  printf("Socket fd: %d\n", socket);
  // TODO: construct TcpStream and use pd_tcp_accept
}

} // namespace pand::core
