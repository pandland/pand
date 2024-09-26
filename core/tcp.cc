#pragma once

#include <cstdio>
#include <v8.h>
#include <pandio.h>
#include <assert.h>

#include <string.h>
#include "io.cc"
#include "v8_utils.cc"

namespace runtime {

class TcpStream {
public:
  pd_tcp_t conn;
  pd_timer_t timeout;
  bool paused = false;
  v8::Persistent<v8::Object> obj;

  TcpStream(v8::Isolate *isolate, v8::Local<v8::Object> obj) {
    this->obj.Reset(isolate, obj);
    pd_timer_init(IO::get()->ctx, &this->timeout);
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

    v8::Local<v8::FunctionTemplate> setTimeoutTemplate = v8::FunctionTemplate::New(isolate, TcpStream::set_timeout);
    t->PrototypeTemplate()->Set(isolate, "setTimeout", setTimeoutTemplate);

    v8::Local<v8::FunctionTemplate> pauseTemplate = v8::FunctionTemplate::New(isolate, TcpStream::pause);
    t->PrototypeTemplate()->Set(isolate, "pause", pauseTemplate);

    v8::Local<v8::FunctionTemplate> resumeTemplate = v8::FunctionTemplate::New(isolate, TcpStream::resume);
    t->PrototypeTemplate()->Set(isolate, "resume", resumeTemplate);
    
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

    pd_write_t *write_op = new pd_write_t;
    pd_write_init(write_op, buf, str.length(), TcpStream::handle_write);
    pd_tcp_write(&stream->conn, write_op);
  }

  static void close(const v8::FunctionCallbackInfo<v8::Value> &args) {
    assert(args.Length() == 0);
    TcpStream *stream = static_cast<TcpStream*>(args.This()->GetAlignedPointerFromInternalField(0));

    pd_tcp_close(&stream->conn);
  }

  static void set_timeout(const v8::FunctionCallbackInfo<v8::Value> &args) {
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);

    if (args.Length() != 1 && !args[0]->IsNumber()) {
      isolate->ThrowException(v8::Exception::Error(v8_str(isolate, "Expected timeout as number")));
      return;
    }

    int64_t timeout = args[0].As<v8::Number>()->Value();
    TcpStream *stream = static_cast<TcpStream*>(args.This()->GetAlignedPointerFromInternalField(0));

    if (timeout == 0) {
      pd_timer_stop(&stream->timeout);
      return;
    }

    pd_timer_stop(&stream->timeout);
    pd_timer_start(&stream->timeout,  TcpStream::handle_timeout, timeout);
  }

  static void pause(const v8::FunctionCallbackInfo<v8::Value> &args) {
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);

    TcpStream *stream = static_cast<TcpStream*>(args.This()->GetAlignedPointerFromInternalField(0));
    if (!stream->paused) {
      pd_tcp_pause(&stream->conn);
      stream->paused = true;
    }
  }

  static void resume(const v8::FunctionCallbackInfo<v8::Value> &args) {
    v8::Isolate *isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);

    TcpStream *stream = static_cast<TcpStream*>(args.This()->GetAlignedPointerFromInternalField(0));
    if (stream->paused) {
      pd_tcp_resume(&stream->conn);
      stream->paused = false;
    }
  }

  /* handlers for event loop */
  static void handle_write(pd_write_t *write_op, int status) {
    free((void*)write_op->data.buf);
    delete write_op;
  }

  static void handle_data(pd_tcp_t *conn, char *buf, size_t size) {
    printf("Status: %ld\n", size); fflush(stdout);
    TcpStream *stream = static_cast<TcpStream*>(conn->data);
    
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Function> callback =
      stream->obj.Get(isolate)->Get(context, v8_symbol(isolate, "onread")).ToLocalChecked().As<v8::Function>();
    if (callback.IsEmpty()) {
      return;
    }

    v8::Local<v8::Value> args[1] = {v8::String::NewFromUtf8(isolate, buf, v8::NewStringType::kNormal, size).ToLocalChecked()};
    callback->Call(context, v8::Undefined(isolate), 1, args).ToLocalChecked();
  }

  static void handle_close(pd_tcp_t *conn) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    TcpStream *stream = static_cast<TcpStream*>(conn->data);
    pd_timer_stop(&stream->timeout);
    v8::Local<v8::Function> callback =
      stream->obj.Get(isolate)->Get(context, v8_symbol(isolate, "onclose")).ToLocalChecked().As<v8::Function>();
    callback->Call(context, v8::Undefined(isolate), 0, {}).ToLocalChecked();
    //delete stream;
  }

  static void handle_timeout(pd_timer_t *timer) {
    TcpStream *stream = static_cast<TcpStream*>(timer->data);
    pd_tcp_close(&stream->conn);
  }
};

v8::Persistent<v8::Function> TcpStream::streamConstructor;

class TcpServer {
  v8::Global<v8::Function> callback;
  pd_tcp_server_t handle;
  int port;

  static const int64_t kDefaultTimeout = 120 * 1000;
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

    pd_tcp_server_init(IO::get()->ctx, &server->handle);
    server->handle.data = server;
    int status = pd_tcp_listen(&server->handle, port, TcpServer::handle_accept);
    if (status < 0) {
      printf("Something went wrong...");
    }
  }

  static void handle_accept(pd_tcp_server_t *handle, pd_socket_t socket, int status) {
    TcpServer *server = static_cast<TcpServer*>(handle->data); 
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Function> callback = server->callback.Get(isolate);

    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(isolate, TcpStream::streamConstructor);
    v8::Local<v8::Object> tcpStreamWrap = cons->NewInstance(context).ToLocalChecked();
    TcpStream *stream = static_cast<TcpStream*>(tcpStreamWrap->GetAlignedPointerFromInternalField(0));
    pd_tcp_init(handle->ctx, &stream->conn);
    pd_tcp_accept(&stream->conn, socket);
    pd_timer_start(&stream->timeout, TcpStream::handle_timeout, TcpServer::kDefaultTimeout);
    stream->conn.data = (void*)stream;
    stream->conn.on_close = TcpStream::handle_close;
    stream->conn.on_data = TcpStream::handle_data;

    if (callback.IsEmpty()) {
      pd_tcp_close(&stream->conn);
      return;
    }

    v8::Local<v8::Value> args[1] = {tcpStreamWrap};
    callback->Call(context, v8::Undefined(isolate), 1, args).ToLocalChecked();
  }
};
}
