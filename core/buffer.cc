#include "buffer.h"
#include "errors.h"
#include "pand.h"

#include "utils/hex.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <pandio.h>
#include <simdutf.h>

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

enum DecodeOption { UTF8 = 1, ASCII = 2, BASE64 = 3, LATIN = 4, HEX = 5 };

v8::MaybeLocal<v8::String> decoder(v8::Isolate *isolate, const char *bytes,
                                   size_t len, int option) {
  switch (option) {

  case ASCII:
  case LATIN:
    return v8::String::NewFromOneByte(
        isolate, reinterpret_cast<const unsigned char *>(bytes),
        v8::NewStringType::kNormal, len);
  case UTF8:
    return v8::String::NewFromUtf8(isolate, bytes, v8::NewStringType::kNormal,
                                   len);
  case HEX: {
    size_t size = hex::hex_length_from_binary(len);
    char *str = new char[size];  // idk how to pass ownership of these bytes to v8
    hex::binary_to_hex(bytes, len, str);
    auto result = v8::String::NewFromOneByte(
        isolate, reinterpret_cast<const uint8_t *>(str),
        v8::NewStringType::kNormal, size);
    delete[] str; 

    return result;
  }
  case BASE64: {
    size_t size = simdutf::base64_length_from_binary(len);
    char *str =
        new char[size];
    simdutf::binary_to_base64(bytes, len, str);
    auto result = v8::String::NewFromOneByte(
        isolate, reinterpret_cast<const uint8_t *>(str),
        v8::NewStringType::kNormal, size);
    delete[] str;

    return result;
  }
  }

  return {};
}

// currenty only basic utf8 decoder
void Buffer::decode(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  if (args.Length() < 2 || !args[0]->IsArrayBuffer() || !args[1]->IsNumber()) {
    Errors::throwTypeException(isolate, "Invalid arguments");
    return;
  }

  v8::Local<v8::ArrayBuffer> buf = args[0].As<v8::ArrayBuffer>();
  int option = args[1]->Int32Value(isolate->GetCurrentContext()).ToChecked();
  char *bytes = static_cast<char *>(buf->Data());

  v8::MaybeLocal<v8::String> result =
      decoder(isolate, bytes, buf->ByteLength(), option);
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
  v8::Local<v8::ArrayBuffer> buf2 = args[1].As<v8::ArrayBuffer>();

  size_t n = std::min(buf1->ByteLength(), buf2->ByteLength());
  int result = std::memcmp(buf1->Data(), buf2->Data(), n);

  args.GetReturnValue().Set(result);
}

} // namespace pand::core
