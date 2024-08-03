#include <unordered_map>
#include <v8.h>
#include <assert.h>
#include <string>

#include "tcp.cc"

namespace runtime {
  static void bind(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.Length() == 1);
    assert(args[0]->IsString());

    v8::Isolate *isolate = args.GetIsolate();
    v8::String::Utf8Value str(isolate, args[0]);
    std::string name = *str;

    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    // TODO: create internal modules registry, create some export macro
    if (name == "tcp") {
      TcpStream::initialize(obj, isolate, isolate->GetCurrentContext());
      TcpServer::initialize(obj, isolate, isolate->GetCurrentContext());
    }

    args.GetReturnValue().Set(obj);
  }
}
