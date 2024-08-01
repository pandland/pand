#include <v8.h>
#include <libplatform/libplatform.h>
#include "globals.cc"
#include "loader.cc"
#include "timers.cc"
#include "io.cc"
#include "tcp.cc"

#include <luxio.h>

namespace runtime {

/* entrypoint class for JS runtime */
class Runtime {
  std::unique_ptr<v8::Platform> platform;
  v8::Isolate::CreateParams create_params;
  v8::Isolate* isolate;

public:
  Runtime() {
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate = v8::Isolate::New(create_params);
  }

  ~Runtime() {
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete this->create_params.array_buffer_allocator;
  }

  void start(const char *entryfile) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    
    IO::create();
    lx_io_t *ctx = IO::get()->ctx;

    Globals globals(global, isolate);
    TcpServer::initialize(global, isolate);
    Timers::initialize(ctx);
    Timers::instance()->setup(global, isolate);
    globals.setup();

    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
    TcpStream::initialize(global, isolate, context);

    v8::Context::Scope context_scope(context);
    v8::Isolate::Scope isolate_scope(isolate);

    runtime::Loader entryscript(entryfile);
    entryscript.execute(isolate, context);

    lx_run(ctx);
  }
};

}
