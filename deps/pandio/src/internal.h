/* Common internal functions / helpers */
#pragma once

#include "pandio/core.h"
#include "pandio/tcp.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr) - offsetof(type, member)))

void pd__tcp_pending_close(pd_io_t *);

void pd__event_init(pd_event_t *);

#ifdef _WIN32
int pd__set_nonblocking(pd_socket_t);
#else

int pd__set_nonblocking(pd_fd_t);

int pd__event_set(pd_io_t *, pd_event_t *, pd_fd_t);

int pd__event_add(pd_io_t *, pd_event_t *, pd_fd_t);

int pd__event_del(pd_io_t *, pd_fd_t);

#endif
