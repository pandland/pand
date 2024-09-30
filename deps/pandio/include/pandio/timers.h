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
#include <limits.h>
#include <stdint.h>
#include "heap.h"
#include "core.h"

struct pd_timer_s;
typedef void (*pd_timer_callback_t)(struct pd_timer_s*);

struct pd_timer_s {
    pd_io_t *ctx;
    uint64_t timeout;
    uint64_t interval;
    bool active;
    void *data;       // custom data for callback
    struct heap_node hnode;
    pd_timer_callback_t on_timeout;
};

typedef struct pd_timer_s pd_timer_t;

void pd_timer_init(pd_io_t*, pd_timer_t*);

void pd_timer_start(pd_timer_t*, pd_timer_callback_t, uint64_t);

void pd_timer_repeat(pd_timer_t*, pd_timer_callback_t, uint64_t);

/* safe to call many times */
void pd_timer_stop(pd_timer_t*);

void pd_timers_heap_init(pd_io_t*);

void pd_timers_run(pd_io_t*);

int pd_timers_next(pd_io_t*);
