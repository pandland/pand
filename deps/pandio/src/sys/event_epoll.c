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
#include <sys/epoll.h>
#include <stdio.h>
#include "internal.h"


uint64_t pd_now() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}


void pd_io_init(pd_io_t *ctx) {
    ctx->poll_fd = epoll_create1(0);
    ctx->refs = 0;

    if (ctx->poll_fd == -1) {
        abort();
    }

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
    int unsigned flags = 0;

    if (event->flags & PD_POLLIN)
        flags |= EPOLLIN;

    if (event->flags & PD_POLLOUT)
        flags |= EPOLLOUT;

    struct epoll_event ev;
    ev.events = flags;
    ev.data.ptr = event;

    return epoll_ctl(ctx->poll_fd, EPOLL_CTL_MOD, fd, &ev);
}


int pd__event_add(pd_io_t *ctx, pd_event_t *event, pd_fd_t fd) {
    int unsigned flags = 0;

    if (event->flags & PD_POLLIN)
        flags |= EPOLLIN;

    if (event->flags & PD_POLLOUT)
        flags |= EPOLLOUT;

    struct epoll_event ev;
    ev.events = flags;
    ev.data.ptr = event;

    return epoll_ctl(ctx->poll_fd, EPOLL_CTL_ADD, fd, &ev);
}


int pd__event_del(pd_io_t *ctx, pd_fd_t fd) {
    return epoll_ctl(ctx->poll_fd, EPOLL_CTL_DEL, fd, NULL);
}


#define MAX_EVENTS 1024

void pd_io_run(pd_io_t *ctx) {
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event ev;
    int timeout = pd_timers_next(ctx);
    pd_event_t *pev;

    while (ctx->refs > 0) {
        int events_count =
                epoll_wait(ctx->poll_fd, events, MAX_EVENTS, timeout);

        if (events_count == -1 && errno == EINTR) {
            // interrupted by signal
            continue;
        }

        if (events_count == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        ctx->now = pd_now();

        for (int i = 0; i < events_count; ++i) {
            ev = events[i];
            unsigned pevents = 0;
            pev = ev.data.ptr;

            if (ev.events & EPOLLIN)
                pevents |= PD_POLLIN;

            if (ev.events & EPOLLOUT)
                pevents |= PD_POLLOUT;

            if (ev.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                // usually we have to close handle in this scenario, so let's simplify detection
                pevents |= PD_CLOSE;

                if (ev.events & EPOLLERR)
                    pevents |= PD_POLLERR;

                if (ev.events & EPOLLHUP)
                    pevents |= PD_POLLHUP;

                if (ev.events & EPOLLRDHUP)
                    pevents |= PD_POLLRDHUP;

            }

            pev->handler(pev, pevents);
        }

        pd_timers_run(ctx);
        timeout = pd_timers_next(ctx);
        pd__tcp_pending_close(ctx);
    }
}
