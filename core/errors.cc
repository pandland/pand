#include "errors.h"
#include "mod.h"
#include "pand.h"
#include <iostream>
#include <stdexcept>

namespace pand::core {

static std::unordered_map<int, v8::Global<v8::Value>> rejected_promises;

void Errors::clearPendingRejects() {
  for (auto &entry : rejected_promises) {
    entry.second.Reset();
  }
  rejected_promises.clear();
}

void Errors::checkPendingErrors(pd_io_t *ctx) {
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
}

void Errors::throwCritical(v8::Local<v8::Value> value) {
  Pand *pand = Pand::get();
  if (value.IsEmpty()) {
    std::cerr << "error: Empty thrown value\n" << std::endl;
    throw std::runtime_error("empty thrown value");
  }

  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context;

  v8::String::Utf8Value err_str(isolate, value);
  v8::Local<v8::Message> err = v8::Exception::CreateMessage(isolate, value);

  std::cerr << "error: Uncaught (in promise) " << *err_str << '\n';
  // module should be found only if err object extends Error class.
  Mod *mod = Mod::find(err->GetScriptOrigin().ScriptId());
  if (!mod) {
    throw std::runtime_error(*err_str);
  }

  // log error:
  int current_line = 1;
  int line = err->GetLineNumber(context).FromJust();
  int col = err->GetStartColumn();

  std::stringstream ss(mod->content);
  std::string str;

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

  v8::Local<v8::StackTrace> stack_trace = err->GetStackTrace();
  if (!stack_trace.IsEmpty()) {
    int frame_count = stack_trace->GetFrameCount();
    for (int i = 0; i < frame_count; i++) {
      v8::Local<v8::StackFrame> frame = stack_trace->GetFrame(isolate, i);

      v8::String::Utf8Value script_name(isolate, frame->GetScriptName());
      v8::String::Utf8Value function_name(isolate, frame->GetFunctionName());
      int line_number = frame->GetLineNumber();
      int column = frame->GetColumn();

      std::cerr << "  at "
                << (function_name.length() > 0 ? *function_name : "(anonymous)")
                << " (" << *script_name << ":" << line_number << ":" << column
                << ")\n";
    }
  }

  throw std::runtime_error(*err_str);
}

} // namespace pand::core
