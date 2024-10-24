#pragma once
#include <v8.h>

namespace pand::core {

class Buffer {
public:
  static void initialize(v8::Local<v8::Object>);

  static void fillRandom(const v8::FunctionCallbackInfo<v8::Value> &);
};

}
