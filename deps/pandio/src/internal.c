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

#include "pandio/core.h"
#include "pandio/tcp.h"
#include "pandio/err.h"
#include "internal.h"
#include <fcntl.h>

#ifdef _WIN32
#include <processthreadsapi.h>
#endif

void pd__tcp_pending_close(pd_io_t *ctx) {
    while (!queue_empty(&ctx->pending_closes)) {
        struct queue_node *next = queue_pop(&ctx->pending_closes);
        pd_tcp_t *stream = container_of(next, pd_tcp_t, close_qnode);

        if (stream->on_close)
            stream->on_close(stream);

        ctx->refs--;
    }
}


const char* pd_get_platform() {
#if defined(_WIN32)
    return "win32";
#elif defined(__APPLE__) || defined(__MACH__)
    return "darwin";
#elif defined(__linux__)
    return "linux";
#elif defined(__FreeBSD__)
    return "freebsd";
#elif defined(__OpenBSD__)
    return "openbsd";
#elif defined(__unix__)
    return "unix";
#else
    return "unknown";
#endif
}


pd_pid_t pd_getpid() {
#if defined(_WIN32)
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}


const char* pd_errstr(int errcode) {
    switch (errcode) {
#define X(pd_err, msg) \
        case(pd_err):   \
            return msg;
            PD_ERR_STR_MAPPING(X)
#undef X
        default:
            return "Unknown error";
    }
}


const char* pd_errname(int errcode) {
    switch (errcode) {
#define X(pd_err, msg) \
        case(pd_err):   \
            return msg;
        PD_ERR_NAME_MAPPING(X)
#undef X
        default:
            return "UNKNOWN";
    }
}


#ifdef _WIN32

