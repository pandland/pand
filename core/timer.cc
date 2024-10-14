#include "pand.h"
#include "timer.h"

namespace pand::core {

void Timer::initialize(v8::Local<v8::Object> exports) {}

void Timer::constructor(const v8::FunctionCallbackInfo<v8::Value> &args) {}

void Timer::setInterval(const v8::FunctionCallbackInfo<v8::Value> &args) {}

void Timer::setTimeout(const v8::FunctionCallbackInfo<v8::Value> &args) {}

void Timer::clear(const v8::FunctionCallbackInfo<v8::Value> &args) {}

}
