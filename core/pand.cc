#include "pand.h"
#include <exception>
#include <iostream>
#include <pandio.h>
#include <v8.h>

#include "errors.h"
#include "mod.h"
#include "runtime.h"

namespace pand::core {

static Pand *instance = nullptr;

Pand::Pand() {
  ctx = new pd_io_t;
  pd_io_init(ctx);

  platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  isolate = v8::Isolate::New(create_params);
}

Pand::~Pand() {
  Errors::clearPendingRejects();
  Mod::clearMods();
  Mod::clearResolveCache();
  tcpStreamConstructor.Reset();
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete this->create_params.array_buffer_allocator;
}

Pand *Pand::get() {
  if (!instance) {
    instance = new Pand();
  }
  return instance;
}

void Pand::exit(int status) {
  if (instance) {
    delete instance;
    instance = nullptr;
  }

  std::exit(status);
}

void Pand::run(const std::string &entryfile) {
  Pand::run(entryfile, 0, nullptr);
}

void Pand::run(const std::string &entryfile, int argc, char *argv) {
  // v8::Isolate::Scope isolate_scope(isolate);
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

  v8::Local<v8::ObjectTemplate> runtime_template =
      v8::ObjectTemplate::New(isolate);
  Runtime::initialize(runtime_template, isolate);
  global->Set(Pand::symbol(isolate, "Runtime"), runtime_template);

  v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
  v8::Context::Scope context_scope(context);

  isolate->SetCaptureStackTraceForUncaughtExceptions(true, 10);
  isolate->SetPromiseRejectCallback(Errors::promiseRejectedCallback);
  isolate->SetHostInitializeImportMetaObjectCallback(Mod::setMeta);
  isolate->SetHostImportModuleDynamicallyCallback(Mod::dynamicImport);
  Mod::execInternal(isolate, "std:bootstrap");
  Mod::execScript(isolate, entryfile);

  pd_io_run(ctx);
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
    Errors::throwCritical(try_catch.Exception());
  }
}

void Pand::setTcpStreamConstructor(v8::Local<v8::Function> constructor) {
  this->tcpStreamConstructor.Reset(isolate, constructor);
}

v8::Local<v8::Function> Pand::getTcpStreamConstructor() {
  return this->tcpStreamConstructor.Get(isolate);
}

} // namespace pand::core
