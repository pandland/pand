#include "mod.h"
#include "pand.h"
#include "v8_utils.cc"
#include <ada.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include "js_internals.h"

namespace pand::core {

std::unordered_map<int, Mod *> mods;
std::unordered_map<std::string, v8::Global<v8::Module>> resolve_cache;

Mod * Mod::find(int id) {
  auto it = mods.find(id);
  if (it == mods.end()) {
    return nullptr;
  }
  return it->second;
}

v8::MaybeLocal<v8::Module>
Mod::load(v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
          v8::Local<v8::FixedArray> import_attributes,
          v8::Local<v8::Module> referrer) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::String::Utf8Value specifierUtf8(isolate, specifier);
  std::string_view specifierName = *specifierUtf8;

  auto modIter = mods.find(referrer->ScriptId());
  if (modIter == mods.end()) {
    printf("error: Unable to find referrer's module\n");
    return v8::MaybeLocal<v8::Module>();
  }

  Mod *parent = modIter->second;
  ModType type = Mod::detectType(specifierName);
  std::string path =
      (type == ModType::kInternal)
          ? std::string(specifierName)
          : Mod::resolveModulePath(parent->fullpath, specifierName);

  // try to load from cache
  auto cached_module = resolve_cache.find(path);
  if (cached_module != resolve_cache.end()) {
    return cached_module->second.Get(isolate);
  }

  Mod *mod = new Mod(path, type);
  v8::MaybeLocal<v8::Module> result = mod->initialize(isolate);
  if (result.IsEmpty()) {
    delete mod;
    return result;
  }

  return result;
}

