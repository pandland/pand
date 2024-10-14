#pragma once
#include <v8.h>
#include <pandio.h>

namespace pand::core {

class Timer {

  enum TimerType {
    INTERVAL,
    TIMEOUT
  };

  int id;
  pd_timer_t handle;
  TimerType type;

public:
  // JS API
  static void initialize(v8::Local<v8::Object> exports);

  static void setTimeout(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void setInterval(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void clear(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
