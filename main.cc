#include <swc_transform.h>
#include <stdio.h>
#include <string.h>

#include <v8.h>
#include <libplatform/libplatform.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <event.h>
#include <luxio.h>

std::string ReadFile(const std::string& filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ExecuteScript(v8::Isolate* isolate, v8::Local<v8::Context> context, const std::string& script_source) {
    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, script_source.c_str()).ToLocalChecked();
    v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

    v8::Local<v8::Value> result;
    script->Run(context).ToLocalChecked();
}

void Println(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() < 1) return;
    v8::String::Utf8Value str(args.GetIsolate(), args[0]);
    std::cout << *str << std::endl;
}

void timer_once(lx_timer_t *timer) {
    printf("Should happen only once\n");
}

void timer_test() {
    lx_io_t ctx = lx_init();
    lx_timer_t timer;
    lx_timer_init(&ctx, &timer);
    lx_timer_start(&timer, timer_once, 1000);

    lx_run(&ctx);
}

int main(int argc, char* argv[]) {
    timer_test();

    const char* flags = "--max-old-space-size=4096";
    v8::V8::SetFlagsFromString(flags);

    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);

    if (argc < 2) {
        printf("Expected filename arg\n");
        return 1;
    }

    char *filename = argv[1];

    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
        global->Set(isolate, "println", v8::FunctionTemplate::New(isolate, Println));
        v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
        v8::Context::Scope context_scope(context);

        v8::Isolate::Scope isolate_scope(isolate);
        std::string ts_source = ReadFile(filename);
        std::string script_source = (char *)transform_sync((unsigned char *)ts_source.c_str(), ts_source.length());
        ExecuteScript(isolate, context, script_source);
    }

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    return 0;
}
