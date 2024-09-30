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
#include "internal.h"
#include <unistd.h>

#ifdef __linux__
#include <sys/eventfd.h>
#endif

void pd__notifier_io(pd_event_t *event, unsigned events) {
    pd_notifier_t *notifier = event->data;
#ifdef __linux__
    pd_fd_t fd = notifier->fd;
#else
    pd_fd_t fd = notifier->fd[0];
#endif


    if ((events & PD_POLLIN) && (notifier->handler)) {
        char buf[512];
        ssize_t status;

        do {
            status = read(fd, buf, 512);
        } while (status == -1 && errno == EINTR);

        if (status >= 0) {
            notifier->handler(notifier);
        }
    }
}


void pd_notifier_init(pd_io_t *ctx, pd_notifier_t *notifier) {
#ifdef __linux__
    notifier->fd = eventfd(0, 0);
    pd__set_nonblocking(notifier->fd);
#else
    pipe(notifier->fd);
    pd__set_nonblocking(notifier->fd[0]);
    pd__set_nonblocking(notifier->fd[1]);
#endif
    notifier->ctx = ctx;
    notifier->handler = NULL;
    pd__event_init(&notifier->event);
    notifier->event.data = notifier;
    notifier->event.handler = pd__notifier_io;
    notifier->event.flags |= PD_POLLIN;

#ifdef __linux__
    pd__event_add(ctx, &notifier->event, notifier->fd);
#else
    pd__event_add(ctx, &notifier->event, notifier->fd[0]);
#endif
}


void pd_notifier_send(pd_notifier_t *notifier) {
    pd_fd_t fd;

#ifdef __linux__
    fd = notifier->fd;
#else
    fd = notifier->fd[1];
#endif

    int64_t u = 1;
    ssize_t ret;

    do {
        ret = write(fd, &u, sizeof(int64_t));
    } while (ret == -1 && errno == EINTR);
}
