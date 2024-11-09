/* Copyright (c) Michał Dziuba
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include "pandio/core.h"
#include "pandio/threadpool.h"
#include <fcntl.h>

#ifdef _WIN32

#define PD_FS_O_RDONLY _O_RDONLY
#define PD_FS_O_WRONLY _O_WRONLY
#define PD_FS_O_RDWR _O_RDWR
#define PD_FS_O_CREAT _O_CREAT
#define PD_FS_O_EXCL _O_EXCL
#define PD_FS_O_TRUNC _O_TRUNC
#define PD_FS_O_APPEND _O_APPEND

#else

#define PD_FS_O_RDONLY O_RDONLY
#define PD_FS_O_WRONLY O_WRONLY
#define PD_FS_O_RDWR O_RDWR
#define PD_FS_O_CREAT O_CREAT
#define PD_FS_O_EXCL O_EXCL
#define PD_FS_O_TRUNC O_TRUNC
#define PD_FS_O_APPEND O_APPEND

#endif

#define FS_OPS(X)                                                              \
  X(open, {                                                                    \
    char *path;                                                                \
    int oflag;                                                                 \
  })                                                                           \
  X(read, {                                                                    \
    pd_fd_t fd;                                                                \
    char *buf;                                                                 \
    size_t size;                                                               \
  })                                                                           \
  X(write, {                                                                   \
    pd_fd_t fd;                                                                \
    const char *buf;                                                           \
    size_t size;                                                               \
  })                                                                           \
  X(close, { pd_fd_t fd; })

#define FS_TYPES(type, params) pd_##type##_op,

enum pd_fs_type { FS_OPS(FS_TYPES) pd_unknown_op = -1 };

typedef enum pd_fs_type pd_fs_type_t;

#define FS_PARAMS(type, params) struct params type;

union pd_fs_params_u {
  FS_OPS(FS_PARAMS)
};

typedef union pd_fs_params_u pd_fs_params_t;

struct pd_fs_s {
  pd_task_t task; // must be first, because later we cast from it
  pd_io_t *ctx;
  pd_fs_type_t type;
  void *udata; // pointer to some user's struct, (don't use it)
  void (*cb)(struct pd_fs_s *);
  int status;
  pd_fs_params_t params;
  union {
    pd_fd_t fd;
    ssize_t size;
  } result;
};

typedef struct pd_fs_s pd_fs_t;

typedef void (*pd_fs_cb_t)(pd_fs_t *);

void pd_fs_init(pd_io_t *, pd_fs_t *);

void pd_fs_open(pd_fs_t *, const char *, int, pd_fs_cb_t);

void pd_fs_read(pd_fs_t *, pd_fd_t, char *, size_t, pd_fs_cb_t);

void pd_fs_write(pd_fs_t *, pd_fd_t, const char *, size_t, pd_fs_cb_t);

void pd_fs_close(pd_fs_t *, pd_fd_t, pd_fs_cb_t);
