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

#include "pandio/err.h"
#include "pandio/fs.h"
#include "string.h"
#include <unistd.h>

void pd__fs_work_done(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  op->cb(op);
}

void pd_fs_init(pd_io_t *ctx, pd_fs_t *op) {
  memset(op, 0, sizeof(pd_fs_t));
  op->ctx = ctx;
  op->status = PD_UNKNOWN;
  op->type = pd_unknown_op;
  op->task.done = pd__fs_work_done;
}


void pd__fs_open_work(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  char *path = op->params.open.path;
  int oflag = op->params.open.oflag;
  int fd;

  do {
    fd = open(path, oflag);
  } while (fd < 0 && errno == EINTR);

  if (fd < 0) {
    op->status = pd_errno();
  } else {
    op->status = 0;
  }

  op->result.fd = fd;
  free(path);
  op->params.open.path = NULL;
}

void pd_fs_open(pd_fs_t *op, const char *path, int oflag,
                void (*cb)(pd_fs_t *))  {
  op->params.open.path = strdup(path);
  op->params.open.oflag = oflag;
  op->type = pd_open_op;
  op->cb = cb;
  op->task.work = pd__fs_open_work;

  pd_task_submit(op->ctx, &op->task);
}


void pd__fs_read_work(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  ssize_t nread;
  int fd = op->params.read.fd;
  size_t n = op->params.read.size;

  do {
    nread = read(fd, op->params.read.buf, n);
  } while (nread < 0 && errno == EINTR);

  if (nread < 0) {
    op->status = pd_errno();
    return;
  } else {
    op->status = 0;
  }

  op->result.size = nread;
}

void pd_fs_read(pd_fs_t *op, pd_fd_t fd, char *buf, size_t size,
                void (*cb)(pd_fs_t *)) {
  op->type = pd_read_op;
  op->cb = cb;
  op->params.read.buf = buf;
  op->params.read.size = size;
  op->params.read.fd = fd;

  op->task.work = pd__fs_read_work;
  pd_task_submit(op->ctx, &op->task);
}


void pd__fs_write_work(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  ssize_t nwritten;
  int fd = op->params.write.fd;
  size_t n = op->params.write.size;

  do {
    nwritten = write(fd, op->params.write.buf, n);
  } while (nwritten < 0 && errno == EINTR);

  if (nwritten < 0) {
    op->status = pd_errno();
    return;
  } else {
    op->status = 0;
  }

  op->result.size = nwritten;
}

void pd_fs_write(pd_fs_t *op, pd_fd_t fd, const char *buf, size_t size,
                 void (*cb)(pd_fs_t *)) {
  op->type = pd_write_op;
  op->cb = cb;
  op->params.write.buf = buf;
  op->params.write.size = size;
  op->params.write.fd = fd;

  op->task.work = pd__fs_write_work;
  pd_task_submit(op->ctx, &op->task);
}


void pd__fs_close_work(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  int status;
  int fd = op->params.close.fd;

  do {
    status = close(fd);
  } while (status < 0 && errno == EINTR);

  if (status < 0) {
    op->status = pd_errno();
    return;
  } else {
    op->status = 0;
  }
}

void pd_fs_close(pd_fs_t *op, pd_fd_t fd, void (*cb)(pd_fs_t *)) {
  op->type = pd_close_op;
  op->cb = cb;
  op->params.close.fd = fd;

  op->task.work = pd__fs_close_work;
  pd_task_submit(op->ctx, &op->task);
}
