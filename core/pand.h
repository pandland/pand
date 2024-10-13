#pragma once

#include <libplatform/libplatform.h>
#include <pandio.h>
#include <string>
#include <v8.h>

namespace pand::core {

class Pand {
  std::unique_ptr<v8::Platform> platform;
  v8::Isolate::CreateParams create_params;
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
};

} // namespace pand::core
