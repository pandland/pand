#include "pand.h"
#include <cstdio>
#include <pandio.h>
#include <string_view>
#include <v8.h>

#include "errors.h"
#include "loader.h"
#include "runtime.h"

namespace pand::core {

// thread_local, because we will add worker threads in the future
thread_local static Pand *instance = nullptr;

Pand::Pand() {
  ctx = new pd_io_t;
  pd_io_init(ctx);
  pd_threadpool_init(4);  // it runs only once btw, so safe to call even inside worker threads
  pd_set_after_tick(ctx, Errors::checkPendingErrors);
  //v8::V8::SetFlagsFromString("--max-old-space-size=2048 --trace-gc");
  platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  isolate = v8::Isolate::New(create_params);
}

Pand::~Pand() {
  Errors::clearPendingRejects();
  Loader::cleanup();
  tcpStreamConstructor.Reset();
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete this->create_params.array_buffer_allocator;
}

Pand *Pand::get() {
  static Pand instance;
  return &instance;
}

void Pand::run(const std::string &entryfile) {
  Pand::run(entryfile, 0, nullptr);
}

void Pand::run(const std::string &entryfile, int argc, char **argv) {
  v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

  v8::Local<v8::ObjectTemplate> runtime_template =
      v8::ObjectTemplate::New(isolate);
  Runtime::initialize(runtime_template, isolate);
  global->Set(Pand::symbol(isolate, "Runtime"), runtime_template);
  v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
  v8::Context::Scope context_scope(context);
  Runtime::setArgv(context, argc, argv);

  isolate->SetCaptureStackTraceForUncaughtExceptions(true, 10);
  isolate->SetPromiseRejectCallback(Errors::promiseRejectedCallback);
  isolate->SetHostInitializeImportMetaObjectCallback(Loader::setMeta);
  isolate->SetHostImportModuleDynamicallyCallback(Loader::dynamicImport);
  Loader::execInternal(isolate, "std:bootstrap");
  Loader::execScript(isolate, entryfile);

  pd_io_run(ctx);
  pd_threadpool_end();
  Errors::checkPendingErrors(ctx);
}

void Pand::makeCallback(v8::Local<v8::Object> &obj, v8::Isolate *isolate,
                        const char *funcName, v8::Local<v8::Value> *argv,
                        size_t argc) {
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Function> callback =
      obj->Get(context, Pand::symbol(isolate, funcName))
          .ToLocalChecked()
          .As<v8::Function>();

  if (callback.IsEmpty()) {
    return;
  }

  v8::TryCatch try_catch(isolate);
  auto result = callback->Call(context, v8::Undefined(isolate), argc, argv);
  if (try_catch.HasCaught()) {
    Errors::reportUncaught(try_catch.Exception());
  }
}

v8::Local<v8::Value> Pand::makeSystemError(v8::Isolate *isolate, int err) {
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::String> code = Pand::symbol(isolate, pd_errname(err));
  v8::Local<v8::String> message = Pand::value(isolate, pd_errstr(err));

  v8::Local<v8::Value> error = v8::Exception::Error(message);
  v8::Local<v8::Object> obj = error.As<v8::Object>();

  obj->Set(context, Pand::symbol(isolate, "code"), code).ToChecked();

  return error;
}

void Pand::setTcpStreamConstructor(v8::Local<v8::Function> constructor) {
  this->tcpStreamConstructor.Reset(isolate, constructor);
}

v8::Local<v8::Function> Pand::getTcpStreamConstructor() {
  return this->tcpStreamConstructor.Get(isolate);
}

} // namespace pand::core
