#pragma once
#include <v8.h>

#include "buffer.h"
#include "bytes/extern.h"
#include "bytes/hex.h"
#include <simdutf.h>

namespace pand::core {

class Transcoder {
public:
  enum {
    UTF8,
    ASCII,
    LATIN,
    BASE64,
    BASE64URL,
    HEX,
  };

  static bool isEncoding(int option) {
    if (option < UTF8 || option > HEX) {
      return false;
    }

    return true;
  }

  static void initialize(v8::Local<v8::Object>);

  static void decode(const v8::FunctionCallbackInfo<v8::Value> &);

  static void encode(const v8::FunctionCallbackInfo<v8::Value> &);

  static v8::MaybeLocal<v8::String>
  decoder(v8::Isolate *isolate, const char *bytes, size_t len, int option);

  static v8::MaybeLocal<v8::ArrayBuffer>
  encoder(v8::Isolate *isolate, v8::Local<v8::String>, int option);
};

} // namespace pand::core
