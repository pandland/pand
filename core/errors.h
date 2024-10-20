#pragma once
#include <pandio.h>
#include <v8.h>

namespace pand::core {

class Errors {
public:
  static void checkPendingErrors(pd_io_t *ctx);

  static void promiseRejectedCallback(v8::PromiseRejectMessage);

  static void throwCritical(std::string_view);

  static void throwCritical(v8::Local<v8::Value>);

  static void clearPendingRejects();

  static v8::Local<v8::Value> Error(v8::Isolate *, std::string_view);

  static v8::Local<v8::Value> TypeError(v8::Isolate *, std::string_view);

  static void throwException(v8::Isolate *, std::string_view);

  static void throwTypeException(v8::Isolate *, std::string_view);
};

} // namespace pand::core
