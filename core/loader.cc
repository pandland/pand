#include <ada.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <v8-message.h>

#include "js_internals.h"
#include "loader.h"
#include "v8_utils.cc"

#define STD_PREFIX "std:"

namespace runtime {

std::unordered_map<int, runtime::Mod> mods;
std::unordered_map<std::string, v8::Global<v8::Module>> resolve_cache;

Loader::~Loader() { Loader::clear_resolve_cache(); }

std::string Loader::resolve_module_path(fs::path parent,
                                        const std::string &specifier) {
  fs::path abs_path = parent.parent_path() / fs::path(specifier);
  return abs_path.lexically_normal().string();
}

std::string Loader::resolve_entry_path(fs::path parent,
                                       const std::string &specifier) {
  fs::path abs_path = parent / fs::path(specifier);
  return abs_path.lexically_normal().string();
}

std::string Loader::load_file(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    printf("error: Unable to load module: '%s'\n", path.c_str());
    exit(1);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::string Loader::load_internal(const std::string &name) {
  auto internal_src = js_internals.find(name);
  if (internal_src == js_internals.end()) {
    printf("error: Invalid std module name: '%s'\n", name.c_str());
    exit(1);
  }

  return std::string(internal_src->second.begin(), internal_src->second.end());
}

void Loader::report_details(v8::Local<v8::Message> msg,
                            v8::Local<v8::Context> context) {
  int target_line = msg->GetLineNumber(context).FromJust();
  int start = msg->GetStartColumn();

  auto result = mods.find(msg->GetScriptOrigin().ScriptId());
  if (result == mods.end()) {
    return;
  }

  runtime::Mod mod = result->second;

  std::stringstream ss(mod.content);
  std::string line;
  int current_line = 1;

  while (std::getline(ss, line)) {
    if (current_line == target_line) {
      std::cout << line << std::endl;

      if (start < line.length()) {
        std::cout << std::string(start, ' ') << "^" << std::endl;
      }
      break;
    }
    current_line++;
  }

  std::cout << "at " << Loader::path_to_url(mod.fullpath) << ":" << target_line
            << ":" << start << std::endl;
}

void Loader::start(v8::Isolate *isolate, v8::Local<v8::Context> context,
                   std::string path) {
  std::string entrypath = Loader::resolve_entry_path(fs::current_path(), path);

  Loader::execute(isolate, context, entrypath);
}

// execute script
void Loader::execute(v8::Isolate *isolate, v8::Local<v8::Context> context,
                     std::string path) {
  v8::HandleScope handle_scope(isolate);

  std::string content =
      Loader::is_internal(path) ? load_internal(path) : load_file(path);

  v8::Local<v8::String> source = v8_value(isolate, content);
  v8::ScriptOrigin origin(v8_value(isolate, path), 0, 0, true, -1,
                          v8::Local<v8::Value>(), false, false, true);

  v8::ScriptCompiler::Source script_source(source, origin);
  v8::Local<v8::Module> v8_mod =
      v8::ScriptCompiler::CompileModule(isolate, &script_source)
          .ToLocalChecked();

  // TODO: I think we want to limit copies of mod struct
  runtime::Mod mod = {.fullpath = path, .content = content};
  mods.emplace(v8_mod->ScriptId(), mod);
  resolve_cache[path].Reset(isolate, v8_mod);

  bool intialized = v8_mod->InstantiateModule(context, Loader::load).IsJust();
  if (!intialized) {
    printf("error: Unable to init module %s\n", path.c_str());
    exit(1);
  }

  v8::TryCatch try_catch(isolate);
  v8::MaybeLocal<v8::Value> result = v8_mod->Evaluate(context);

  if (result.IsEmpty()) {
    v8::String::Utf8Value error(isolate, try_catch.Exception());
    printf("error: %s\n", *error);
    exit(1);
  }

  v8::Local<v8::Value> value = result.ToLocalChecked();
  if (value->IsPromise()) {
    v8::Local<v8::Promise> promise = value.As<v8::Promise>();
    if (promise->State() == v8::Promise::kRejected) {
      v8::String::Utf8Value error(isolate, promise->Result());
      v8::Local<v8::Message> errmsg =
          v8::Exception::CreateMessage(isolate, promise->Result());
      printf("error: Uncaught (in promise) %s\n", *error);
      v8::String::Utf8Value errfile(isolate, errmsg->GetScriptResourceName());
      // printf("filename: %s\n", *errfile);
      Loader::report_details(errmsg, context);
    }
  }
}

bool Loader::is_internal(std::string specifier) {
  return specifier.starts_with(STD_PREFIX);
}

v8::MaybeLocal<v8::Module>
Loader::load(v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
             v8::Local<v8::FixedArray> import_assertions,
             v8::Local<v8::Module> referrer) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::String::Utf8Value specifier_utf8(isolate, specifier);

  std::string specifier_name(*specifier_utf8);

  if (Loader::is_internal(specifier_name)) {
    return create_module(isolate, specifier_name);
  }

  auto mod = mods.find(referrer->ScriptId());
  if (mod == mods.end()) {
    printf("error: Unable to find referrer's path\n");
    exit(1);
    return {};
  }

  std::string path =
      Loader::resolve_module_path(mod->second.fullpath, specifier_name);
  return create_module(isolate, path);
}

v8::MaybeLocal<v8::Module> Loader::create_module(v8::Isolate *isolate,
                                                 std::string path) {
  // load from cache:
  auto cached_module = resolve_cache.find(path);
  if (cached_module != resolve_cache.end()) {
    return cached_module->second.Get(isolate);
  }

  std::string content =
      Loader::is_internal(path) ? load_internal(path) : load_file(path);
  v8::Local<v8::String> source = v8_value(isolate, content);
  v8::ScriptOrigin origin(v8_value(isolate, path), 0, 0, true, -1,
                          v8::Local<v8::Value>(), false, false, true);

  v8::ScriptCompiler::Source script_source(source, origin);
  v8::Local<v8::Module> module;

  if (v8::ScriptCompiler::CompileModule(isolate, &script_source)
          .ToLocal(&module)) {
    // TODO: I think we want to limit copies of mod struct
    runtime::Mod mod = {.fullpath = path, .content = content};
    mods.emplace(module->ScriptId(), mod);
    resolve_cache[path].Reset(isolate, module);

    return module;
  }

  printf("error: Unable to find module '%s'\n", path.c_str());
  exit(1);
  return {};
}

// setup import.meta
void Loader::set_meta(v8::Local<v8::Context> context,
                      v8::Local<v8::Module> module,
                      v8::Local<v8::Object> meta) {
  auto result = mods.find(module->ScriptId());
  if (result != mods.end()) {
    v8::Isolate *isolate = context->GetIsolate();
    std::string fullpath = result->second.fullpath;
    std::string dirname = fs::path(fullpath).parent_path().string();
    std::string url = Loader::path_to_url(fullpath);

    meta->Set(context, v8_symbol(isolate, "url"), v8_value(isolate, url))
        .Check();
    meta->Set(context, v8_symbol(isolate, "filename"),
              v8_value(isolate, fullpath))
        .Check();
    meta->Set(context, v8_symbol(isolate, "dirname"),
              v8_value(isolate, dirname))
        .Check();
    meta->Set(context, v8_symbol(isolate, "resolve"),
              v8::Function::New(context, Loader::resolve).ToLocalChecked())
        .Check();
  }
}

// implementation for import.meta.resolve()
void Loader::resolve(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> meta = args.Holder();
  v8::MaybeLocal<v8::Value> parent =
      meta->Get(isolate->GetCurrentContext(), v8_symbol(isolate, "filename"));
  if (parent.IsEmpty()) {
    isolate->ThrowException(v8::Exception::ReferenceError(
        v8::String::NewFromUtf8(isolate, "Unable to get import.meta.dirname")
            .ToLocalChecked()));
    return;
  }

  v8::Local<v8::Value> path_to_resolve = args[0];
  if (!path_to_resolve->IsString()) {
    isolate->ThrowException(v8::Exception::TypeError(
        v8::String::NewFromUtf8(isolate, "Resolve path must be a string")
            .ToLocalChecked()));
    return;
  }

  v8::String::Utf8Value parent_path(isolate, parent.ToLocalChecked());
  v8::String::Utf8Value path(isolate, path_to_resolve);

  std::string pathstr(*path);
  if (!pathstr.starts_with("/") && !pathstr.starts_with("./") &&
      !pathstr.starts_with("../")) {
    isolate->ThrowException(v8::Exception::TypeError(
        v8::String::NewFromUtf8(
            isolate, "Resolve path must be prefixed with / or ./ or ../")
            .ToLocalChecked()));
    return;
  }

  std::string abs_path =
      Loader::resolve_module_path(fs::path(*parent_path), *path);
  std::string url = Loader::path_to_url(abs_path);
  args.GetReturnValue().Set(v8_value(isolate, url));
}

// await import()
v8::MaybeLocal<v8::Promise> Loader::dynamic_load(
    v8::Local<v8::Context> context, v8::Local<v8::Data> host_defined_options,
    v8::Local<v8::Value> resource_name, v8::Local<v8::String> specifier,
    v8::Local<v8::FixedArray> import_attributes) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::Local<v8::Promise::Resolver> resolver =
      v8::Promise::Resolver::New(context).ToLocalChecked();

