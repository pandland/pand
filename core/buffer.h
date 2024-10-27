#pragma once
#include <v8.h>

namespace pand::core {

class Buffer {
public:
  static bool isBuffer(v8::Local<v8::Value> value) {
    return value->IsUint8Array();
  }

  static void initialize(v8::Local<v8::Object>);

  static void fillRandom(const v8::FunctionCallbackInfo<v8::Value> &);

  static void memcmp(const v8::FunctionCallbackInfo<v8::Value> &);
};

} // namespace pand::core
