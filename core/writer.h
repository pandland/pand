#pragma once
#include <pandio.h>
#include <v8.h>

namespace pand::core {
/* Writer is mechanism to make async writes with buffers managed by v8 GC without
being garbage collected before write */
struct Writer {
  pd_write_t op;
  v8::Persistent<v8::Value> value;

  Writer(char *buf, size_t len) {
    pd_write_init(&op, buf, len, Writer::afterWrite);
    op.udata = this;
  }

  ~Writer() { value.Reset(); }

  void setArrayBuffer(v8::Isolate *isolate, v8::Local<v8::ArrayBuffer> ab) {
    value.Reset(isolate, ab);
  }

  void setBufferView(v8::Isolate *isolate, v8::Local<v8::Uint8Array> ui) {
    value.Reset(isolate, ui);
  }

  static void afterWrite(pd_write_t *op, int written) {
    Writer *writer = static_cast<Writer *>(op->udata);
    delete writer;
  }
};

} // namespace pand::core