  v8::String::Utf8Value resource(isolate, resource_name);
  v8::String::Utf8Value specifier_str(isolate, specifier);

  std::string parent_path(*resource);

  std::string path =
      Loader::is_internal(*specifier_str)
          ? *specifier_str
          : Loader::resolve_module_path(parent_path, *specifier_str);

  v8::MaybeLocal<v8::Module> mod = Loader::create_module(isolate, path);
  if (mod.IsEmpty()) {
    resolver
        ->Reject(context, v8::Exception::Error(v8::String::NewFromUtf8Literal(
                              isolate, "Module not found")))
        .Check();
  } else {
    v8::Local<v8::Module> v8_mod = mod.ToLocalChecked();
    if (v8_mod->InstantiateModule(context, Loader::load).IsJust()) {
      v8::Local<v8::Value> val;
      if (v8_mod->Evaluate(context).ToLocal(&val)) {
        v8::Local<v8::Value> namespace_obj = v8_mod->GetModuleNamespace();
        resolver->Resolve(context, namespace_obj).Check();
        return resolver->GetPromise();
      } else {
        resolver
            ->Reject(context,
                     v8::Exception::Error(v8::String::NewFromUtf8Literal(
                         isolate, "Error evaluating module")))
            .Check();
      }
    } else {
      resolver
          ->Reject(context, v8::Exception::Error(v8::String::NewFromUtf8Literal(
                                isolate, "Error instantiating module")))
          .Check();
    }
  }

  return resolver->GetPromise();
}

// prevent seg faults on cleanup
void Loader::clear_resolve_cache() {
  for (auto &entry : resolve_cache) {
    entry.second.Reset();
  }
  resolve_cache.clear();
}

std::string Loader::path_to_url(std::string &path) {
  if (path.find('%') == std::string_view::npos) {
    return ada::href_from_file(path);
  }

  std::string escaped_path; // escape % sign
  for (char ch : path) {
    if (ch == '%') {
      escaped_path += "%25";
    } else {
      escaped_path += ch;
    }
  }

  return ada::href_from_file(escaped_path);
}

} // namespace runtime
