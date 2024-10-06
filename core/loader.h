#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <v8.h>

namespace fs = std::filesystem;

namespace runtime {

struct Mod {
  std::string fullpath;
  std::string content;
};

class Loader {
  static v8::MaybeLocal<v8::Module> create_module(v8::Isolate *, std::string);
  static std::string resolve_module_path(fs::path, const std::string &);
  static std::string resolve_entry_path(fs::path, const std::string &);
  static std::string load_file(const std::string &);
  static std::string load_internal(const std::string &);
  static inline bool is_internal(std::string specifier);
  static void report_details(v8::Local<v8::Message> msg,
                           v8::Local<v8::Context> context);

public:
  ~Loader();
  void execute(v8::Isolate *, v8::Local<v8::Context>, std::string);
  void start(v8::Isolate *, v8::Local<v8::Context>, std::string);
  static inline std::string path_to_url(std::string &);
  static void resolve(const v8::FunctionCallbackInfo<v8::Value> &);
  static v8::MaybeLocal<v8::Promise> dynamic_load(v8::Local<v8::Context>,
                                                  v8::Local<v8::Data>,
                                                  v8::Local<v8::Value>,
                                                  v8::Local<v8::String>,
                                                  v8::Local<v8::FixedArray>);
  static void set_meta(v8::Local<v8::Context>, v8::Local<v8::Module>,
                       v8::Local<v8::Object>);
  static v8::MaybeLocal<v8::Module> load(v8::Local<v8::Context>,
                                         v8::Local<v8::String>,
                                         v8::Local<v8::FixedArray>,
                                         v8::Local<v8::Module>);
  static void clear_resolve_cache();
};

} // namespace runtime
