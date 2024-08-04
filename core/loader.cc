#pragma once

#include <swc_transform.h>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <unordered_map>

#include <assert.h>
#include <v8.h>
#include "v8_utils.cc"
#include "js_internals.hh"

namespace fs = std::filesystem;

namespace runtime
{
  std::unordered_map<int, std::string> absolute_paths;

  class Loader
  {
  public:
    // path is always supposed to be absolute
    void execute(v8::Isolate *isolate, v8::Local<v8::Context> context, std::string path) {
      std::ifstream file(path);
      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string src = buffer.str();

      v8::Local<v8::String> source = v8_value(isolate, src);
      v8::ScriptOrigin origin(v8_value(isolate, path),
                              0,
                              0,
                              true,
                              -1,
                              v8::Local<v8::Value>(),
                              false,
                              false,
                              true);

      v8::ScriptCompiler::Source script_source(source, origin);
      v8::Local<v8::Module> mod = v8::ScriptCompiler::CompileModule(isolate, &script_source).ToLocalChecked();
      
      absolute_paths.emplace(mod->ScriptId(), path);
      mod->InstantiateModule(context, Loader::es6_import).IsJust();
      mod->Evaluate(context).ToLocalChecked();
    }

    // Not implemented yet!
    static v8::MaybeLocal<v8::Promise> resolve(
        v8::Local<v8::Context> context,
        v8::Local<v8::Data> host_defined_options,
        v8::Local<v8::Value> resource_name,
        v8::Local<v8::String> specifier,
        v8::Local<v8::FixedArray> import_attributes)
    {
      v8::Isolate *isolate = context->GetIsolate();
      v8::EscapableHandleScope handle_scope(isolate);
      v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

      resolver->Reject(context, v8::Exception::Error(v8::String::NewFromUtf8Literal(isolate, "Module not found"))).Check();

      return handle_scope.Escape(resolver->GetPromise());
    }

    static void set_meta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta) {}

    static v8::MaybeLocal<v8::Module> es6_import(
        v8::Local<v8::Context> context,
        v8::Local<v8::String> specifier,
        v8::Local<v8::FixedArray> import_assertions,
        v8::Local<v8::Module> referrer)
    {
      v8::Isolate *isolate = context->GetIsolate();
      v8::String::Utf8Value specifier_utf8(isolate, specifier);
      
      std::string specifier_name(*specifier_utf8);

      auto result = absolute_paths.find(referrer->ScriptId());
      if (result == absolute_paths.end()) {
        printf("error: Unable to find referer's path\n");
        return v8::MaybeLocal<v8::Module>();
      }
      
      fs::path referer_path = result->second;
      fs::path abs_path = referer_path.parent_path() / fs::path(specifier_name);
      abs_path = abs_path.lexically_normal();

      std::string path = abs_path.string();

      std::ifstream file(path);
      std::stringstream buffer;
      buffer << file.rdbuf();
      std::string src = buffer.str();

      v8::Local<v8::String> source = v8_value(isolate, src);
      v8::ScriptOrigin origin(v8_value(isolate, path),
                              0,
                              0,
                              true,
                              -1,
                              v8::Local<v8::Value>(),
                              false,
                              false,
                              true);

      v8::ScriptCompiler::Source script_source(source, origin);
      v8::Local<v8::Module> module;
      
      if (v8::ScriptCompiler::CompileModule(isolate, &script_source).ToLocal(&module)) {
        absolute_paths.emplace(module->ScriptId(), path);
        return module;
      }

      return v8::MaybeLocal<v8::Module>();
    }
  };
}
