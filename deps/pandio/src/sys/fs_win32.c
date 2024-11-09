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

#include "pandio/core.h"
#include "pandio/err.h"
#include "pandio/fs.h"
#include "pandio/threadpool.h"
#include <fileapi.h>


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

  DWORD access = 0;
  DWORD creation = OPEN_EXISTING;
  DWORD attributes = FILE_ATTRIBUTE_NORMAL;

  if (oflag & PD_FS_O_RDONLY)
    access |= GENERIC_READ;
  if (oflag & PD_FS_O_WRONLY)
    access |= GENERIC_WRITE;
  if (oflag & PD_FS_O_RDWR)
    access |= (GENERIC_READ | GENERIC_WRITE);

  if (oflag & PD_FS_O_CREAT) {
    creation = (oflag & PD_FS_O_EXCL) ? CREATE_NEW : OPEN_ALWAYS;
  }
  if (oflag & PD_FS_O_TRUNC)
    creation = CREATE_ALWAYS;

  if (oflag & PD_FS_O_APPEND)
    attributes |= FILE_APPEND_DATA;

  pd_fd_t handle =
      CreateFile(path, access, 0, NULL, creation, attributes, NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    op->status = pd_errno();
  } else {
    op->status = 0;
  }

  op->result.fd = handle;
  free(path);
  op->params.open.path = NULL;
}

void pd_fs_open(pd_fs_t *op, const char *path, int oflag, int mode, pd_fs_cb_t cb) {
  op->params.open.path = _strdup(path);
  op->params.open.oflag = oflag;
  op->params.open.mode = mode;
  op->type = pd_open_op;
  op->cb = cb;
  op->task.work = pd__fs_open_work;

  pd_task_submit(op->ctx, &op->task);
}

void pd__fs_close_work(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  pd_fd_t handle = op->params.close.fd;

  if (!CloseHandle(handle)) {
    op->status = pd_errno();
  } else {
    op->status = 0;
  }
}

void pd_fs_close(pd_fs_t *op, pd_fd_t handle, pd_fs_cb_t cb) {
  op->params.close.fd = handle;
  op->type = pd_close_op;
  op->cb = cb;
  op->task.work = pd__fs_close_work;

  pd_task_submit(op->ctx, &op->task);
}

void pd__fs_read_work(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  pd_fd_t handle = op->params.read.fd;
  char *buf = op->params.read.buf;
  size_t size = op->params.read.size;

  DWORD nread;
  if (ReadFile(handle, buf, size, &nread, NULL)) {
    op->status = 0;
    op->result.size = nread;
  } else {
    op->status = pd_errno();
    op->result.size = -1;
  }
}

void pd_fs_read(pd_fs_t *op, pd_fd_t fd, char *buf, size_t size,
                pd_fs_cb_t cb) {
  op->type = pd_read_op;
  op->params.read.fd = fd;
  op->params.read.buf = buf;
  op->params.read.size = size;
  op->cb = cb;
  op->task.work = pd__fs_read_work;

  pd_task_submit(op->ctx, &op->task);
}

void pd__fs_write_work(pd_task_t *task) {
  pd_fs_t *op = (pd_fs_t *)(task);
  pd_fd_t handle = op->params.write.fd;
  const char *buf = op->params.write.buf;
  size_t size = op->params.write.size;
  
  DWORD nwritten;
  if(WriteFile(handle, buf, size, &nwritten, NULL)) {
    op->status = 0;
    op->result.size = nwritten;
  } else {
    op->status = pd_errno();
    op->result.size = -1;
  }
}

void pd_fs_write(pd_fs_t *op, pd_fd_t fd, const char *buf, size_t size,
                 pd_fs_cb_t cb) {
  op->type = pd_write_op;
  op->params.write.fd = fd;
  op->params.write.buf = buf;
  op->params.write.size = size;
  op->cb = cb;
  op->task.work = pd__fs_write_work;

  pd_task_submit(op->ctx, &op->task);
}
