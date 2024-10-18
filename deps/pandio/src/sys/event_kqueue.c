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
#include "pandio/timers.h"
#include "pandio/threadpool.h"
#include "internal.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


uint64_t pd_now() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}


void pd_io_init(pd_io_t *ctx) {
    ctx->refs = 0;
    ctx->after_tick = NULL;
    ctx->poll_fd = kqueue();
    if (ctx->poll_fd < 0)
        abort();

    ctx->now = pd_now();
    queue_init(&ctx->pending_closes);
    pd_timers_heap_init(ctx);

    ctx->task_signal = malloc(sizeof(pd_notifier_t));
    pd_notifier_init(ctx, ctx->task_signal);
    ctx->task_signal->handler = pd__task_done;
    ctx->task_signaled = false;

    queue_init(&ctx->finished_tasks);
}


void pd__event_init(pd_event_t *event) {
    event->handler = NULL;
    event->flags = 0;
    event->data = NULL;
}


int pd__event_set(pd_io_t *ctx, pd_event_t *event, pd_fd_t fd) {
    struct kevent ev[2];
    int n = 0;

    if (event->flags & PD_POLLIN) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, event);
    } else {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, event);
    }

    if (event->flags & PD_POLLOUT) {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, event);
    } else {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, event);
    }

    return kevent(ctx->poll_fd, ev, n, NULL, 0, NULL);
}


int pd__event_add(pd_io_t *ctx, pd_event_t *event, pd_fd_t fd) {
    return pd__event_set(ctx, event, fd);
}


int pd__event_del(pd_io_t *ctx, pd_fd_t fd) {
    // close() should remove event from kqueue (unlike in epoll).
    return 0;
}


#define MAX_EVENTS 1024

void pd_io_run(pd_io_t *ctx) {
    struct kevent events[MAX_EVENTS];
    struct kevent ev;
    int timeout = pd_timers_next(ctx);
    pd_event_t *pev;

    while (ctx->refs > 0) {
        struct timespec ts;
        if (timeout >= 0) {
            ts.tv_sec = timeout / 1000;
            ts.tv_nsec = (timeout % 1000) * 1000000;
        }

        int nevents = kevent(ctx->poll_fd, NULL, 0, events, MAX_EVENTS, timeout >= 0 ? &ts : NULL);
        if (nevents == -1 && errno == EINTR) {
            // interrupted by signal
            continue;
        }

        if (nevents == -1) {
            exit(EXIT_FAILURE);
        }

        ctx->now = pd_now();

        for (int i = 0; i < nevents; ++i) {
            ev = events[i];
            unsigned pevents = 0;
            pev = (pd_event_t *)ev.udata;

            if (ev.filter == EVFILT_READ)
                pevents |= PD_POLLIN;

            if (ev.filter == EVFILT_WRITE)
                pevents |= PD_POLLOUT;

            if ((ev.flags & EV_ERROR) || (ev.flags & EV_EOF)) {
                // usually we have to close handle in this scenario, so let's simplify detection
                pevents |= PD_CLOSE;

                if (ev.flags & EV_ERROR)
                    pevents |= PD_POLLERR;

                if (ev.flags & EV_EOF)
                    pevents |= PD_POLLHUP;
            }

            pev->handler(pev, pevents);
        }

        pd_timers_run(ctx);
        timeout = pd_timers_next(ctx);
        pd__tcp_pending_close(ctx);

        if (ctx->after_tick)
            ctx->after_tick(ctx);
    }
}
