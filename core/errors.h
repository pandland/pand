#pragma once
#include <v8.h>

namespace pand::core {

class Errors {
public:
  static void checkPendingErrors(void *);

  static void promiseRejectedCallback(v8::PromiseRejectMessage);

  static void throwCritical(v8::Local<v8::Value>);

  static void clearPendingRejects();
};

} // namespace pand::core
