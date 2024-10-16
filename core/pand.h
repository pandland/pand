#pragma once

#include <libplatform/libplatform.h>
#include <pandio.h>
#include <string>
#include <v8-local-handle.h>
#include <v8-persistent-handle.h>
#include <v8.h>

namespace pand::core {

class Pand {
  std::unique_ptr<v8::Platform> platform;
  v8::Isolate::CreateParams create_params;
  v8::Persistent<v8::Function> tcpStreamConstructor;

  Pand();
  Pand(const Pand &) = delete;
  ~Pand();

public:
  pd_io_t *ctx;
  v8::Isolate *isolate;
  static Pand *get();
  void exit(int status);
  void run(const std::string &);
  void run(const std::string &, int argc, char *argv);

  static void makeCallback(v8::Local<v8::Object> &, v8::Isolate *, const char *,
                           v8::Local<v8::Value> *, size_t);
  void setTcpStreamConstructor(v8::Local<v8::Function>);
  v8::Local<v8::Function> getTcpStreamConstructor();
};

} // namespace pand::core
