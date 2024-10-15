#pragma once
#include "pand.h"
#include <pandio.h>
#include <v8.h>

namespace pand::core {

class Timer {
  int id;
  pd_timer_t handle;
  v8::Persistent<v8::Object> obj;

public:
  Timer(v8::Local<v8::Object> obj) {
    Pand *pand = Pand::get();
    pd_timer_init(pand->ctx, &handle);
    this->handle.data = this;
    this->id = Timer::counter++;
    this->obj.Reset(obj->GetIsolate(), obj);
  }

  ~Timer() { this->obj.Reset(); }

  static int counter;

  static void initialize(v8::Local<v8::Object> exports);

  static void constructor(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void setTimeout(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void setInterval(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void onTimeout(pd_timer_t *);

  static void onInterval(pd_timer_t *);

  static void makeCallback(Timer *);

  static void clear(const v8::FunctionCallbackInfo<v8::Value> &args);
};

} // namespace pand::core
