#include "buffer.h"
#include "errors.h"
#include "pand.h"

#include <algorithm>
#include <cstring>
#include <pandio.h>

namespace pand::core {

void Buffer::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  exports
      ->Set(context, Pand::symbol(isolate, "fillRandom"),
            Pand::func(context, Buffer::fillRandom))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "decode"),
            Pand::func(context, Buffer::decode))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "fromString"),
            Pand::func(context, Buffer::fromString))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "memcmp"),
            Pand::func(context, Buffer::memcmp))
      .ToChecked();
}

void Buffer::fromString(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (args.Length() < 1 || !args[0]->IsString()) {
    Errors::throwTypeException(isolate, "Invalid String");
    return;
  }

  v8::Local<v8::String> str = args[0].As<v8::String>();
  size_t len = str->Utf8Length(isolate);

  v8::Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, len);
  char *bytes = static_cast<char *>(buf->Data());
  str->WriteUtf8(isolate, bytes);

  args.GetReturnValue().Set(buf);
}

// currenty only basic utf8 decoder
void Buffer::decode(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (args.Length() < 1 || !args[0]->IsArrayBuffer()) {
    Errors::throwTypeException(isolate, "Invalid buffer");
    return;
  }

  v8::Local<v8::ArrayBuffer> buf = args[0].As<v8::ArrayBuffer>();
  char *bytes = static_cast<char *>(buf->Data());
  auto result = v8::String::NewFromUtf8(
      isolate, bytes, v8::NewStringType::kNormal, buf->ByteLength());
  if (result.IsEmpty()) {
    Errors::throwTypeException(isolate, "Unable to decode buffer");
    return;
  }

  args.GetReturnValue().Set(result.ToLocalChecked());
}

// sync function
void Buffer::fillRandom(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !args[0]->IsArrayBuffer()) {
    Errors::throwTypeException(isolate, "Invalid buffer");
    return;
  }

  v8::Local<v8::ArrayBuffer> buf = args[0].As<v8::ArrayBuffer>();
  int errcode = pd_random(buf->Data(), buf->ByteLength());
  if (errcode < 0) {
    isolate->ThrowException(Pand::makeSystemError(isolate, errcode));
  }
}

void Buffer::memcmp(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  if (args.Length() < 2 || !args[0]->IsArrayBuffer() ||
      !args[1]->IsArrayBuffer()) {
    Errors::throwTypeException(isolate, "Invalid buffers");
    return;
  }

  v8::Local<v8::ArrayBuffer> buf1 = args[0].As<v8::ArrayBuffer>();
  v8::Local<v8::ArrayBuffer> buf2 = args[0].As<v8::ArrayBuffer>();

  size_t n = std::min(buf1->ByteLength(), buf2->ByteLength());
  int result = std::memcmp(buf1->Data(), buf2->Data(), n);

  args.GetReturnValue().Set(result);
}

} // namespace pand::core
