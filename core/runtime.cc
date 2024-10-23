#include "runtime.h"
#include "tcp.h"
#include "tcp_server.h"
#include "timer.h"
#include <filesystem>
#include <iostream>
#include <pandio.h>

#define RUNTIME_VERSION "v0.0.0"

namespace pand::core {

void Runtime::initialize(v8::Local<v8::ObjectTemplate> runtime_template,
                         v8::Isolate *isolate) {
  runtime_template->Set(isolate, "print",
                        v8::FunctionTemplate::New(isolate, Runtime::print));
  runtime_template->Set(isolate, "printerr",
                        v8::FunctionTemplate::New(isolate, Runtime::printerr));
  runtime_template->Set(isolate, "getenv",
                        v8::FunctionTemplate::New(isolate, Runtime::getenv));
  runtime_template->Set(isolate, "bind",
                        v8::FunctionTemplate::New(isolate, Runtime::bind));
  runtime_template->Set(isolate, "cwd",
                        v8::FunctionTemplate::New(isolate, Runtime::cwd));
  runtime_template->Set(isolate, "exit",
                        v8::FunctionTemplate::New(isolate, Runtime::exit));
  runtime_template->Set(
      isolate, "promiseState",
      v8::FunctionTemplate::New(isolate, Runtime::promiseState));
  runtime_template->Set(isolate, "isProxy",
                        v8::FunctionTemplate::New(isolate, Runtime::isProxy));
  runtime_template->Set(isolate, "platform",
                        Pand::symbol(isolate, pd_get_platform()));
  runtime_template->Set(isolate, "version",
                        Pand::symbol(isolate, RUNTIME_VERSION));
  runtime_template->Set(isolate, "pid", Pand::value(isolate, pd_getpid()));
}

void Runtime::setArgv(v8::Local<v8::Context> context, int argc, char **argv) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::Local<v8::Object> runtime =
      context->Global()
          ->Get(context, Pand::symbol(isolate, "Runtime"))
          .ToLocalChecked()
          .As<v8::Object>();
  v8::Local<v8::Array> arr = v8::Array::New(isolate, argc);
  for (int i = 0; i < argc; ++i) {
    v8::Local<v8::String> arg = Pand::value(isolate, argv[i]);
    arr->Set(context, i, arg).FromJust();
  }

  runtime->Set(context, Pand::symbol(isolate, "argv"), arr).Check();
}

void Runtime::print(const v8::FunctionCallbackInfo<v8::Value> &args) {
  if (args.Length() < 1)
    return;
  v8::String::Utf8Value str(args.GetIsolate(), args[0]);
  std::cout << *str;
}

void Runtime::printerr(const v8::FunctionCallbackInfo<v8::Value> &args) {
  if (args.Length() < 1)
    return;
  v8::String::Utf8Value str(args.GetIsolate(), args[0]);
  std::cerr << *str;
}

void Runtime::cwd(const v8::FunctionCallbackInfo<v8::Value> &args) {
  args.GetReturnValue().Set(
      Pand::value(args.GetIsolate(), std::filesystem::current_path().string()));
}

void Runtime::exit(const v8::FunctionCallbackInfo<v8::Value> &args) {
  int status = 0;
  if (args.Length() == 1 && args[0]->IsNumber()) {
    status = args[0]
                 .As<v8::Number>()
                 ->Int32Value(args.GetIsolate()->GetCurrentContext())
                 .ToChecked();
  }
  std::exit(status);
}

void Runtime::getenv(const v8::FunctionCallbackInfo<v8::Value> &args) {
  assert(args.Length() == 1);
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (!args[0]->IsString()) {
    isolate->ThrowException(Pand::symbol(isolate, "Argument must be a string"));
    return;
  }

  v8::String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *value = std::getenv(*str);
  if (!value) {
    args.GetReturnValue().Set(v8::Null(isolate));
    return;
  }

  return args.GetReturnValue().Set(Pand::value(isolate, value));
}

void Runtime::bind(const v8::FunctionCallbackInfo<v8::Value> &args) {
  assert(args.Length() == 1);
  assert(args[0]->IsString());

  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::String::Utf8Value str(isolate, args[0]);
  std::string_view name = *str;
  v8::Local<v8::Object> exports = v8::Object::New(isolate);

  if (name == "timer") {
    Timer::initialize(exports);
  } else if (name == "tcp") {
    TcpStream::initialize(exports);
    TcpServer::initialize(exports);
  }

  args.GetReturnValue().Set(exports);
}

void Runtime::isProxy(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (args.Length() < 1 || !args[0]->IsObject()) {
    args.GetReturnValue().Set(Pand::boolean(isolate, false));
    return;
  }

  v8::Local<v8::Object> obj = args[0].As<v8::Object>();
  args.GetReturnValue().Set(Pand::boolean(isolate, obj->IsProxy()));
}

// important function for inspecting Promise objects
void Runtime::promiseState(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (args.Length() < 1 || !args[0]->IsPromise()) {
    args.GetReturnValue().Set(Pand::symbol(isolate, "unknown"));
    return;
  }

  v8::Local<v8::Promise> promise = args[0].As<v8::Promise>();
  v8::Promise::PromiseState state = promise->State();

  switch (state) {
  case v8::Promise::kPending:
    args.GetReturnValue().Set(Pand::symbol(isolate, "pending"));
    break;
  case v8::Promise::kFulfilled:
    args.GetReturnValue().Set(Pand::symbol(isolate, "resolved"));
    break;
  case v8::Promise::kRejected:
    args.GetReturnValue().Set(Pand::symbol(isolate, "rejected"));
    break;
  default:
    args.GetReturnValue().Set(Pand::symbol(isolate, "unknown"));
  }
}

} // namespace pand::core
