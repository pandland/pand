#pragma once
#include <v8.h>
#include <pandio.h>

namespace pand::core {

class Errors {
public:
  static void checkPendingErrors(pd_io_t *ctx);

  static void promiseRejectedCallback(v8::PromiseRejectMessage);

  static void throwCritical(v8::Local<v8::Value>);

  static void clearPendingRejects();
};

} // namespace pand::core
