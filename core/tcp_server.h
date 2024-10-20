#pragma once
#include "pand.h"
#include <pandio.h>
#include <v8.h>

namespace pand::core {

class TcpServer {
public:
  TcpServer(v8::Local<v8::Object> obj) {
    Pand *pand = Pand::get();
    pd_tcp_server_init(pand->ctx, &this->handle);
    this->handle.data = this;
    this->obj.Reset(obj->GetIsolate(), obj);
  }

  ~TcpServer() { this->obj.Reset(); }

  pd_tcp_server_t handle;
  v8::Persistent<v8::Object> obj;

  static void initialize(v8::Local<v8::Object>);

  static void constructor(const v8::FunctionCallbackInfo<v8::Value> &);

  static void listen(const v8::FunctionCallbackInfo<v8::Value> &);

  static void close(const v8::FunctionCallbackInfo<v8::Value> &);

  static void onConnection(pd_tcp_server_t *server, pd_socket_t socket,
                           int status);
};

} // namespace pand::core
