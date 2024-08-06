#include <luxio.h>
#include <v8.h>
#include <unordered_map>
#include <iostream>

namespace runtime {

enum TimerType {
  INTERVAL,
  TIMEOUT
};

typedef int64_t TimerId;

struct TimerWrap {
  TimerId id;
  TimerType type;
  lx_timer_t handle;
  v8::Global<v8::Function> callback;
};

class Timers {
  lx_io_t *ctx;
  TimerId ids = 1;
  std::unordered_map<TimerId, TimerWrap*> active_timers;

  TimerId get_id() {
    return ids++;
  }
  
protected:
  static Timers *instance_;
  Timers(lx_io_t *ctx): ctx(ctx) {}
public:
  static void initialize(lx_io_t *ctx) {
    Timers::instance_ = new Timers(ctx);
  }

  static Timers *instance() {
    return Timers::instance_;
  }

  TimerId set_timeout(v8::Local<v8::Function> callback, int64_t timeout) {
    TimerWrap *timer = new TimerWrap;
    timer->id = get_id();
    timer->callback.Reset(v8::Isolate::GetCurrent(), callback);
    timer->type = TIMEOUT;
    lx_timer_init(ctx, &timer->handle);
    timer->handle.data = timer;
  
    lx_timer_start(&timer->handle, handle_timer, timeout);
    active_timers[timer->id] = std::move(timer);

    return timer->id;
  }

  TimerId set_interval(v8::Local<v8::Function> callback, int64_t interval) {
    TimerWrap *timer = new TimerWrap;
    timer->id = get_id();
    timer->callback.Reset(v8::Isolate::GetCurrent(), callback);
    timer->type = INTERVAL;
    lx_timer_init(ctx, &timer->handle);
    timer->handle.data = timer;

    lx_timer_repeat(&timer->handle, handle_timer, interval);
    active_timers[timer->id] = std::move(timer);

    return timer->id;
  }
  
  void cleanup(TimerWrap *timer) {
    active_timers.erase(timer->id);
    timer->callback.Reset();
    delete timer;
  }

  void clear_timer(TimerId id) {
    auto it = active_timers.find(id);

    if (it != active_timers.end()) {
      lx_timer_stop(&it->second->handle);
      cleanup(it->second);
    }
  }

  static void handle_timer(lx_timer_t *handle) {
    TimerWrap *timer = static_cast<TimerWrap*>(handle->data);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    if (isolate->IsDead()) {
      return;
    }

    v8::Local<v8::Function> callback = timer->callback.Get(isolate);

    // maybe handle it in better way?
    if (callback.IsEmpty()) {
      return;
    }

    callback->Call(context, v8::Undefined(isolate), 0, {}).ToLocalChecked();

    if (timer->type == TIMEOUT) {
      Timers::instance()->cleanup(timer);
    }
  }

  void setup(v8::Local<v8::ObjectTemplate> global, v8::Isolate *isolate) {
    global->Set(isolate, "setTimeout", v8::FunctionTemplate::New(isolate, JS_setTimeout));
    global->Set(isolate, "setInterval", v8::FunctionTemplate::New(isolate, JS_setInterval));
    global->Set(isolate, "clearTimeout", v8::FunctionTemplate::New(isolate, JS_clearTimer));
    global->Set(isolate, "clearInterval", v8::FunctionTemplate::New(isolate, JS_clearTimer));
  }

    /* JavaScript bindings */
  static void JS_setTimeout(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    int64_t timeout = args[1]->IntegerValue(context).ToChecked();
    v8::Local<v8::Function> callback = args[0].As<v8::Function>();

    TimerId id = Timers::instance()->set_timeout(callback, timeout);

    args.GetReturnValue().Set(v8::Number::New(isolate, id));
  }

  static void JS_setInterval(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    int64_t interval = args[1]->IntegerValue(context).ToChecked();
    v8::Local<v8::Function> callback = args[0].As<v8::Function>();

    TimerId id = Timers::instance()->set_interval(callback, interval);

    args.GetReturnValue().Set(v8::Number::New(isolate, id));
  }

  static void JS_clearTimer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    int64_t id = args[0]->IntegerValue(context).ToChecked();
    Timers::instance()->clear_timer(id);
  }
};

Timers* Timers::instance_ = nullptr;

}
