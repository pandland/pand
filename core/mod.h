#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <v8.h>

namespace fs = std::filesystem;

namespace pand::core {

inline constexpr std::string_view kInternalPrefix = "std:";

class Mod {
public:
  enum class ModType { kScript, kInternal };
  enum class ScriptType { kJavaScript, kTypeScript };

  std::string fullpath;
  std::string url;
  std::string dirname;
  std::string content;
  ModType type;
  ScriptType scriptType;

  Mod(std::string_view fullpath, ModType type)
      : fullpath(fullpath), type(type) {
    if (type == ModType::kScript) {
      url = Mod::pathToUrl(fullpath);
      dirname = fs::path(fullpath).parent_path().string();
    }
  }

  static ModType detectType(std::string_view specifier) {
    if (specifier.starts_with(kInternalPrefix))
      return ModType::kInternal;

    return ModType::kScript;
  }

  inline bool isInternal() { return type == ModType::kInternal; }

  inline bool isTS() { return scriptType == ScriptType::kTypeScript; }

  v8::MaybeLocal<v8::Module> initialize(v8::Isolate *isolate);

  static void evaluate(v8::Isolate *isolate, Mod *mod);

  int loadContent();

  static void clearResolveCache();

  static void execScript(v8::Isolate *, std::string_view);

  static void execInternal(v8::Isolate *, std::string_view);

  static std::string canonicalPath(std::string_view);
  /* parent path means full path to script file which loads given specifier */
  static std::string resolveModulePath(fs::path parent,
                                       std::string_view specifier);

  static std::string pathToUrl(std::string_view);

  // JS API:
  static void resolve(const v8::FunctionCallbackInfo<v8::Value> &);

  static v8::MaybeLocal<v8::Promise> dynamicImport(v8::Local<v8::Context>,
                                                   v8::Local<v8::Data>,
                                                   v8::Local<v8::Value>,
                                                   v8::Local<v8::String>,
                                                   v8::Local<v8::FixedArray>);

  static void setMeta(v8::Local<v8::Context>, v8::Local<v8::Module>,
                      v8::Local<v8::Object>);

  static v8::MaybeLocal<v8::Module> load(v8::Local<v8::Context>,
                                         v8::Local<v8::String>,
                                         v8::Local<v8::FixedArray>,
                                         v8::Local<v8::Module>);
};

} // namespace pand::core
