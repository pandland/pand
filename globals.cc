#include <v8.h>
#include <assert.h>
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
    global->Set(isolate, "env", v8::FunctionTemplate::New(isolate, env));
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

  static void env(const v8::FunctionCallbackInfo<v8::Value>& args) {
    assert(args.Length() == 1);
    v8::Isolate *isolate = args.GetIsolate();

    if (!args[0]->IsString()) {
      isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Argument must be a string", v8::NewStringType::kNormal).ToLocalChecked());
      return;
    }

    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *value = std::getenv(*str);


    if (!value) {
       args.GetReturnValue().Set(v8::Null(isolate));
       return;
    }

    return args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, value).ToLocalChecked());
  }
};

}
