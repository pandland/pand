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

#include <stdio.h>
#include "pandio/core.h"
#include "pandio/timers.h"
#include <winsock2.h>
#include "internal.h"
#include "pandio/threadpool.h"

#define ENTRIES_MAX 128


static LARGE_INTEGER pd__freq;

uint64_t pd_now() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (li.QuadPart * 1000ULL) / pd__freq.QuadPart;
}


void pd_io_init(pd_io_t *ctx) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    QueryPerformanceFrequency(&pd__freq);

    ctx->refs = 0;
    ctx->poll_fd = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,NULL,
            0, 1);
    ctx->after_tick = NULL;

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
    event->data = NULL;
    event->bytes = 0;
    ZeroMemory(&event->overlapped, sizeof(event->overlapped));
}


void pd_io_run(pd_io_t *ctx) {
    pd_event_t *ev = NULL;
    OVERLAPPED_ENTRY entries[ENTRIES_MAX];
    ULONG count;
    BOOL result;
    int timeout = pd_timers_next(ctx);

    while (ctx->refs > 0) {
        result = GetQueuedCompletionStatusEx(
                ctx->poll_fd,
                entries,
                ENTRIES_MAX,
                &count,
                (timeout == -1) ? INFINITE : timeout,
                FALSE);

        ctx->now = pd_now();

        if (result) {
           for (int i = 0; i < count; ++i) {
               LPOVERLAPPED overlapped = entries[i].lpOverlapped;
               if (!overlapped) {
                   continue;
               }

               // OVERLAPPED is first member of pd_event_t, so we can cast to it.
               ev = (pd_event_t *)overlapped;
               ev->bytes = entries[i].dwNumberOfBytesTransferred;

               if (ev->handler)
                   ev->handler(ev);
           }
       } else if (GetLastError() != WAIT_TIMEOUT) {
           printf("Something went wrong\n");
           abort();
       }

        pd_timers_run(ctx);
        timeout = pd_timers_next(ctx);
        pd__tcp_pending_close(ctx);

        if (ctx->after_tick)
            ctx->after_tick(ctx);
    }
}
