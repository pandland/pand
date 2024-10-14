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
  v8::Persistent<v8::Object> obj;

public:
  Timer(Timer::Type type, v8::Local<v8::Object> obj) : type(type) {
    Pand *pand = Pand::get();
    pd_timer_init(pand->ctx, &handle);
    this->handle.data = this;
    this->id = Timer::counter++;
    this->obj.Reset(obj->GetIsolate(), obj);
  }

  ~Timer() {
    this->obj.Reset();
  }

  static int counter;

  // JS API:
  static void initialize(v8::Local<v8::Object> exports);

  static void constructor(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void setTimeout(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void setInterval(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void onTimeout(pd_timer_t *);

  static void clear(const v8::FunctionCallbackInfo<v8::Value> &args);
};

} // namespace pand::core
