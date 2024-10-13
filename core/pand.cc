#include "pand.h"
#include <iostream>
#include <pandio.h>
#include <v8.h>

#include "mod.h"
#include "runtime.h"
#include "v8_utils.cc"

namespace pand::core {

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
  Mod::clearMods();
  Mod::clearResolveCache();
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete this->create_params.array_buffer_allocator;
}

Pand *Pand::get() {
  static Pand *instance = nullptr;
  if (!instance) {
    instance = new Pand();
  }
  return instance;
}

void Pand::run(const std::string &entryfile) {
  Pand::run(entryfile, 0, nullptr);
}

void Pand::run(const std::string &entryfile, int argc, char *argv) {
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

  v8::Local<v8::ObjectTemplate> runtime_template = v8::ObjectTemplate::New(isolate);
  Runtime::initialize(runtime_template, isolate);
  global->Set(v8_symbol(isolate, "Runtime"), runtime_template);

  v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
  v8::Context::Scope context_scope(context);
  v8::Isolate::Scope isolate_scope(isolate);

  isolate->SetHostInitializeImportMetaObjectCallback(Mod::setMeta);
  isolate->SetHostImportModuleDynamicallyCallback(Mod::dynamicImport);
  Mod::execInternal(isolate, "std:bootstrap");
  Mod::execScript(isolate, entryfile);

  pd_io_run(ctx);
}

} // namespace pand::core
