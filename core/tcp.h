#pragma once
#include "pand.h"
#include <pandio.h>
#include <v8.h>

namespace pand::core {

class TcpStream {
public:
  pd_tcp_t handle;
  v8::Persistent<v8::Object> obj;

  TcpStream(v8::Local<v8::Object> obj) {
    Pand *pand = Pand::get();
    pd_tcp_init(pand->ctx, &this->handle);
    this->handle.data = this;
    this->handle.on_close = TcpStream::onClose;
    this->handle.on_data = TcpStream::onData;
    this->obj.Reset(obj->GetIsolate(), obj);
  }

  ~TcpStream() { this->obj.Reset(); }

  static void initialize(v8::Local<v8::Object>);

  static void setNoDelay(const v8::FunctionCallbackInfo<v8::Value> &);

  static void setKeepAlive(const v8::FunctionCallbackInfo<v8::Value> &);

  static void constructor(const v8::FunctionCallbackInfo<v8::Value> &);

  static void connect(const v8::FunctionCallbackInfo<v8::Value> &);

  static void onConnect(pd_tcp_t *, int);

  static void onData(pd_tcp_t *, char *, size_t);

  static void onClose(pd_tcp_t *);
};

} // namespace pand::core
