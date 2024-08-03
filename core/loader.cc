#include <swc_transform.h>
#include <string>
#include <sstream>
#include <fstream>

#include <v8.h>

namespace runtime {

class Loader {
private:
  std::string filename;
  std::string src;

  void load() {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    src = buffer.str();
  }

  void transform() {
    src = (char *)transform_sync((unsigned char *)src.c_str(), src.length());
  }
public:
  Loader(std::string &filename): filename(filename) {}
  Loader(const char *filename): filename(filename) {}

  void execute(v8::Isolate* isolate, v8::Local<v8::Context> context) {
    load();
    transform();

    if (src.empty()) {
      return;
    }

    v8::ScriptOrigin origin(v8::String::NewFromUtf8(isolate, filename.c_str()).ToLocalChecked());
    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, src.c_str()).ToLocalChecked();
    v8::Local<v8::Script> script = v8::Script::Compile(context, source, &origin).ToLocalChecked();

    script->Run(context).ToLocalChecked();
  }
};

}