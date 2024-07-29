#include <swc_transform.h>
#include <stdio.h>
#include <string.h>

#include <v8.h>
#include <libplatform/libplatform.h>
#include <fstream>
#include <sstream>
#include <iostream>

std::string ReadFile(const std::string& filename) {
    printf("Reading file...\n");
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ExecuteScript(v8::Isolate* isolate, const std::string& script_source) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);

    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, script_source.c_str()).ToLocalChecked();
    v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
    script->Run(context).ToLocalChecked();
}

int main(int argc, char* argv[]) {
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        std::string ts_source = ReadFile("sample.ts");
        //printf("Original source:\n%s\n", ts_source.c_str());
        std::string script_source = (char *)transform_sync((unsigned char *)ts_source.c_str(), ts_source.length());
        printf("Executing:\n%s\n", script_source.c_str());
        ExecuteScript(isolate, script_source);
    }

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    return 0;
}
