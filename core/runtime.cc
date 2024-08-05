#include <v8.h>
#include <libplatform/libplatform.h>
#include <filesystem>
#include <luxio.h>

#include "loader.cc"
#include "timers.cc"
#include "io.cc"
#include "tcp.cc"

namespace fs = std::filesystem;
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

    std::string entrypath = fs::absolute(entryfile).string();
    global->Set(isolate, "__entryfile", v8::String::NewFromUtf8(isolate, entrypath.c_str()).ToLocalChecked());
    IO::create();
    lx_io_t *ctx = IO::get()->ctx;

    Runtime::initialize(global, isolate);
    Timers::initialize(ctx);
    Timers::instance()->setup(global, isolate);

    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
    //TcpStream::initialize(global, isolate, context);

    v8::Context::Scope context_scope(context);
    v8::Isolate::Scope isolate_scope(isolate);

    Loader loader;
    isolate->SetHostImportModuleDynamicallyCallback(Loader::dynamic_load);
    isolate->SetHostInitializeImportMetaObjectCallback(Loader::set_meta);
    loader.execute(isolate, context, "std:bootstrap");
    loader.execute(isolate, context, entrypath);

    IO::get()->run();
  }

  static void initialize(v8::Local<v8::ObjectTemplate> global, v8::Isolate* isolate) {
    global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, Runtime::print));
    global->Set(isolate, "env", v8::FunctionTemplate::New(isolate, Runtime::env));
    global->Set(isolate, "bind", v8::FunctionTemplate::New(isolate, Runtime::bind));
  }

  static void print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) return;
    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    std::cout << *str;
  }

  static void env(const v8::FunctionCallbackInfo<v8::Value>& args) {
    assert(args.Length() == 1);
    v8::Isolate *isolate = args.GetIsolate();

    if (!args[0]->IsString()) {
      isolate->ThrowException(v8::String::NewFromUtf8(isolate, "Argument must be a string", v8::NewStringType::kNormal).ToLocalChecked());
      return;
    }

    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *value = std::getenv(*str);


    if (!value) {
       args.GetReturnValue().Set(v8::Null(isolate));
       return;
    }

    return args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, value).ToLocalChecked());
  }

  static void bind(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.Length() == 1);
    assert(args[0]->IsString());

    v8::Isolate *isolate = args.GetIsolate();
    v8::String::Utf8Value str(isolate, args[0]);
    std::string name = *str;

    v8::Local<v8::Object> obj = v8::Object::New(isolate);

    // TODO: create internal modules registry, create some export macro
    if (name == "tcp") {
      TcpStream::initialize(obj, isolate, isolate->GetCurrentContext());
      TcpServer::initialize(obj, isolate, isolate->GetCurrentContext());
    }

    args.GetReturnValue().Set(obj);
  }
};

}
