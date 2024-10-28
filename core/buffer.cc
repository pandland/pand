#include "buffer.h"
#include "errors.h"
#include "pand.h"

#include <algorithm>
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
      ->Set(context, Pand::symbol(isolate, "memcmp"),
            Pand::func(context, Buffer::memcmp))
      .ToChecked();
}

// sync function
void Buffer::fillRandom(const v8::FunctionCallbackInfo<v8::Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !Buffer::isBuffer(args[0])) {
    Errors::throwTypeException(isolate, "Invalid buffer");
    return;
  }

  v8::Local<v8::Uint8Array> buf = args[0].As<v8::Uint8Array>();
  int errcode = pd_random(Buffer::getBytes(buf), Buffer::getSize(buf));
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