#define PD__ERRNO_MAPPING(X)               \
  X(ERROR_ACCESS_DENIED, PD_EACCES)             \
  X(ERROR_ALREADY_EXISTS, PD_EEXIST)            \
  X(ERROR_BAD_COMMAND, PD_EACCES)               \
  X(ERROR_BAD_EXE_FORMAT, PD_ENOEXEC)           \
  X(ERROR_BAD_LENGTH, PD_EACCES)                \
  X(ERROR_BAD_NETPATH, PD_ENOENT)               \
  X(ERROR_BAD_NET_NAME, PD_ENOENT)              \
  X(ERROR_BAD_NET_RESP, PD_ENETDOWN)            \
  X(ERROR_BAD_PATHNAME, PD_ENOENT)              \
  X(ERROR_BROKEN_PIPE, PD_EPIPE)                \
  X(ERROR_CANNOT_MAKE, PD_EACCES)               \
  X(ERROR_COMMITMENT_LIMIT, PD_ENOMEM)          \
  X(ERROR_CONNECTION_ABORTED, PD_ECONNABORTED)  \
  X(ERROR_CONNECTION_ACTIVE, PD_EISCONN)        \
  X(ERROR_CONNECTION_REFUSED, PD_ECONNREFUSED)  \
  X(ERROR_CRC, PD_EACCES)                       \
  X(ERROR_DIR_NOT_EMPTY, PD_ENOTEMPTY)          \
  X(ERROR_DISK_FULL, PD_ENOSPC)                 \
  X(ERROR_DUP_NAME, PD_EADDRINUSE)              \
  X(ERROR_FILENAME_EXCED_RANGE, PD_ENOENT)      \
  X(ERROR_FILE_NOT_FOUND, PD_ENOENT)            \
  X(ERROR_GEN_FAILURE, PD_EACCES)               \
  X(ERROR_GRACEFUL_DISCONNECT, PD_EPIPE)        \
  X(ERROR_HOST_DOWN, PD_EHOSTUNREACH)           \
  X(ERROR_HOST_UNREACHABLE, PD_EHOSTUNREACH)    \
  X(ERROR_INSUFFICIENT_BUFFER, PD_EFAULT)       \
  X(ERROR_INVALID_ADDRESS, PD_EADDRNOTAVAIL)    \
  X(ERROR_INVALID_FUNCTION, PD_EINVAL)          \
  X(ERROR_INVALID_HANDLE, PD_EBADF)             \
  X(ERROR_INVALID_NETNAME, PD_EADDRNOTAVAIL)    \
  X(ERROR_INVALID_PARAMETER, PD_EINVAL)         \
  X(ERROR_INVALID_USER_BUFFER, PD_EMSGSIZE)     \
  X(ERROR_IO_PENDING, PD_EINPROGRESS)           \
  X(ERROR_LOCK_VIOLATION, PD_EACCES)            \
  X(ERROR_MORE_DATA, PD_EMSGSIZE)               \
  X(ERROR_NETNAME_DELETED, PD_ECONNABORTED)     \
  X(ERROR_NETWORK_ACCESS_DENIED, PD_EACCES)     \
  X(ERROR_NETWORK_BUSY, PD_ENETDOWN)            \
  X(ERROR_NETWORK_UNREACHABLE, PD_ENETUNREACH)  \
  X(ERROR_NOACCESS, PD_EFAULT)                  \
  X(ERROR_NONPAGED_SYSTEM_RESOURCES, PD_ENOMEM) \
  X(ERROR_NOT_ENOUGH_MEMORY, PD_ENOMEM)         \
  X(ERROR_NOT_ENOUGH_QUOTA, PD_ENOMEM)          \
  X(ERROR_NOT_FOUND, PD_ENOENT)                 \
  X(ERROR_NOT_LOCKED, PD_EACCES)                \
  X(ERROR_NOT_READY, PD_EACCES)                 \
  X(ERROR_NOT_SAME_DEVICE, PD_EXDEV)            \
  X(ERROR_NOT_SUPPORTED, PD_ENOTSUP)            \
  X(ERROR_NO_MORE_FILES, PD_ENOENT)             \
  X(ERROR_NO_SYSTEM_RESOURCES, PD_ENOMEM)       \
  X(ERROR_OPERATION_ABORTED, PD_EINTR)          \
  X(ERROR_OUT_OF_PAPER, PD_EACCES)              \
  X(ERROR_PAGED_SYSTEM_RESOURCES, PD_ENOMEM)    \
  X(ERROR_PAGEFILE_QUOTA, PD_ENOMEM)            \
  X(ERROR_PATH_NOT_FOUND, PD_ENOENT)            \
  X(ERROR_PIPE_NOT_CONNECTED, PD_EPIPE)         \
  X(ERROR_PORT_UNREACHABLE, PD_ECONNRESET)      \
  X(ERROR_PROTOCOL_UNREACHABLE, PD_ENETUNREACH) \
  X(ERROR_REM_NOT_LIST, PD_ECONNREFUSED)        \
  X(ERROR_REQUEST_ABORTED, PD_EINTR)            \
  X(ERROR_REQ_NOT_ACCEP, PD_EWOULDBLOCK)        \
  X(ERROR_SECTOR_NOT_FOUND, PD_EACCES)          \
  X(ERROR_SEM_TIMEOUT, PD_ETIMEDOUT)            \
  X(ERROR_SHARING_VIOLATION, PD_EACCES)         \
  X(ERROR_TOO_MANY_NAMES, PD_ENOMEM)            \
  X(ERROR_TOO_MANY_OPEN_FILES, PD_EMFILE)       \
  X(ERROR_UNEXP_NET_ERR, PD_ECONNABORTED)       \
  X(ERROR_WAIT_NO_CHILDREN, PD_ECHILD)          \
  X(ERROR_WORKING_SET_QUOTA, PD_ENOMEM)         \
  X(ERROR_WRITE_PROTECT, PD_EACCES)             \
  X(ERROR_WRONG_DISK, PD_EACCES)                \
  X(WSAEACCES, PD_EACCES)                       \
  X(WSAEADDRINUSE, PD_EADDRINUSE)               \
  X(WSAEADDRNOTAVAIL, PD_EADDRNOTAVAIL)         \
  X(WSAEAFNOSUPPORT, PD_EAFNOSUPPORT)           \
  X(WSAECONNABORTED, PD_ECONNABORTED)           \
  X(WSAECONNREFUSED, PD_ECONNREFUSED)           \
  X(WSAECONNRESET, PD_ECONNRESET)               \
  X(WSAEDISCON, PD_EPIPE)                       \
  X(WSAEFAULT, PD_EFAULT)                       \
  X(WSAEHOSTDOWN, PD_EHOSTUNREACH)              \
  X(WSAEHOSTUNREACH, PD_EHOSTUNREACH)           \
  X(WSAEINPROGRESS, PD_EBUSY)                   \
  X(WSAEINTR, PD_EINTR)                         \
  X(WSAEINVAL, PD_EINVAL)                       \
  X(WSAEISCONN, PD_EISCONN)                     \
  X(WSAEMSGSIZE, PD_EMSGSIZE)                   \
  X(WSAENETDOWN, PD_ENETDOWN)                   \
  X(WSAENETRESET, PD_EHOSTUNREACH)              \
  X(WSAENETUNREACH, PD_ENETUNREACH)             \
  X(WSAENOBUFS, PD_ENOMEM)                      \
  X(WSAENOTCONN, PD_ENOTCONN)                   \
  X(WSAENOTSOCK, PD_ENOTSOCK)                   \
  X(WSAEOPNOTSUPP, PD_EOPNOTSUPP)               \
  X(WSAEPROCLIM, PD_ENOMEM)                     \
  X(WSAESHUTDOWN, PD_EPIPE)                     \
  X(WSAETIMEDOUT, PD_ETIMEDOUT)                 \
  X(WSAEWOULDBLOCK, PD_EWOULDBLOCK)             \
  X(WSANOTINITIALISED, PD_ENETDOWN)             \
  X(WSASYSNOTREADY, PD_ENETDOWN)                \
  X(WSAVERNOTSUPPORTED, PD_ENOSYS)

int pd_errmap(pd_errno_t err) {
    if (err <= 0)
        return err;

    switch (err) {
#define X(sys_err, pd_err) \
        case sys_err:               \
            return pd_err;
        PD__ERRNO_MAPPING(X)
#undef X
        default:
            return PD_UNKNOWN;
    }
}

int pd_errno() {
    pd_errno_t sys_err = GetLastError();
    return pd_errmap(sys_err);
}


int pd__set_nonblocking(pd_socket_t fd) {
    DWORD enable = 1;
    return ioctlsocket(fd, FIONBIO, &enable);
}

#else

int pd_errmap(pd_errno_t err) {
    if (err <= 0)
        return err;

    return -err;
}

int pd_errno() {
    return pd_errmap(errno);
}

int pd__set_nonblocking(pd_fd_t fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1)
        return -1;

    return 0;
}

#endif
