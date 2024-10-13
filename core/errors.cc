#include "errors.h"
#include "mod.h"
#include "pand.h"
#include <exception>
#include <iostream>
#include <v8-context.h>
#include <v8-local-handle.h>

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

  Errors::throwCritical(value);
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

void Errors::throwCritical(v8::Local<v8::Value> value) {
  Pand *pand = Pand::get();
  if (value.IsEmpty()) {
    std::cerr << "error: Empty thrown value\n" << std::endl;
    return pand->exit(1);
  }

  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context;

  v8::String::Utf8Value err_str(isolate, value);
  v8::Local<v8::Message> err = v8::Exception::CreateMessage(isolate, value);

  std::cerr << "error: Uncaught (in promise) " << *err_str << '\n';

  // module should be found only if err object extends Error class.
  const Mod *mod = Mod::find(err->GetScriptOrigin().ScriptId());
  if (!mod) {
    return pand->exit(1);
  }

  // log error:
  int line = err->GetLineNumber(context).FromJust();
  int col = err->GetStartColumn();

  std::stringstream ss(mod->content);
  std::string str;
  int current_line = 1;

  while (std::getline(ss, str)) {
    if (current_line == line) {
      std::cerr << str << '\n';

      if (col < str.length()) {
        std::cerr << std::string(col, ' ') << "^\n";
      }
      break;
    }
    current_line++;
  }

  std::cerr << "  at: " << mod->url << ':' << line << ':' << col << std::endl;

  pand->exit(1);
}

} // namespace pand::core