// await import()
v8::MaybeLocal<v8::Promise> Mod::dynamicImport(
    v8::Local<v8::Context> context, v8::Local<v8::Data> host_defined_options,
    v8::Local<v8::Value> resource_name, v8::Local<v8::String> specifier,
    v8::Local<v8::FixedArray> import_attributes) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::Local<v8::Promise::Resolver> resolver =
      v8::Promise::Resolver::New(context).ToLocalChecked();

  v8::String::Utf8Value resource(isolate, resource_name);
  v8::String::Utf8Value specifier_str(isolate, specifier);

  std::string_view parent_path(*resource);
  std::string_view specifierName(*specifier_str);

  ModType type = Mod::detectType(specifierName);
  std::string path = (type == ModType::kInternal)
                         ? std::string(specifierName)
                         : Mod::resolveModulePath(parent_path, specifierName);

  // try to load from cache
  v8::MaybeLocal<v8::Module> v8mod;
  auto cached_module = resolve_cache.find(path);
  if (cached_module != resolve_cache.end()) {
    v8mod = cached_module->second.Get(isolate);
  } else {
    Mod *mod = new Mod(path, type);
    v8mod = mod->initialize(isolate);
  }

  if (v8mod.IsEmpty()) {
    resolver
        ->Reject(context, v8::Exception::Error(v8::String::NewFromUtf8Literal(
                              isolate, "Module not found")))
        .Check();
  } else {
    v8::Local<v8::Module> v8_mod = v8mod.ToLocalChecked();
    if (v8_mod->InstantiateModule(context, Mod::load).IsJust()) {
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

int Mod::loadContent() {
  if (isInternal()) {
    auto internal_src = js_internals.find(fullpath);
    if (internal_src == js_internals.end()) {
      return -1;
    }

    content =
        std::string(internal_src->second.begin(), internal_src->second.end());
    return 0;
  }

  std::ifstream file(fullpath);
  if (!file.is_open()) {
    return -1;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  content = buffer.str();

  return 0;
}

v8::MaybeLocal<v8::Module> Mod::initialize(v8::Isolate *isolate) {
  int status = loadContent();
  if (status == -1) {
    return v8::MaybeLocal<v8::Module>();
  }

  v8::Local<v8::String> source = v8_value(isolate, content);
  v8::ScriptOrigin origin(v8_value(isolate, url), 0, 0, true, -1,
                          v8::Local<v8::Value>(), false, false, true);

  v8::ScriptCompiler::Source script_source(source, origin);
  v8::Local<v8::Module> v8mod;

  if (v8::ScriptCompiler::CompileModule(isolate, &script_source)
          .ToLocal(&v8mod)) {
    mods.emplace(v8mod->ScriptId(), this);
    resolve_cache[fullpath].Reset(isolate, v8mod);

    return v8mod;
  }

  return {};
}

std::string Mod::canonicalPath(std::string_view filepath) {
  fs::path abs_path = fs::current_path() / fs::path(filepath);
  return abs_path.lexically_normal().string();
}

void Mod::execScript(v8::Isolate *isolate, std::string_view filepath) {
  fs::path path(filepath);
  std::string fullpath =
      path.is_absolute() ? path.string() : Mod::canonicalPath(filepath);
  Mod *mod = new Mod(fullpath, ModType::kScript);
  return Mod::evaluate(isolate, mod);
}

void Mod::execInternal(v8::Isolate *isolate, std::string_view modulename) {
  Mod *mod = new Mod(modulename, ModType::kInternal);
  return Mod::evaluate(isolate, mod);
}

void Mod::evaluate(v8::Isolate *isolate, Mod *mod) {
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::MaybeLocal<v8::Module> result = mod->initialize(isolate);

  if (result.IsEmpty())
    return;

  if (mod->loadContent())
    return;

  v8::Local<v8::Module> v8mod = result.ToLocalChecked();
  bool intialized = v8mod->InstantiateModule(context, Mod::load).IsJust();
  if (!intialized) {
    printf("error: Unable to init module\n");
    exit(1);
  }

  v8::MaybeLocal<v8::Value> evaluation = v8mod->Evaluate(context);
  if (evaluation.IsEmpty()) {
    printf("error: %s\n", "evaluation failed");
    exit(1);
  }

  v8::Local<v8::Value> value = evaluation.ToLocalChecked();
  if (value->IsPromise()) {
    v8::Local<v8::Promise> promise = value.As<v8::Promise>();
    if (promise->State() == v8::Promise::kRejected) {
      v8::String::Utf8Value error(isolate, promise->Result());
      v8::Local<v8::Message> errmsg =
          v8::Exception::CreateMessage(isolate, promise->Result());
      printf("error: Uncaught (in promise) %s\n", *error);
      v8::String::Utf8Value errfile(isolate, errmsg->GetScriptResourceName());
      printf("filename: %s\n", *errfile);
      // Loader::report_details(errmsg, context);
    }
  }
}

void Mod::setMeta(v8::Local<v8::Context> context, v8::Local<v8::Module> module,
                  v8::Local<v8::Object> meta) {
  auto result = mods.find(module->ScriptId());
  if (result != mods.end()) {
    v8::Isolate *isolate = context->GetIsolate();
    Mod *mod = result->second;

    meta->Set(context, v8_symbol(isolate, "url"), v8_value(isolate, mod->url))
        .Check();
    meta->Set(context, v8_symbol(isolate, "filename"),
              v8_value(isolate, mod->fullpath))
        .Check();
    meta->Set(context, v8_symbol(isolate, "dirname"),
              v8_value(isolate, mod->dirname))
        .Check();
    meta->Set(context, v8_symbol(isolate, "resolve"),
              v8::Function::New(context, Mod::resolve).ToLocalChecked())
        .Check();
  }
}

void Mod::resolve(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> meta = args.Holder();
  v8::MaybeLocal<v8::Value> parent =
      meta->Get(isolate->GetCurrentContext(), v8_symbol(isolate, "filename"));
  if (parent.IsEmpty()) {
    isolate->ThrowException(v8::Exception::ReferenceError(
        v8_symbol(isolate, "Unable to get import.meta.dirname")));
    return;
  }

  v8::Local<v8::Value> path_to_resolve = args[0];
  if (!path_to_resolve->IsString()) {
    isolate->ThrowException(v8::Exception::TypeError(
        v8_symbol(isolate, "Unable to get import.meta.dirname")));
    return;
  }

  v8::String::Utf8Value parent_path(isolate, parent.ToLocalChecked());
  v8::String::Utf8Value path(isolate, path_to_resolve);

  std::string_view pathstr = *path;
  if (!pathstr.starts_with("/") && !pathstr.starts_with("./") &&
      !pathstr.starts_with("../")) {
    isolate->ThrowException(v8::Exception::TypeError(v8_symbol(
        isolate, "Resolve path must be prefixed with / or ./ or ../")));
    return;
  }

  std::string abs_path = Mod::resolveModulePath(fs::path(*parent_path), *path);
  std::string url = Mod::pathToUrl(abs_path);
  args.GetReturnValue().Set(v8_value(isolate, url));
}

void Mod::clearResolveCache() {
  for (auto &entry : resolve_cache) {
    entry.second.Reset();
  }
  resolve_cache.clear();
}

void Mod::clearMods() {
  for (auto &entry : mods) {
    delete entry.second;
  }
  resolve_cache.clear();
}

std::string Mod::resolveModulePath(fs::path parent,
                                   std::string_view specifier) {
  fs::path abs_path = parent.parent_path() / fs::path(specifier);
  return abs_path.lexically_normal().string();
}

std::string Mod::pathToUrl(std::string_view path) {
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

} // namespace pand::core
