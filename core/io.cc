#pragma once
#include <pandio.h>

namespace runtime {

/* IO singleton - manages actual event loop */
class IO {
  static IO *instance_;
public:
  pd_io_t *ctx;

  IO(pd_io_t *ctx): ctx(ctx) {}
  ~IO() {
    delete ctx;
  }

  static void create() {
    pd_io_t *ctx = new pd_io_t;
    pd_io_init(ctx);
    IO:instance_ = new IO(ctx);
  }

  static IO *get() {
    return IO::instance_;
  }

  inline void run() {
    pd_io_run(this->ctx);
  }
};

IO *IO::instance_ = nullptr;

}
