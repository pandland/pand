#pragma once
#include <buffer.h>
#include <iostream>
#include <pandio.h>
#include <v8.h>

namespace pand::core {
/* Writer is mechanism to make async writes with buffers managed by v8 GC
without being garbage collected before write */
template <typename TStream> struct Writer {
  TStream *stream;
  void (*onWrite)(TStream *, int, size_t);
  pd_write_t op;
  v8::Persistent<v8::Value> value;

  Writer(TStream *stream, v8::Local<v8::Uint8Array> data,
         void (*onWrite)(TStream *, int, size_t)) {
    value.Reset(data->GetIsolate(), data);
    char *buf = Buffer::getBytes(data);
    size_t len = Buffer::getSize(data);
    pd_write_init(&op, buf, len, Writer::afterWrite);
    this->stream = stream;
    this->onWrite = onWrite;
    op.udata = this;
  }

  ~Writer() { value.Reset(); }

  static void afterWrite(pd_write_t *op, int status) {
    Writer *writer = static_cast<Writer *>(op->udata);
    if (writer->onWrite) {
      // pandio notifies only when full buffer is written (or when we get error)
      writer->onWrite(writer->stream, status, op->data.len);
    }

    delete writer;
  }
};

} // namespace pand::core
