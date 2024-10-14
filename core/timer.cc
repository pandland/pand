#include "timer.h"
#include "v8_utils.cc"
#include <iostream>
#include <v8-context.h>
#include <v8-isolate.h>
#include <v8-local-handle.h>
#include <v8-object.h>

namespace pand::core {

int Timer::counter = 1;

void Timer::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, Timer::constructor);

  t->SetClassName(v8_symbol(isolate, "Timer"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  v8::Local<v8::FunctionTemplate> setTimeoutT =
      v8::FunctionTemplate::New(isolate, Timer::setTimeout);
  t->PrototypeTemplate()->Set(isolate, "setTimeout", setTimeoutT);

  v8::Local<v8::FunctionTemplate> setIntervalT =
      v8::FunctionTemplate::New(isolate, Timer::setInterval);
  t->PrototypeTemplate()->Set(isolate, "setInterval", setIntervalT);

  v8::Local<v8::FunctionTemplate> clearT =
      v8::FunctionTemplate::New(isolate, Timer::clear);
  t->PrototypeTemplate()->Set(isolate, "clear", clearT);

  v8::Local<v8::Function> func = t->GetFunction(context).ToLocalChecked();
  exports
      ->Set(context, v8::String::NewFromUtf8(isolate, "Timer").ToLocalChecked(),
            func)
      .ToChecked();
}

void Timer::constructor(const v8::FunctionCallbackInfo<v8::Value> &args) {
  assert(args.IsConstructCall());
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  // TODO: read type from args
  Timer *timer = new Timer(Timer::Type::TIMEOUT, args.This());
  args.This()->SetAlignedPointerInInternalField(0, timer);
}

void Timer::setInterval(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  int64_t timeout = args[0]->IntegerValue(context).ToChecked();
  Timer *timer =
      static_cast<Timer *>(args.This()->GetAlignedPointerFromInternalField(0));
  pd_timer_repeat(&timer->handle, Timer::onTimeout, timeout);

  return args.GetReturnValue().Set(v8::Number::New(isolate, timer->id));
}

void Timer::setTimeout(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  int64_t timeout = args[0]->IntegerValue(context).ToChecked();
  Timer *timer =
      static_cast<Timer *>(args.This()->GetAlignedPointerFromInternalField(0));
  pd_timer_start(&timer->handle, Timer::onTimeout, timeout);

  return args.GetReturnValue().Set(v8::Number::New(isolate, timer->id));
}

void Timer::clear(const v8::FunctionCallbackInfo<v8::Value> &args) {}

void Timer::onTimeout(pd_timer_t *handle) {
  Pand *pand = Pand::get();
  Timer *timer = static_cast<Timer *>(handle->data);

  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Object> obj = timer->obj.Get(pand->isolate);
  v8::Local<v8::Function> callback =
      obj->Get(context, v8_symbol(isolate, "onTimeout"))
          .ToLocalChecked()
          .As<v8::Function>();

  v8::Local<v8::Value> args[0] = {};
  callback->Call(context, v8::Undefined(isolate), 1, args).ToLocalChecked();

  delete timer;
}

} // namespace pand::core
