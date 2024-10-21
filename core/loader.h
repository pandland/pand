#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <v8.h>

namespace fs = std::filesystem;

namespace pand::core {

struct Module: public std::enable_shared_from_this<Module> {
  enum class Type { kScript, kInternal };

  std::string fullpath;
  std::string url;
  std::string dirname;
  std::string content;
  Module::Type type;
  bool isMain = false;

  bool read(v8::Isolate *);

  inline bool isInternal() { return type == Type::kInternal; }

  v8::MaybeLocal<v8::Module> compile(v8::Isolate *);

  v8::MaybeLocal<v8::Module> init(v8::Isolate *);

  static std::shared_ptr<Module> find(int);

  Module(std::string_view, std::string_view, Type);
};

class Loader {
public:
  static v8::MaybeLocal<v8::Module> fromCache(v8::Isolate *, std::string);

  static void evaluate(v8::Isolate *, std::shared_ptr<Module>);

  static void execScript(v8::Isolate *, std::string_view);

  static void execInternal(v8::Isolate *, std::string_view);

  static v8::MaybeLocal<v8::Module> load(v8::Local<v8::Context>,
                                         v8::Local<v8::String>,
                                         v8::Local<v8::FixedArray>,
                                         v8::Local<v8::Module>);

  static v8::MaybeLocal<v8::Promise> dynamicImport(v8::Local<v8::Context>,
                                                   v8::Local<v8::Data>,
                                                   v8::Local<v8::Value>,
                                                   v8::Local<v8::String>,
                                                   v8::Local<v8::FixedArray>);

  static void resolve(const v8::FunctionCallbackInfo<v8::Value> &);

  static void setMeta(v8::Local<v8::Context>, v8::Local<v8::Module>,
                      v8::Local<v8::Object>);

  static void clearResolveCache();

  static void clearMods();

  static std::string resolveModulePath(fs::path, std::string_view);

  static std::string pathURL(std::string_view);
};

} // namespace pand::core
