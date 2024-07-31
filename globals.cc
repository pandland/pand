#include <v8.h>
#include <iostream>

namespace runtime {

class Globals {
  v8::Local<v8::ObjectTemplate> global;
  v8::Isolate* isolate;
public:
  Globals(v8::Local<v8::ObjectTemplate> global, v8::Isolate* isolate): global(global), isolate(isolate) {}

  void setup() {
    global->Set(isolate, "println", v8::FunctionTemplate::New(isolate, println));
    global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, print));
  }

  static void println(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) return;
    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    std::cout << *str << std::endl;
  }

  static void print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) return;
    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    std::cout << *str;
  }
};

}
