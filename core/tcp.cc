#pragma once

#include <v8.h>
#include <luxio.h>
#include <assert.h>

#include <string.h>
#include "io.cc"
#include "v8_utils.cc"

namespace runtime {

class TcpStream {
public:
  lx_connection_t *conn;
  lx_timer_t timeout;
  v8::Persistent<v8::Object> obj;

  TcpStream(v8::Isolate *isolate, v8::Local<v8::Object> obj) {
    this->obj.Reset(isolate, obj);
    lx_timer_init(IO::get()->ctx, &this->timeout);
    this->timeout.data = this;
  }

  ~TcpStream() {
    this->obj.Reset();
  }

  static v8::Persistent<v8::Function> streamConstructor;

  static void initialize(v8::Local<v8::Object> exports, v8::Isolate *isolate, v8::Local<v8::Context> context) {
    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, TcpStream::constructor);
    t->SetClassName(v8::String::NewFromUtf8(isolate, "TcpStream").ToLocalChecked());
    t->InstanceTemplate()->SetInternalFieldCount(1);

    v8::Local<v8::FunctionTemplate> writeTemplate = v8::FunctionTemplate::New(isolate, TcpStream::write);
    t->PrototypeTemplate()->Set(isolate, "write", writeTemplate);

    v8::Local<v8::FunctionTemplate> closeTemplate = v8::FunctionTemplate::New(isolate, TcpStream::close);
    t->PrototypeTemplate()->Set(isolate, "close", closeTemplate);
    
    v8::Local<v8::Function> func = t->GetFunction(context).ToLocalChecked();
    TcpStream::streamConstructor.Reset(isolate, func);

    exports->Set(context, v8::String::NewFromUtf8(isolate, "TcpStream").ToLocalChecked(), func).ToChecked();
  }

  static void constructor(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.IsConstructCall());
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    TcpStream *stream = new TcpStream(isolate, args.This());
    args.This()->SetAlignedPointerInInternalField(0, stream);
  }

  static void write(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.Length() == 1);
    
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);

    v8::String::Utf8Value str(isolate, args[0]);
    TcpStream *stream = static_cast<TcpStream*>(args.This()->GetAlignedPointerFromInternalField(0));
    char *buf = strdup(*str);

    lx_write_t *write_op = lx_write_alloc(buf, str.length());
    lx_write(write_op, stream->conn, TcpStream::handle_write);
  }

  static void handle_write(lx_write_t *write_op, int status) {
    free((void*)write_op->buf);
    free(write_op);
  }

  static void close(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.Length() == 0);
    TcpStream *stream = static_cast<TcpStream*>(args.This()->GetAlignedPointerFromInternalField(0));

    lx_close(stream->conn);
  }

  /* handlers for event loop */
  static void handle_data(lx_connection_t *conn) {
    TcpStream *stream = static_cast<TcpStream*>(conn->data);
    
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Function> callback =
      stream->obj.Get(isolate)->Get(context, v8_symbol(isolate, "onread")).ToLocalChecked().As<v8::Function>();
    if (callback.IsEmpty()) {
      return;
    }

    char buf[1024];
    ssize_t res =  recv(conn->fd, buf, 1024, 0);
    if (res <= 0) {
      lx_close(conn);
      return;
    }

    v8::Local<v8::Value> args[1] = {v8::String::NewFromUtf8(isolate, buf, v8::NewStringType::kNormal, res).ToLocalChecked()};
    callback->Call(context, v8::Undefined(isolate), 1, args).ToLocalChecked();
  }

  static void handle_close(lx_connection_t *conn) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    TcpStream *stream = static_cast<TcpStream*>(conn->data);
    lx_timer_stop(&stream->timeout);
    v8::Local<v8::Function> callback =
      stream->obj.Get(isolate)->Get(context, v8_symbol(isolate, "onclose")).ToLocalChecked().As<v8::Function>();
    callback->Call(context, v8::Undefined(isolate), 0, {}).ToLocalChecked();
    delete stream;
  }

  static void handle_timeout(lx_timer_t *timer) {
    TcpStream *stream = static_cast<TcpStream*>(timer->data);
    lx_close(stream->conn);
  }
};

v8::Persistent<v8::Function> TcpStream::streamConstructor;

class TcpServer {
  v8::Global<v8::Function> callback;
  int port;

  static const int64_t kDefaultTimeout = 5 * 1000;
public:
  static void initialize(v8::Local<v8::Object> exports, v8::Isolate *isolate, v8::Local<v8::Context> context)  {
    v8::Local<v8::Function> func = v8::FunctionTemplate::New(isolate, listen)->GetFunction(context).ToLocalChecked();
    exports->Set(context, v8::String::NewFromUtf8(isolate, "tcpListen").ToLocalChecked(), func).ToChecked();
  }

  static void listen(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.Length() == 2);
    
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Function> callback = args[0].As<v8::Function>();
    int port = args[1]->IntegerValue(context).ToChecked();

    TcpServer *server = new TcpServer;
    server->callback.Reset(isolate, callback);
    server->port = port;

    lx_listener_t *l = lx_listen(IO::get()->ctx, port, TcpServer::handle_accept);
    l->data = server;
  }

  static void handle_accept(lx_connection_t *conn) {
    TcpServer *server = static_cast<TcpServer*>(conn->listener->data);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Function> callback = server->callback.Get(isolate);

    if (callback.IsEmpty()) {
      lx_close(conn);
      return;
    }

    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, TcpStream::streamConstructor);
    v8::Local<v8::Object> tcpStreamWrap = cons->NewInstance(context).ToLocalChecked();
    TcpStream *stream = static_cast<TcpStream*>(tcpStreamWrap->GetAlignedPointerFromInternalField(0));
    stream->conn = conn;
    lx_timer_start(&stream->timeout, TcpStream::handle_timeout, TcpServer::kDefaultTimeout);
    conn->data = (void*)stream;
    conn->onclose = TcpStream::handle_close;
    conn->ondata = TcpStream::handle_data;

    v8::Local<v8::Value> args[1] = {tcpStreamWrap};
    callback->Call(context, v8::Undefined(isolate), 1, args).ToLocalChecked();
  }
};
}
