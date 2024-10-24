#include "buffer.h"
#include "errors.h"
#include "pand.h"

#include <cstdio>
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

} // namespace pand::core
