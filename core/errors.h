#pragma once
#include <pandio.h>
#include <v8.h>

namespace pand::core {

class Errors {
public:
  static void checkPendingErrors(pd_io_t *ctx);

  static void promiseRejectedCallback(v8::PromiseRejectMessage);

  static void removePendingReject(int);
  static void clearPendingRejects();

  static v8::Local<v8::Value> Error(v8::Isolate *, std::string_view);

  static v8::Local<v8::Value> TypeError(v8::Isolate *, std::string_view);

  static void printError(std::string_view);

  static void tryPrintLine(v8::Isolate *, v8::Local<v8::Context>,
                           v8::Local<v8::Message>);

  /* reports uncaught error and terminate program. DO NOT throw it inside
   * microtasks. */
  static void reportUncaught(v8::Local<v8::Value>, bool promise = false);

  static void reportCritical(std::string);

  static void reportCritical(v8::Local<v8::Value> value);

  static void tryPrintStackTrace(v8::Isolate *, v8::Local<v8::Context>,
                                 v8::Local<v8::Message>);

  static void throwException(v8::Isolate *, std::string_view);

  static void throwTypeException(v8::Isolate *, std::string_view);
};

} // namespace pand::core
