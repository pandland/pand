#include "transcoder.h"
#include "errors.h"
#include "pand.h"

namespace pand::core {

void Transcoder::decode(const v8::FunctionCallbackInfo<v8::Value> &args) {
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


v8::MaybeLocal<v8::String> Transcoder::decoder(v8::Isolate *isolate,
                                               const char *bytes, size_t len,
                                               int option) {
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
    size_t size = bytes::hex_length_from_binary(len);
    std::unique_ptr<char[]> str = std::make_unique<char[]>(size);
    bytes::binary_to_hex(bytes, len, str.get());
    auto resource = ExternOneByte::from(str.release(), size);

    return v8::String::NewExternalOneByte(isolate, resource);
  }
  case BASE64: {
    size_t size = simdutf::base64_length_from_binary(len);
    std::unique_ptr<char[]> str = std::make_unique<char[]>(size);
    simdutf::binary_to_base64(bytes, len, str.get());
    auto resource = ExternOneByte::from(str.release(), size);

    return v8::String::NewExternalOneByte(isolate, resource);
  }
  case BASE64URL: {
    size_t size = simdutf::base64_length_from_binary(len, simdutf::base64_url);
    std::unique_ptr<char[]> str = std::make_unique<char[]>(size);
    simdutf::binary_to_base64(bytes, len, str.get(), simdutf::base64_url);
    auto resource = ExternOneByte::from(str.release(), size);

    return v8::String::NewExternalOneByte(isolate, resource);
  }
  }

  return {};
}

void Transcoder::encode(const v8::FunctionCallbackInfo<v8::Value> &args) {}

void Transcoder::encoder(v8::Isolate *isolate, const char *bytes, size_t len,
                         int option) {}

void Transcoder::initialize(v8::Local<v8::Object> exports) {
  Pand *pand = Pand::get();
  v8::Isolate *isolate = pand->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  // set constants to share between JS and C++ worlds
  exports
      ->Set(context, Pand::symbol(isolate, "UTF8"),
            Pand::integer(isolate, Transcoder::UTF8))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "ASCII"),
            Pand::integer(isolate, Transcoder::ASCII))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "LATIN"),
            Pand::integer(isolate, Transcoder::LATIN))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "BASE64"),
            Pand::integer(isolate, Transcoder::BASE64))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "BASE64URL"),
            Pand::integer(isolate, Transcoder::BASE64URL))
      .ToChecked();

  exports
      ->Set(context, Pand::symbol(isolate, "HEX"),
            Pand::integer(isolate, Transcoder::HEX))
      .ToChecked();

  // set functions
  exports
      ->Set(context, Pand::symbol(isolate, "decode"),
            Pand::func(context, Transcoder::decode))
      .ToChecked();
}

} // namespace pand::core
