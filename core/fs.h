#pragma once
#include <v8.h>
#include <pandio.h>

namespace pand::core {

class File {
public:
  pd_fd_t fd;
  bool isClosed = true;
  bool isClosing = false;
  v8::Persistent<v8::Value> obj;

  File(v8::Local<v8::Object> obj) {
    this->isClosed = true;
    this->obj.Reset(obj->GetIsolate(), obj);
    obj->SetAlignedPointerInInternalField(0, this);
    this->obj.SetWeak(this, File::onCleanup, v8::WeakCallbackType::kParameter);
  }

  ~File() {
    this->obj.Reset();
  }

  static void initialize(v8::Local<v8::Object>);

  static void constructor(const v8::FunctionCallbackInfo<v8::Value> &);

  static void onCleanup(const v8::WeakCallbackInfo<File> &);

  static void open(const v8::FunctionCallbackInfo<v8::Value> &);

  static void read(const v8::FunctionCallbackInfo<v8::Value> &);

  static void write(const v8::FunctionCallbackInfo<v8::Value> &);

  static void close(const v8::FunctionCallbackInfo<v8::Value> &);

};

}
