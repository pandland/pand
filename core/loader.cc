#pragma once

#include <swc_transform.h>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <assert.h>
#include <v8.h>
#include "v8_utils.cc"
#include "js_internals.hh"

namespace fs = std::filesystem;

namespace runtime {

class Loader {
public:
  void execute(v8::Isolate* isolate, v8::Local<v8::Context> context) {
    auto result = js_internals.find("bootstrap");
    if (result == js_internals.end()) {
      printf("Unexpected error - unable to find entrypoint JS file - you forgot to generate headers?\n");
      return;
    }

    std::string src = result->second;
    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, src.c_str()).ToLocalChecked();
    v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

    script->Run(context).ToLocalChecked();
  }

  static void load(const v8::FunctionCallbackInfo<v8::Value> &args) {
      assert(args.Length() == 1);
      assert(args[0]->IsString());

      v8::Isolate *isolate = args.GetIsolate();
      v8::HandleScope handle_scope(isolate);
      //v8::Local<v8::Context> context = v8::Context::New(isolate);
      //v8::Context::Scope context_scope(context);
      v8::Local<v8::Context> context = isolate->GetCurrentContext();
      
      v8::String::Utf8Value filepath(isolate, args[0]);

      fs::path path = fs::path(*filepath);
      std::string __dirname = path.parent_path().string();
      std::string __filename = path.filename().string();

      std::ifstream file(*filepath);
      std::stringstream buffer;
      buffer << "(function (exports, require, module, __filename, __dirname) { ";
      buffer << file.rdbuf();
      buffer << "\n})";
      
      std::string src = buffer.str();
      src = (char *)transform_sync((unsigned char *)src.c_str(), src.length());
      v8::ScriptOrigin origin(v8::String::NewFromUtf8(isolate, *filepath).ToLocalChecked());
      v8::Local<v8::String> source = v8_value(isolate, src);
      v8::Local<v8::Script> script = v8::Script::Compile(context, source, &origin).ToLocalChecked();
      
      v8::Local<v8::Object> mod = v8::Object::New(isolate);
      v8::Local<v8::Value> func = script->Run(context).ToLocalChecked();
      
      mod->Set(
        context, 
        v8_symbol(isolate, "__dirname"),
        v8_value(isolate, __dirname)
      ).ToChecked();

      mod->Set(
        context, 
        v8_symbol(isolate, "__filename"),
        v8_value(isolate, __filename)
      ).ToChecked();

      mod->Set(context, v8::String::NewFromOneByte(isolate, (const uint8_t*)"func").ToLocalChecked(), func).ToChecked();

      if (!func->IsFunction()) {
        printf("Invalid code:\n %s\n", src.c_str());
        printf("Invalid module!\n");
        return;
      }

      args.GetReturnValue().Set(mod);
  }
};
}
