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

#include "pandio/timers.h"
#include "internal.h"

/* get timer struct pointer from the heap node */
#define pd_timer_unwrap(ptr) \
  (ptr == NULL) ? NULL : container_of(ptr, pd_timer_t, hnode)


int pd_timers_comparator(struct heap_node *a,
                          struct heap_node *b) {
    pd_timer_t *child = pd_timer_unwrap(a);
    pd_timer_t *parent = pd_timer_unwrap(b);

    return child->timeout < parent->timeout;
}


void pd_timers_heap_init(pd_io_t *ctx) {
    heap_init(&ctx->timers, pd_timers_comparator);
}


void pd_timer_init(pd_io_t *ctx, pd_timer_t *timer) {
    timer->ctx = ctx;
    timer->timeout = 0;
    timer->interval = 0;
    timer->active = false;
    timer->data = NULL;
    timer->on_timeout = NULL;
    heap_init_node(&timer->hnode);
}


void pd_timer_start(pd_timer_t *timer,
                     pd_timer_callback_t cb,
                     uint64_t timeout) {
    if (timer->active) {
        return;
    }

    timer->active = true;
    timer->timeout = timer->ctx->now + timeout;
    timer->on_timeout = cb;
    timer->ctx->refs++;
    heap_insert(&timer->ctx->timers, &timer->hnode);
}


void pd_timer_repeat(pd_timer_t *timer,
                      pd_timer_callback_t cb,
                      uint64_t interval) {
    timer->interval = interval;
    pd_timer_start(timer, cb, interval);
}


void pd_timer_stop(pd_timer_t *timer) {
    if (!timer->active) {
        return;
    }

    timer->active = false;
    heap_remove(&timer->ctx->timers, &timer->hnode);
    timer->ctx->refs--;
}


void pd_timers_run(pd_io_t *ctx) {
    pd_timer_t *min;

    while(true) {
        min = pd_timer_unwrap(heap_peek(&ctx->timers));
        if (min == NULL)
            break;

        if (min->timeout > ctx->now)
            break;

        pd_timer_stop(min);

        // very unlikely scenario to be NULL, but still...
        if (min->on_timeout)
            min->on_timeout(min);

        if (min->interval != 0)
            pd_timer_start(min, min->on_timeout, min->interval);
    }
}


// return next timeout - useful for epoll
int pd_timers_next(pd_io_t *ctx) {
    pd_timer_t *min = pd_timer_unwrap(heap_peek(&ctx->timers));
    if (min == NULL)
        return -1;

    uint64_t next_timeout = min->timeout - ctx->now;
    return (next_timeout >= INT_MAX) ? INT_MAX : (int)next_timeout;
}
