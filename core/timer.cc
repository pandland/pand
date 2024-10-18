#include "timer.h"
#include "errors.h"

namespace pand::core {

std::unordered_map<int, Timer *> active_timers;
int Timer::counter = 1;

void Timer::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::FunctionTemplate> t =
      v8::FunctionTemplate::New(isolate, Timer::constructor);

  t->SetClassName(Pand::symbol(isolate, "Timer"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  v8::Local<v8::FunctionTemplate> setTimeoutT =
      v8::FunctionTemplate::New(isolate, Timer::setTimeout);
  t->PrototypeTemplate()->Set(isolate, "setTimeout", setTimeoutT);

  v8::Local<v8::FunctionTemplate> setIntervalT =
      v8::FunctionTemplate::New(isolate, Timer::setInterval);
  t->PrototypeTemplate()->Set(isolate, "setInterval", setIntervalT);

  t->Set(isolate, "clear", v8::FunctionTemplate::New(isolate, Timer::clear));

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
  Timer *timer = new Timer(args.This());
  args.This()->SetAlignedPointerInInternalField(0, timer);
  active_timers[timer->id] = timer;
}

void Timer::setInterval(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  int64_t timeout = args[0]->IntegerValue(context).ToChecked();
  Timer *timer =
      static_cast<Timer *>(args.This()->GetAlignedPointerFromInternalField(0));
  pd_timer_repeat(&timer->handle, Timer::onInterval, timeout);

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

void Timer::clear(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  int64_t id = args[0]->IntegerValue(context).ToChecked();
  auto iter = active_timers.find(id);
  if (iter == active_timers.end()) {
    return;
  }

  Timer *timer = iter->second;
  pd_timer_stop(&timer->handle);
  active_timers.erase(timer->id);
  delete timer;
}

void Timer::makeCallback(Timer *timer) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Object> obj = timer->obj.Get(isolate);
  Pand::makeCallback(obj, isolate, "onTimeout", {}, 0);
}

void Timer::onTimeout(pd_timer_t *handle) {
  Timer *timer = static_cast<Timer *>(handle->data);
  Timer::makeCallback(timer);

  active_timers.erase(timer->id);
  delete timer;
}

void Timer::onInterval(pd_timer_t *handle) {
  Timer *timer = static_cast<Timer *>(handle->data);
  Timer::makeCallback(timer);
}

} // namespace pand::core
