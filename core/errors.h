#include <v8.h>

namespace pand::core {

class Errors {
public:
  static void checkPendingErrors(void *);

  static void promiseRejectedCallback(v8::PromiseRejectMessage);

  static void reportDetails(v8::Local<v8::Message> msg,
                            v8::Local<v8::Context> context);

  static void clearPendingRejects();
};

} // namespace pand::core
