#include <v8.h>
#include <libplatform/libplatform.h>
#include <filesystem>
#include <luxio.h>

#include "loader.cc"
#include "timers.cc"
#include "io.cc"
#include "tcp.cc"

#define RUNTIME_VERSION "v0.0.0"

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
    v8::Local<v8::ObjectTemplate> runtime_template = v8::ObjectTemplate::New(isolate);
    runtime_template->Set(isolate, "print", v8::FunctionTemplate::New(isolate, Runtime::print));
    runtime_template->Set(isolate, "env", v8::FunctionTemplate::New(isolate, Runtime::env));
    runtime_template->Set(isolate, "bind", v8::FunctionTemplate::New(isolate, Runtime::bind));
    runtime_template->Set(isolate, "cwd", v8::FunctionTemplate::New(isolate, Runtime::cwd));
    runtime_template->Set(isolate, "platform", v8_symbol(isolate, Runtime::get_platform().c_str()));
    runtime_template->Set(isolate, "version", v8_symbol(isolate, RUNTIME_VERSION));
    runtime_template->Set(isolate, "pid", v8::Number::New(isolate, Runtime::get_pid()));
    global->Set(v8::String::NewFromUtf8(isolate, "Runtime", v8::NewStringType::kNormal).ToLocalChecked(), runtime_template);
  }

  static void print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) return;
    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    std::cout << *str;
  }

  static int get_pid() {
    #if defined(_WIN32) || defined(_WIN64)
        return GetCurrentProcessId();
    #else
        return getpid();
    #endif
  }

  static void cwd(const v8::FunctionCallbackInfo<v8::Value> &args) {
    args.GetReturnValue().Set(v8_value(args.GetIsolate(), std::filesystem::current_path()));
  }

  static std::string get_platform() {
    #if defined(_WIN32) || defined(_WIN64)
        return "win32";
    #elif defined(__APPLE__) || defined(__MACH__)
        return "darwin";
    #elif defined(__linux__)
        return "linux";
    #elif defined(__FreeBSD__)
        return "freebsd";
    #elif defined(__OpenBSD__)
        return "openbsd";
    #elif defined(__unix__)
        return "unix";
    #else
        return "unknown";
    #endif
  }

  static void env(const v8::FunctionCallbackInfo<v8::Value> &args) {
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
