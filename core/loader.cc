#include "loader.h"
#include "errors.h"
#include "fmt/format.h"
#include "js_internals.h"
#include "pand.h"
#include <ada.h>
#include <filesystem>

namespace pand::core {

std::unordered_map<int, Module *> modules;
std::unordered_map<std::string, v8::Global<v8::Module>> resolve_cache;


Module *Module::find(int id) {
  auto it = modules.find(id);
  if (it == modules.end()) {
    return nullptr;
  }
  return it->second;
}


Module::Module(std::string_view fullpath, std::string_view url, Type type)
    : fullpath(fullpath), url(url), type(type) {
  if (type == Type::kScript) {
    dirname = fs::path(fullpath).parent_path().string();
  }
}


bool Module::read(v8::Isolate *isolate) {
  if (isInternal()) {
    auto it = js_internals.find(fullpath);
    if (it == js_internals.end()) {
      Errors::throwException(isolate,
                             fmt::format("Invalid internal module '{}'", url));
      return false;
    }

    content = std::string(it->second.begin(), it->second.end());
    return true;
  }

  std::ifstream file(fullpath);
  if (!file.is_open()) {
    std::string reason;
    if (fs::exists(fullpath)) {
      // TODO: it would be cool to actually check read permissions or something
      reason = fmt::format("Permission denied '{}'", url);
    } else {
      reason = fmt::format("Module not found '{}'", url);
    }

    Errors::throwException(isolate, reason);
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  content = buffer.str();

  return true;
}


v8::MaybeLocal<v8::Module> Module::compile(v8::Isolate *isolate) {
  v8::Local<v8::String> source = Pand::value(isolate, content);
  v8::Local<v8::String> resource_name = Pand::value(isolate, url);
  v8::ScriptOrigin origin(resource_name, 0, 0, true, -1, v8::Local<v8::Value>(),
                          false, false, true);

  v8::ScriptCompiler::Source script_source(source, origin);
  v8::Local<v8::Module> v8mod;

  if (v8::ScriptCompiler::CompileModule(isolate, &script_source)
          .ToLocal(&v8mod)) {
    modules.emplace(v8mod->ScriptId(), this);
    resolve_cache[url].Reset(isolate, v8mod);
  }

  return v8mod;
}


v8::MaybeLocal<v8::Module> Module::get(v8::Isolate *isolate) {
  if (read(isolate)) {
    return compile(isolate);
  }
  return {};
}


v8::MaybeLocal<v8::Module> Loader::fromCache(v8::Isolate *isolate,
                                             std::string url) {
  auto cached_module = resolve_cache.find(url);
  if (cached_module != resolve_cache.end()) {
    return cached_module->second.Get(isolate);
  }
  return {};
}


void Loader::execScript(v8::Isolate *isolate, std::string_view filepath) {
  fs::path path(filepath);
  std::string fullpath =
      path.is_absolute()
          ? path.string()
          : Loader::resolveModulePath(fs::current_path(), filepath);
  std::string url = Loader::pathURL(fullpath);
  Module *mod = new Module(fullpath, url, Module::Type::kScript);
  mod->isMain = true;
  return Loader::evaluate(isolate, mod);
}


void Loader::execInternal(v8::Isolate *isolate, std::string_view name) {
  Module *mod = new Module(name, name, Module::Type::kInternal);
  return Loader::evaluate(isolate, mod);
}


v8::MaybeLocal<v8::Module>
Loader::load(v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
             v8::Local<v8::FixedArray> import_attributes,
             v8::Local<v8::Module> referrer) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::String::Utf8Value specifier_(isolate, specifier);
  std::string_view specifier_str = *specifier_;

  Module *parent = Module::find(referrer->ScriptId());
  if (!parent) {
    Errors::throwException(
        isolate,
        fmt::format("Request for '{}' is from invalid module", specifier_str));
    return {};
  }

  bool isInternal = specifier_str.starts_with("std:");
  std::string path =
      isInternal ? std::string(specifier_str)
                 : Loader::resolveModulePath(parent->dirname, specifier_str);

  std::string url = isInternal ? path : Loader::pathURL(path);

  auto cached = Loader::fromCache(isolate, url);
  if (!cached.IsEmpty())
    return cached;

  Module::Type type =
      isInternal ? Module::Type::kInternal : Module::Type::kScript;
  Module *mod = new Module(path, url, type);

  return mod->get(isolate);
}


v8::MaybeLocal<v8::Promise> Loader::dynamicImport(
    v8::Local<v8::Context> context, v8::Local<v8::Data> host_defined_options,
    v8::Local<v8::Value> resource_name, v8::Local<v8::String> specifier,
    v8::Local<v8::FixedArray> import_attributes) {

  v8::Isolate *isolate = context->GetIsolate();
  v8::Local<v8::Promise::Resolver> resolver =
      v8::Promise::Resolver::New(context).ToLocalChecked();

  v8::String::Utf8Value resource(isolate, resource_name);
  v8::String::Utf8Value specifier_(isolate, specifier);

  std::string parent_url(*resource);
  std::string_view specifier_str(*specifier_);

  bool isInternal = specifier_str.starts_with("std:");
  auto cached_parent = Loader::fromCache(isolate, parent_url);
  if (cached_parent.IsEmpty()) {
    resolver
        ->Reject(context,
                 Errors::TypeError(isolate, "Unable to get parent from cache"))
        .Check();
    return resolver->GetPromise();
  }

  Module *parent = Module::find(cached_parent.ToLocalChecked()->ScriptId());
  if (!parent) {
    resolver
        ->Reject(context,
                 Errors::TypeError(isolate, "Unable to get parent module"))
        .Check();
    return resolver->GetPromise();
  }

  std::string path =
      isInternal ? std::string(specifier_str)
                 : Loader::resolveModulePath(parent->dirname, specifier_str);
  std::string url = isInternal ? path : Loader::pathURL(path);

  v8::MaybeLocal<v8::Module> v8mod;
  Module::Type type =
      isInternal ? Module::Type::kInternal : Module::Type::kScript;

  v8::TryCatch try_catch(isolate);
  auto cached = Loader::fromCache(isolate, url);
  if (cached.IsEmpty()) {
    Module *mod = new Module(path, url, type);
    v8mod = mod->get(isolate);
  }

  if (try_catch.HasCaught()) {
    resolver->Reject(context, try_catch.Exception()).Check();
    return resolver->GetPromise();
  }

  if (v8mod.IsEmpty()) {
    std::string msg = fmt::format("Module not found: '{}'", path);
    resolver->Reject(context, Errors::TypeError(isolate, msg)).Check();
    return resolver->GetPromise();
  }

  v8::Local<v8::Module> v8_mod = v8mod.ToLocalChecked();
  if (v8_mod->InstantiateModule(context, Loader::load).IsJust()) {
    v8::Local<v8::Value> value;
    if (v8_mod->Evaluate(context).ToLocal(&value)) {
      v8::Local<v8::Value> _namespace = v8_mod->GetModuleNamespace();
      if (value->IsPromise()) {
        v8::Local<v8::Promise> promise = value.As<v8::Promise>();
        if (promise->State() == v8::Promise::kRejected) {
          resolver->Reject(context, promise->Result()).Check();
          return resolver->GetPromise();
        }
      }

      resolver->Resolve(context, _namespace).Check();
      return resolver->GetPromise();
    } else {
      resolver
          ->Reject(context, Errors::Error(isolate, "Error evaluating module"))
          .Check();
    }
    return resolver->GetPromise();
  }

  resolver
      ->Reject(context, Errors::Error(isolate, "Error instantiating module"))
      .Check();

  return resolver->GetPromise();
}


void Loader::evaluate(v8::Isolate *isolate, Module *mod) {
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::TryCatch try_catch(isolate);

  v8::MaybeLocal<v8::Module> result = mod->get(isolate);
  if (try_catch.HasCaught()) {
    Errors::throwCritical(try_catch.Exception());
    return;
  }

  if (result.IsEmpty()) {
    Errors::throwCritical(Errors::Error(isolate, "Unable to compile module"));
    return;
  }

  v8::Local<v8::Module> v8mod = result.ToLocalChecked();
  bool success = v8mod->InstantiateModule(context, Loader::load).IsJust();
  if (try_catch.HasCaught()) {
    Errors::throwCritical(try_catch.Exception());
    return;
  }

  if (!success) {
    Errors::throwCritical(
        Errors::Error(isolate, "Unable to instantiate module"));
    return;
  }

  v8::MaybeLocal<v8::Value> evaluation = v8mod->Evaluate(context);
  if (evaluation.IsEmpty()) {
    Errors::throwCritical(Errors::Error(isolate, "Unable to evaluate module"));
    return;
  }

  v8::Local<v8::Value> value = evaluation.ToLocalChecked();
  if (value->IsPromise()) {
    v8::Local<v8::Promise> promise = value.As<v8::Promise>();
    if (promise->State() == v8::Promise::kRejected) {
      Errors::throwCritical(promise->Result());
    }
  }
}


void Loader::resolve(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> meta = args.Holder();
  v8::MaybeLocal<v8::Value> parent =
      meta->Get(isolate->GetCurrentContext(), Pand::symbol(isolate, "dirname"));
  if (parent.IsEmpty()) {
    Errors::throwException(isolate, "Unable to get import.meta.dirname");
    return;
  }

  v8::Local<v8::Value> path_to_resolve = args[0];
  if (!path_to_resolve->IsString()) {
    Errors::throwTypeException(isolate, "Path to resolve must be a string");
    return;
  }

  v8::String::Utf8Value parent_path(isolate, parent.ToLocalChecked());
  v8::String::Utf8Value path(isolate, path_to_resolve);

  std::string_view pathstr = *path;
  if (!pathstr.starts_with("/") && !pathstr.starts_with("./") &&
      !pathstr.starts_with("../")) {
    Errors::throwTypeException(
        isolate, "Resolve path must be prefixed with / or ./ or ../");
    return;
  }

  std::string abs_path =
      Loader::resolveModulePath(fs::path(*parent_path), *path);
  std::string url = Loader::pathURL(abs_path);
  args.GetReturnValue().Set(Pand::value(isolate, url));
}


void Loader::setMeta(v8::Local<v8::Context> context,
                     v8::Local<v8::Module> module, v8::Local<v8::Object> meta) {
  auto mod = Module::find(module->ScriptId());
  if (mod && !mod->isInternal()) {
    v8::Isolate *isolate = context->GetIsolate();

    meta->Set(context, Pand::symbol(isolate, "url"),
              Pand::value(isolate, mod->url))
        .Check();
    meta->Set(context, Pand::symbol(isolate, "filename"),
              Pand::value(isolate, mod->fullpath))
        .Check();
    meta->Set(context, Pand::symbol(isolate, "dirname"),
              Pand::value(isolate, mod->dirname))
        .Check();
    meta->Set(context, Pand::symbol(isolate, "resolve"),
              Pand::func(context, Loader::resolve))
        .Check();
    meta->Set(context, Pand::symbol(isolate, "main"),
              Pand::boolean(isolate, mod->isMain))
        .Check();
  }
}


void Loader::clearResolveCache() {
  for (auto &entry : resolve_cache) {
    entry.second.Reset();
  }
  resolve_cache.clear();
}


void Loader::clearMods() {
  for (auto &entry : modules) {
    delete entry.second;
  }
  resolve_cache.clear();
}


std::string Loader::resolveModulePath(fs::path parent,
                                      std::string_view specifier) {
  fs::path abs_path = parent / fs::path(specifier);
  return abs_path.lexically_normal().string();
}


std::string Loader::pathURL(std::string_view path) {
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
