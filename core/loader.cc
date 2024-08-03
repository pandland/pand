#pragma once

#include <swc_transform.h>
#include <string>
#include <sstream>
#include <fstream>

#include <assert.h>
#include <v8.h>

namespace runtime {

#define MODULES_DIR "./js"

class Loader {
public:
  void execute(v8::Isolate* isolate, v8::Local<v8::Context> context) {
    std::ifstream file(MODULES_DIR "/bootstrap.js");
    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string src = buffer.str();

    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, src.c_str()).ToLocalChecked();
    v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

    script->Run(context).ToLocalChecked();
  }

  static void load(const v8::FunctionCallbackInfo<v8::Value> &args) {
      assert(args.Length() == 1);
      assert(args[0]->IsString());

      v8::Isolate *isolate = args.GetIsolate();
      //v8::Local<v8::Context> context = v8::Context::New(isolate);
      //v8::Context::Scope context_scope(context);
      v8::Local<v8::Context> context = isolate->GetCurrentContext();
      
      v8::String::Utf8Value filename(isolate, args[0]);
      
      printf("Loading file: %s\n", *filename);

      std::ifstream file(*filename);
      std::stringstream buffer;
      buffer << "(function (exports, require, module, __filename, __dirname) { ";
      buffer << file.rdbuf();
      buffer << "\n})";
      
      std::string src = buffer.str();
      src = (char *)transform_sync((unsigned char *)src.c_str(), src.length());
      v8::ScriptOrigin origin(v8::String::NewFromUtf8(isolate, *filename).ToLocalChecked());
      v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, src.c_str()).ToLocalChecked();
      v8::Local<v8::Script> script = v8::Script::Compile(context, source, &origin).ToLocalChecked();
      
      v8::Local<v8::Value> func = script->Run(context).ToLocalChecked();
      
      if (!func->IsFunction()) {
        printf("Invalid code:\n %s\n", src.c_str());
        printf("Invalid module!\n");
      }

      args.GetReturnValue().Set(func);
  }
};

}
