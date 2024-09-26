#include <v8.h>
#include <libplatform/libplatform.h>
#include <filesystem>
#include <pandio.h>

#include "loader.cc"
#include "pandio/core.h"
#include "timers.cc"
#include "io.cc"
#include "tcp.cc"

#include <thread>
#include <chrono>

#define RUNTIME_VERSION "v0.0.0"

namespace fs = std::filesystem;
namespace runtime {


void PrintMemoryUsage(v8::Isolate* isolate) {
    v8::HeapStatistics heap_stats;
    isolate->GetHeapStatistics(&heap_stats);
    std::cout << "--------------------------------------------------------\n";
    std::cout << "Total heap size: " << heap_stats.total_heap_size() << " bytes\n";
    std::cout << "Total heap size executable: " << heap_stats.total_heap_size_executable() << " bytes\n";
    std::cout << "Total physical size: " << heap_stats.total_physical_size() << " bytes\n";
    std::cout << "Total available size: " << heap_stats.total_available_size() << " bytes\n";
    std::cout << "Used heap size: " << heap_stats.used_heap_size() << " bytes\n";
    std::cout << "Heap size limit: " << heap_stats.heap_size_limit() << " bytes\n";
    std::cout << "Malloced memory: " << heap_stats.malloced_memory() << " bytes\n";
    std::cout << "Peak malloced memory: " << heap_stats.peak_malloced_memory() << " bytes\n";
    std::cout << "Number of native contexts: " << heap_stats.number_of_native_contexts() << "\n";
    std::cout << "Number of detached contexts: " << heap_stats.number_of_detached_contexts() << "\n";
    std::cout << "--------------------------------------------------------\n";
}

void MonitorMemoryUsage(v8::Isolate* isolate) {
    while (true) {
        PrintMemoryUsage(isolate);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

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

  void start(const char *entryfile, int argc, char* argv[]) {
    IO::create();
    pd_io_t *ctx = IO::get()->ctx;

    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

    std::string entrypath = fs::absolute(entryfile).string();

    v8::Local<v8::ObjectTemplate> runtime_template = v8::ObjectTemplate::New(isolate);
    Runtime::initialize(runtime_template, isolate);
    global->Set(v8_symbol(isolate, "Runtime"), runtime_template);

    Timers::initialize(ctx);
    Timers::instance()->setup(global, isolate);

    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
    v8::Context::Scope context_scope(context);
    v8::Isolate::Scope isolate_scope(isolate);

    v8::Local<v8::Object> runtime_obj = context->Global()->Get(context, v8_symbol(isolate, "Runtime")).ToLocalChecked().As<v8::Object>();
    runtime_obj->Set(context, v8_symbol(isolate, "argv"), init_argv(argc, argv, isolate, context)).Check();

    Loader loader;
    isolate->SetHostImportModuleDynamicallyCallback(Loader::dynamic_load);
    isolate->SetHostInitializeImportMetaObjectCallback(Loader::set_meta);
    loader.execute(isolate, context, "std:bootstrap");
    loader.execute(isolate, context, entrypath);

    //std::thread memory_monitor(MonitorMemoryUsage, isolate);
    //memory_monitor.detach();

    IO::get()->run();
  }

  static void initialize(v8::Local<v8::ObjectTemplate> runtime_template, v8::Isolate* isolate) {
    runtime_template->Set(isolate, "print", v8::FunctionTemplate::New(isolate, Runtime::print));
    runtime_template->Set(isolate, "printerr", v8::FunctionTemplate::New(isolate, Runtime::printerr));
    runtime_template->Set(isolate, "env", v8::FunctionTemplate::New(isolate, Runtime::env));
    runtime_template->Set(isolate, "bind", v8::FunctionTemplate::New(isolate, Runtime::bind));
    runtime_template->Set(isolate, "cwd", v8::FunctionTemplate::New(isolate, Runtime::cwd));
    runtime_template->Set(isolate, "exit", v8::FunctionTemplate::New(isolate, Runtime::exit_process));
    runtime_template->Set(isolate, "platform", v8_symbol(isolate, Runtime::get_platform()));
    runtime_template->Set(isolate, "version", v8_symbol(isolate, RUNTIME_VERSION));
    runtime_template->Set(isolate, "pid", v8::Number::New(isolate, Runtime::get_pid()));
  }

  static v8::Local<v8::Array> init_argv(int argc, char* argv[], v8::Isolate* isolate, v8::Local<v8::Context> context) {
    v8::Local<v8::Array> js_argv = v8::Array::New(isolate, argc);
    for (int i = 0; i < argc; ++i) {
        v8::Local<v8::String> arg = v8::String::NewFromUtf8(isolate, argv[i], v8::NewStringType::kNormal).ToLocalChecked();
        js_argv->Set(context, i, arg).FromJust();
    }

    return js_argv;
  }

  static void print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) return;
    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    std::cout << *str;
  }

  static void printerr(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) return;
    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    std::cerr << *str;
  }

  static int get_pid() {
    return pd_getpid();
  }

  static void cwd(const v8::FunctionCallbackInfo<v8::Value> &args) {
    args.GetReturnValue().Set(v8_value(args.GetIsolate(), std::filesystem::current_path()));
  }

  static void exit_process(const v8::FunctionCallbackInfo<v8::Value> &args) {
    int status = 0;
    if (args.Length() == 1 && args[0]->IsNumber()) {
      status = args[0].As<v8::Number>()->Int32Value(args.GetIsolate()->GetCurrentContext()).ToChecked();
    }
    exit(status);
  }

  static const char *get_platform() {
    return pd_get_platform();
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
