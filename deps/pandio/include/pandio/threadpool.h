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
#include "core.h"

#define PD_THREADPOOL_SIZE 4

struct pd_task_s {
    pd_io_t *ctx;
    void (*work)(struct pd_task_s*);  // work executed inside the thread pool.
    void (*done)(struct pd_task_s*); // callback executed by the main thread.
    void *udata;                    // pointer to some user's data.
    struct queue_node qnode;
};

typedef struct pd_task_s pd_task_t;

void pd_threadpool_init(size_t);

void pd_threadpool_end();

/* Submit a task to the thread pool */
int pd_task_submit(pd_io_t*, pd_task_t*);

void pd__task_done(pd_notifier_t*);
