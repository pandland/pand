#include <v8.h>
#include <luxio.h>
#include <assert.h>

#include "io.cc"

namespace runtime {

class TcpServer {
  v8::Global<v8::Function> callback;
  int port;
public:
  static void initialize(v8::Local<v8::ObjectTemplate> exports, v8::Isolate *isolate) {
    exports->Set(isolate, "tcpListen", v8::FunctionTemplate::New(isolate, listen));
  }

  static void listen(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.Length() == 2);
    
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Function> callback = args[0].As<v8::Function>();
    int port = args[1]->IntegerValue(context).ToChecked();

    TcpServer *server = new TcpServer;
    server->callback.Reset(isolate, callback);
    server->port = port;

    lx_listener_t *l = lx_listen(IO::get()->ctx, port, TcpServer::handle_accept);
    l->data = server;
  }

  static void handle_accept(lx_connection_t *conn) {
    TcpServer *server = static_cast<TcpServer*>(conn->listener->data);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Function> callback = server->callback.Get(isolate);

    if (callback.IsEmpty()) {
      lx_close(conn);
      return;
    }

    callback->Call(context, v8::Undefined(isolate), 0, {}).ToLocalChecked();
    lx_close(conn);
  }
};

}
