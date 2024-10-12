#include "mod.h"
#include "v8_utils.cc"
#include <ada.h>
#include <string_view>

namespace pand::core {

std::unordered_map<int, Mod> mods;
std::unordered_map<std::string, v8::Global<v8::Module>> resolve_cache;

v8::MaybeLocal<v8::Module>
Mod::load(v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
          v8::Local<v8::FixedArray> importAssertions,
          v8::Local<v8::Module> referrer) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::String::Utf8Value specifierUtf8(isolate, specifier);
  std::string_view specifierName = *specifierUtf8;

  auto modRes = mods.find(referrer->ScriptId());
  if (modRes == mods.end()) {
    printf("error: Unable to find referrer's path\n");
    exit(1);
  }

  Mod &mod = modRes->second;
  std::string path =
      Mod::resolve_module_path(mod.fullpath, specifierName);
}

void Mod::set_meta(v8::Local<v8::Context> context, v8::Local<v8::Module> module,
                   v8::Local<v8::Object> meta) {
  auto result = mods.find(module->ScriptId());
  if (result != mods.end()) {
    v8::Isolate *isolate = context->GetIsolate();
    Mod &mod = result->second;

    meta->Set(context, v8_symbol(isolate, "url"), v8_value(isolate, mod.url))
        .Check();
    meta->Set(context, v8_symbol(isolate, "filename"),
              v8_value(isolate, mod.fullpath))
        .Check();
    meta->Set(context, v8_symbol(isolate, "dirname"),
              v8_value(isolate, mod.dirname))
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
      Mod::resolve_module_path(fs::path(*parent_path), *path);
  std::string url = Mod::path_to_url(abs_path);
  args.GetReturnValue().Set(v8_value(isolate, url));
}

void Mod::clear_resolve_cache() {
  for (auto &entry : resolve_cache) {
    entry.second.Reset();
  }
  resolve_cache.clear();
}

std::string Mod::resolve_module_path(fs::path parent,
                                     std::string_view specifier) {
  fs::path abs_path = parent.parent_path() / fs::path(specifier);
  return abs_path.lexically_normal().string();
}

std::string Mod::path_to_url(std::string_view path) {
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
