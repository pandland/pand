#include "pand.h"
#include <pandio.h>
#include <v8.h>

namespace pand::core {

class TcpStream {
public:
  enum Type { SERVER, CLIENT };
  pd_tcp_t handle;
  v8::Persistent<v8::Object> obj;
  TcpStream::Type type;

  TcpStream(v8::Local<v8::Object> obj) {
    Pand *pand = Pand::get();
    pd_tcp_init(pand->ctx, &this->handle);
    this->handle.data = this;
    this->obj.Reset(obj->GetIsolate(), obj);
  }

  ~TcpStream() { this->obj.Reset(); }

  static void initialize(v8::Local<v8::Object> exports);

  static void setNoDelay(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void setKeepAlive(const v8::FunctionCallbackInfo<v8::Value> &args);
};

} // namespace pand::core
