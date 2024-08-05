#pragma once
#include <luxio.h>

namespace runtime {

/* IO singleton - manages actuall event loop */
class IO {
  static IO *instance_;
public:
  lx_io_t *ctx;

  IO(lx_io_t *ctx): ctx(ctx) {}
  ~IO() {
    delete ctx;
  }

  static void create() {
    lx_io_t *ctx = new lx_io_t;
    lx_init(ctx);
    IO:instance_ = new IO(ctx);
  }

  static IO *get() {
    return IO::instance_;
  }

  inline void run() {
    lx_run(this->ctx);
  }
};

IO *IO::instance_ = nullptr;

}
