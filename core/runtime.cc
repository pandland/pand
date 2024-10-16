#include "runtime.h"
#include "tcp.h"
#include "tcp_server.h"
#include "timer.h"
#include "v8_utils.cc"
#include <filesystem>
#include <iostream>
#include <pandio.h>

#define RUNTIME_VERSION "v0.0.0"

namespace pand::core {

void Runtime::initialize(v8::Local<v8::ObjectTemplate> runtime_template,
                         v8::Isolate *isolate) {
  runtime_template->Set(isolate, "print",
                        v8::FunctionTemplate::New(isolate, Runtime::print));
  runtime_template->Set(isolate, "env",
                        v8::FunctionTemplate::New(isolate, Runtime::env));
  runtime_template->Set(isolate, "bind",
                        v8::FunctionTemplate::New(isolate, Runtime::bind));
  runtime_template->Set(isolate, "cwd",
                        v8::FunctionTemplate::New(isolate, Runtime::cwd));
  runtime_template->Set(isolate, "exit",
                        v8::FunctionTemplate::New(isolate, Runtime::exit));
  runtime_template->Set(isolate, "platform",
                        v8_symbol(isolate, pd_get_platform()));
  runtime_template->Set(isolate, "version",
                        v8_symbol(isolate, RUNTIME_VERSION));
  runtime_template->Set(isolate, "pid", v8::Number::New(isolate, pd_getpid()));
}

void Runtime::print(const v8::FunctionCallbackInfo<v8::Value> &args) {
  if (args.Length() < 1)
    return;
  v8::String::Utf8Value str(args.GetIsolate(), args[0]);
  std::cout << *str;
}

void Runtime::cwd(const v8::FunctionCallbackInfo<v8::Value> &args) {
  args.GetReturnValue().Set(
      v8_value(args.GetIsolate(), std::filesystem::current_path().string()));
}

void Runtime::exit(const v8::FunctionCallbackInfo<v8::Value> &args) {
  int status = 0;
  if (args.Length() == 1 && args[0]->IsNumber()) {
    status = args[0]
                 .As<v8::Number>()
                 ->Int32Value(args.GetIsolate()->GetCurrentContext())
                 .ToChecked();
  }
  Pand::get()->exit(status);
}

void Runtime::env(const v8::FunctionCallbackInfo<v8::Value> &args) {
  assert(args.Length() == 1);
  v8::Isolate *isolate = args.GetIsolate();

  if (!args[0]->IsString()) {
    isolate->ThrowException(v8::String::NewFromUtf8(isolate,
                                                    "Argument must be a string",
                                                    v8::NewStringType::kNormal)
                                .ToLocalChecked());
    return;
  }

  v8::String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *value = std::getenv(*str);
  if (!value) {
    args.GetReturnValue().Set(v8::Null(isolate));
    return;
  }

  return args.GetReturnValue().Set(
      v8::String::NewFromUtf8(isolate, value).ToLocalChecked());
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

} // namespace pand::core
