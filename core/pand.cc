#include "pand.h"
#include "pandio/core.h"
#include <iostream>
#include <pandio.h>
#include <v8.h>

namespace pand::core {

Pand::Pand() {
  ctx  = new pd_io_t;
  pd_io_init(ctx);

  platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  isolate = v8::Isolate::New(create_params);
}

Pand::~Pand() {
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

void Pand::run(const std::string& entryfile) {
  Pand::run(entryfile, 0, nullptr);
}

void Pand::run(const std::string& entryfile, int argc, char *argv) {
  v8::HandleScope handle_scope(isolate);
  std::cout << "Run " << entryfile << std::endl; 

  pd_io_run(ctx);
}

} // namespace pand::core
