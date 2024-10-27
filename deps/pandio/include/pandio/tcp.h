/* Copyright (c) Michał Dziuba
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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

#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#include <ws2def.h>
#include <mswsock.h>
#include <ws2tcpip.h>

typedef SOCKET pd_socket_t;

#else

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
typedef int pd_socket_t;

#endif

#include "core.h"
#include "queue.h"

enum pd_tcp_status {
    PD_TCP_NONE,
    PD_TCP_ACTIVE,
    PD_TCP_SHUTDOWN,
    PD_TCP_CLOSED
};

enum pd_stream_flags {
    PD_WRITING = 1 << 0,
    PD_READING = 1 << 1,
    PD_PENDING_READ = 1 << 2
};

// WSABUF compatible structure
struct pd_buf_s {
    size_t len;
    char *buf;
};

typedef struct pd_buf_s pd_buf_t;

/* struct that represents TCP connection and stream */
struct pd_tcp_s {
    pd_io_t *ctx;
    pd_socket_t fd;
#ifdef _WIN32
    WSABUF read_buf;
    pd_event_t revent;
#else
    struct queue writes;
    pd_event_t event;
#endif
    void* (*allocator)(struct pd_tcp_s *, size_t size);
    void (*on_data)(struct pd_tcp_s *, char *buf, size_t size);
    void (*on_close)(struct pd_tcp_s *);
    void (*on_connect)(struct pd_tcp_s *, int);
    enum pd_tcp_status status;
    unsigned flags;
    void *data; // pointer for user's data
    size_t writes_size; // amount of pending writes
    struct queue_node close_qnode;
};

typedef struct pd_tcp_s pd_tcp_t;

static void *pd_default_allocator(pd_tcp_t *stream, size_t size) {
    return malloc(size);
}

struct pd_tcp_server_s;
typedef void (*pd_on_connection_cb)(struct pd_tcp_server_s*, pd_socket_t, int);

/* struct that represents TCP server/listener */
struct pd_tcp_server_s {
    pd_io_t *ctx;
    pd_socket_t fd;
    pd_on_connection_cb on_connection;
    enum pd_tcp_status status;
    void *data; // pointer for user's data
#ifdef _WIN32
    LPFN_ACCEPTEX acceptex;
    struct queue accept_ops;
#else
    pd_event_t event;
#endif
};

typedef struct pd_tcp_server_s pd_tcp_server_t;

struct pd_write_s;

typedef void (*pd_write_cb)(struct pd_write_s*, int status);

struct pd_write_s {
#ifdef _WIN32
    WSABUF data;
    pd_event_t event;
    pd_tcp_t *handle;
#else
    pd_buf_t data;
    struct queue_node qnode;
    size_t written;
#endif
    void *udata;
    pd_write_cb cb;
};

typedef struct pd_write_s pd_write_t;

int pd_tcp_listen(pd_tcp_server_t*, int, pd_on_connection_cb);

void pd_tcp_server_init(pd_io_t*, pd_tcp_server_t*);

int pd_tcp_server_close(pd_tcp_server_t*);

void pd_tcp_init(pd_io_t*, pd_tcp_t*);

/* Creates pd_tcp_t from OS socket handle */
void pd_tcp_accept(pd_tcp_t*, pd_socket_t);

void pd_write_init(pd_write_t*, char*, size_t, pd_write_cb);

void pd_tcp_write(pd_tcp_t*, pd_write_t*);

/* Forcefully closes connection - useful for timeouts/error disconnects. */
void pd_tcp_close(pd_tcp_t*);

void pd_tcp_pause(pd_tcp_t*);

void pd_tcp_resume(pd_tcp_t*);

/* Graceful connection shutdown. Ensures that incoming data from the peer will be read. */
void pd_tcp_shutdown(pd_tcp_t*);

int pd_tcp_connect(pd_tcp_t*, const char*, int, void (*on_connect)(pd_tcp_t*, int));

/* Enable/disable keep-alive functionality and set initial delay */
int pd_tcp_keepalive(pd_tcp_t*, int, int);

/* Enable/disable Nagle's algorithm */
int pd_tcp_nodelay(pd_tcp_t*, int);
