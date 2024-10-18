#pragma once

#include <libplatform/libplatform.h>
#include <pandio.h>
#include <string>
#include <v8-object.h>
#include <v8.h>

namespace pand::core {

class Pand {
  std::unique_ptr<v8::Platform> platform;
  v8::Isolate::CreateParams create_params;
  v8::Persistent<v8::Function> tcpStreamConstructor;

  Pand();
  Pand(const Pand &) = delete;
  ~Pand();

public:
  pd_io_t *ctx;
  v8::Isolate *isolate;
  static Pand *get();
  void exit(int status);
  void run(const std::string &);
  void run(const std::string &, int argc, char *argv);

  static void makeCallback(v8::Local<v8::Object> &, v8::Isolate *, const char *,
                           v8::Local<v8::Value> *, size_t);

  static v8::Local<v8::Value> makeSystemError(v8::Isolate *, int err);

  static inline v8::Local<v8::String> symbol(v8::Isolate *isolate,
                                             const char *symbol) {
    v8::MaybeLocal<v8::String> maybe_str = v8::String::NewFromOneByte(
        isolate, (const uint8_t *)symbol, v8::NewStringType::kNormal);
    if (maybe_str.IsEmpty()) {
      return v8::String::Empty(isolate);
    }

    return maybe_str.ToLocalChecked();
  }

  static inline v8::Local<v8::String> value(v8::Isolate *isolate,
                                            std::string_view str) {
    v8::MaybeLocal<v8::String> maybe_str = v8::String::NewFromUtf8(
        isolate, str.data(), v8::NewStringType::kNormal);
    if (maybe_str.IsEmpty()) {
      return v8::String::Empty(isolate);
    }

    return maybe_str.ToLocalChecked();
  }

  static inline v8::Local<v8::Number> value(v8::Isolate *isolate,
                                            double number) {
    return v8::Number::New(isolate, number);
  }

  void setTcpStreamConstructor(v8::Local<v8::Function>);
  v8::Local<v8::Function> getTcpStreamConstructor();
};

} // namespace pand::core
