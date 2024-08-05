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
  std::unordered_map<std::string, v8::Global<v8::Module>> resolve_cache;

  class Loader
  {
  public:
    ~Loader() {
      Loader::clear_resolve_cache();
    }
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
      resolve_cache[path].Reset(isolate, mod);

      bool intialized = mod->InstantiateModule(context, Loader::load).IsJust();

      v8::TryCatch try_catch(isolate);
      v8::MaybeLocal<v8::Value> result = mod->Evaluate(context);

      if (result.IsEmpty()) {
          v8::String::Utf8Value error(isolate, try_catch.Exception());
          printf("Exception: %s\n", *error);
          return;
      }

      v8::Local<v8::Value> value = result.ToLocalChecked();
      if (value->IsPromise()) {
          v8::Local<v8::Promise> promise = value.As<v8::Promise>();
          if (promise->State() == v8::Promise::kRejected) {
              v8::String::Utf8Value error(isolate, promise->Result());
              printf("%s\n", *error);
          }
      }
    }

    // Not implemented yet!
    static v8::MaybeLocal<v8::Promise> dynamic_load(
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

    static void set_meta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta) {
      auto result = absolute_paths.find(module->ScriptId());
      if (result != absolute_paths.end()) {
        v8::Isolate *isolate = context->GetIsolate();
        meta->Set(context, v8_symbol(isolate, "url"), v8_value(isolate, "file//" + result->second)).Check();
      }
    }

    static v8::MaybeLocal<v8::Module> load(
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

      // load from cache:
      auto cached_module = resolve_cache.find(path);
      if (cached_module != resolve_cache.end()) {
        return cached_module->second.Get(isolate);
      }

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
        resolve_cache[path].Reset(isolate, module);
        return module;
      }

      printf("Error: something went wrong :o\n");
      return v8::MaybeLocal<v8::Module>();
    }

    static void clear_resolve_cache() {
      for (auto& entry : resolve_cache) {
        entry.second.Reset();
      }
      resolve_cache.clear();
    }
  };
}
