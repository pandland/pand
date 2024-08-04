#pragma once
#include <v8.h>

inline v8::Local<v8::String> v8_value(v8::Isolate *isolate, const std::string &str) {
    v8::MaybeLocal<v8::String> maybe_str = v8::String::NewFromUtf8(isolate, str.c_str(), v8::NewStringType::kNormal);
    if (maybe_str.IsEmpty()) {
        return v8::String::Empty(isolate);
    }

    return maybe_str.ToLocalChecked();
}

inline v8::Local<v8::Number> v8_value(v8::Isolate *isolate, double value) {
  return v8::Number::New(isolate, value);  
}

inline v8::Local<v8::Boolean> v8_value(v8::Isolate *isolate, bool value) {
    return v8::Boolean::New(isolate, value);
}

inline v8::Local<v8::String> v8_symbol(v8::Isolate *isolate, const char *symbol) {
    v8::MaybeLocal<v8::String> maybe_str = v8::String::NewFromOneByte(isolate, (const uint8_t*)symbol, v8::NewStringType::kNormal);
    if (maybe_str.IsEmpty()) {
        return v8::String::Empty(isolate);
    }

    return maybe_str.ToLocalChecked();
}
