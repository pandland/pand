#include "errors.h"
#include "pand.h"
#include <exception>
#include <iostream>

namespace pand::core {

static std::unordered_map<int, v8::Global<v8::Value>> rejected_promises;
static bool checked = false;

void Errors::clearPendingRejects() {
  for (auto &entry : rejected_promises) {
    entry.second.Reset();
  }
  rejected_promises.clear();
}

void Errors::checkPendingErrors(void *data) {
  checked = false;

  if (rejected_promises.empty()) {
    return;
  }

  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Global<v8::Value> &result = rejected_promises.begin()->second;
  v8::Local<v8::Value> value = result.Get(isolate);

  std::cerr << "error: Uncaught (in promise) in checkPendingErrors:"
            << std::endl;
  pand->exit(1);
}

void Errors::promiseRejectedCallback(v8::PromiseRejectMessage message) {
  v8::Isolate *isolate = Pand::get()->isolate;
  v8::HandleScope handle_scope(isolate);

  v8::PromiseRejectEvent event = message.GetEvent();
  v8::Local<v8::Promise> promise = message.GetPromise();
  v8::Local<v8::Value> error = message.GetValue();

  int promiseId = promise->GetIdentityHash();

  switch (event) {
  case v8::kPromiseRejectWithNoHandler:
    rejected_promises[promiseId].Reset(isolate, error);
    break;
  case v8::kPromiseHandlerAddedAfterReject:
    if (rejected_promises.find(promiseId) != rejected_promises.end()) {
      rejected_promises[promiseId].Reset();
      rejected_promises.erase(promiseId);
    }
    break;
  default:
    return;
  }
  // TODO: use pandio's event loop for checks
  if (!checked) {
    isolate->EnqueueMicrotask(Errors::checkPendingErrors);
    checked = true;
  }
}

void Errors::reportDetails(v8::Local<v8::Message> msg,
                           v8::Local<v8::Context> context) {}

} // namespace pand::core
