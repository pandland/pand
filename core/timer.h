#pragma once
#include "pand.h"
#include <pandio.h>
#include <v8.h>

namespace pand::core {

class Timer {
  enum class Type { INTERVAL, TIMEOUT };

  int id;
  pd_timer_t handle;
  Timer::Type type;

public:
  Timer(int id, Timer::Type type) : id(id), type(type) {
    pd_timer_init(Pand::get()->ctx, &handle);
  }

  static int counter;

  // JS API:
  static void initialize(v8::Local<v8::Object> exports);

  static void constructor(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void setTimeout(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void setInterval(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void clear(const v8::FunctionCallbackInfo<v8::Value> &args);
};

} // namespace pand::core
