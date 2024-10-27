#pragma once
#include <v8.h>

namespace pand::core {

class Buffer {
public:
  static bool isBuffer(v8::Local<v8::Value> value) {
    return value->IsUint8Array();
  }

  static inline char *getBytes(v8::Local<v8::Uint8Array> ui) {
    char *bytes = static_cast<char*>(ui->Buffer()->Data());
    return bytes + ui->ByteOffset();
  }

  static inline size_t getSize(v8::Local<v8::Uint8Array> ui) {
    return ui->ByteLength();
  }

  static void initialize(v8::Local<v8::Object>);

  static void fillRandom(const v8::FunctionCallbackInfo<v8::Value> &);

  static void memcmp(const v8::FunctionCallbackInfo<v8::Value> &);
};

} // namespace pand::core
