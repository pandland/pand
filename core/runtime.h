#pragma once
#include <v8.h>

namespace pand::core {

class Runtime {
public:
  static void initialize(v8::Local<v8::ObjectTemplate>, v8::Isolate *);

  static void setArgv(v8::Local<v8::Context>, int, char **);

  static void print(const v8::FunctionCallbackInfo<v8::Value> &);

  static void cwd(const v8::FunctionCallbackInfo<v8::Value> &);

  static void env(const v8::FunctionCallbackInfo<v8::Value> &);

  static void exit(const v8::FunctionCallbackInfo<v8::Value> &);

  static void bind(const v8::FunctionCallbackInfo<v8::Value> &);
};

} // namespace pand::core
