#include "errors.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "loader.h"
#include "pand.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace pand::core {

static std::unordered_map<int, v8::Global<v8::Value>> rejected_promises;

void Errors::clearPendingRejects() {
  for (auto &entry : rejected_promises) {
    entry.second.Reset();
  }
  rejected_promises.clear();
}

// runs at the end of each event loop cycle as 'after_tick' pandio hook
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

  Errors::reportUncaught(value, true);
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

v8::Local<v8::Value> Errors::Error(v8::Isolate *isolate,
                                   std::string_view message) {
  return v8::Exception::Error(Pand::value(isolate, message));
}

v8::Local<v8::Value> Errors::TypeError(v8::Isolate *isolate,
                                       std::string_view message) {
  return v8::Exception::TypeError(Pand::value(isolate, message));
}

/* throws v8::Exception::Error */
void Errors::throwException(v8::Isolate *isolate, std::string_view message) {
  v8::Local<v8::Value> error =
      v8::Exception::Error(Pand::value(isolate, message));
  isolate->ThrowException(error);
}

/* throws v8::Exception::TypeError */
void Errors::throwTypeException(v8::Isolate *isolate,
                                std::string_view message) {
  v8::Local<v8::Value> error =
      v8::Exception::TypeError(Pand::value(isolate, message));
  isolate->ThrowException(error);
}

void Errors::tryPrintLine(v8::Isolate *isolate, v8::Local<v8::Context> context,
                          v8::Local<v8::Message> message) {
  /* it happens when user throws primitive or object that does not extend Error
  class. */
  Module *mod = Module::find(message->GetScriptOrigin().ScriptId());
  if (!mod) {
    return;
  }

  int current_line = 1;
  int line = message->GetLineNumber(context).FromJust();
  int col = message->GetStartColumn();

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
}

void Errors::tryPrintStackTrace(v8::Isolate *isolate,
                                v8::Local<v8::Context> context,
                                v8::Local<v8::Message> message) {
  v8::Local<v8::StackTrace> stack_trace = message->GetStackTrace();
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
}

void Errors::reportUncaught(v8::Local<v8::Value> value, bool promise) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context;

  if (value.IsEmpty()) {
    std::string reason = "Uncaught (empty)";
    Errors::printError(reason);
    throw std::runtime_error(reason);
  }

  v8::String::Utf8Value err_str(isolate, value);
  std::string_view details = err_str.length() ? *err_str : "(empty)";

  std::string reason =
      fmt::format("Uncaught {}{}", promise ? "(in promise) " : "", details);
  Errors::printError(reason);

  v8::Local<v8::Message> err = v8::Exception::CreateMessage(isolate, value);
  Errors::tryPrintLine(isolate, context, err);
  Errors::tryPrintStackTrace(isolate, context, err);

  throw std::runtime_error(reason);
}

void Errors::reportCritical(v8::Local<v8::Value> value) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context;

  if (value.IsEmpty()) {
    std::string reason = "unknown (empty)";
    Errors::printError(reason);
    throw std::runtime_error(reason);
  }

  v8::String::Utf8Value err_str(isolate, value);
  std::string_view reason = err_str.length() ? *err_str : "(empty)";
  Errors::printError(reason);

  throw std::runtime_error(reason.data());
}

void Errors::reportCritical(std::string reason) {
  Errors::printError(reason);
  throw std::runtime_error(reason);
}

void Errors::printError(std::string_view reason) {
  // TODO: switch to specialized library for output styling
  fmt::print("{}{}\n", fmt::format(fg(fmt::color::red), "error"),
             fmt::format(": {}", reason));
}

} // namespace pand::core
