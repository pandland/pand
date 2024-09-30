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

#include "pandio/threadpool.h"
#include "pandio/err.h"
#include "internal.h"

static size_t nthreads = 0;
static pd_thread_t *threads = NULL;
static size_t waiting_threads = 0;

static pd_mutex_t mux;
static pd_cond_t cond;

static struct queue tasks = { .head = NULL, .tail = NULL };
static size_t ntasks = 0;

static bool abort_threads = false;


int pd_task_submit(pd_io_t *ctx, pd_task_t *task) {
    if (abort_threads)
        return PD_ECANCELED;

    if (nthreads == 0)
        return PD_ECANCELED;

    task->ctx = ctx;
    queue_init_node(&task->qnode);
    pd_mutex_lock(&mux);
    queue_push(&tasks, &task->qnode);
    ntasks++;

    if (waiting_threads > 0)
        pd_cond_signal(&cond);

    pd_mutex_unlock(&mux);

    return 0;
}


void* pd__threadpool_exec(void *arg) {
    while (true) {
        pd_mutex_lock(&mux);
        assert(ntasks >= 0);

        while ((ntasks == 0) && (!abort_threads)) {
            waiting_threads++;
            pd_cond_wait(&cond, &mux);
            waiting_threads--;
        }

        if (abort_threads) {
            pd_cond_signal(&cond);
            pd_mutex_unlock(&mux);
            break;
        }

        struct queue_node *node = queue_pop(&tasks);
        pd_task_t *task = container_of(node, pd_task_t, qnode);
        ntasks--;

        pd_mutex_unlock(&mux);

        if (task->work)
            task->work(task);

        queue_init_node(&task->qnode);
        pd_mutex_lock(&mux);
        queue_push(&task->ctx->finished_tasks, &task->qnode);

        if (!task->ctx->task_signaled)
            pd_notifier_send(task->ctx->task_signal);

        pd_mutex_unlock(&mux);
    }

    return 0;
}


void pd__task_done(pd_notifier_t *notifier) {
    pd_io_t *ctx = notifier->ctx;

    struct queue tasks_done;
    queue_init(&tasks_done);

    pd_mutex_lock(&mux);

    // copy data from synchronized queue to the local queue
    // because we do not want to execute callbacks under mutex section.
    while (!queue_empty(&ctx->finished_tasks)) {
        struct queue_node *node = queue_pop(&ctx->finished_tasks);
        queue_push(&tasks_done, node);
    }

    ctx->task_signaled = false;
    pd_mutex_unlock(&mux);

    // execute done callbacks in the main thread
    while (!queue_empty(&tasks_done)) {
        struct queue_node *node = queue_pop(&tasks_done);
        pd_task_t *task = container_of(node, pd_task_t, qnode);

        if (task->done)
            task->done(task);
    }
}


void pd_threadpool_end() {
    if (nthreads == 0)
        return;

    pd_mutex_lock(&mux);
    abort_threads = true;
    pd_cond_signal(&cond);
    pd_mutex_unlock(&mux);

    for (int i = 0; i < nthreads; ++i) {
        pd_thread_join(&threads[i]);
    }

    free(threads);
    nthreads = 0;
    threads = NULL;

    pd_mutex_destroy(&mux);
    pd_cond_destroy(&cond);
}


void pd_threadpool_init(size_t n) {
    // init only once for sure
    if (nthreads != 0)
        return;

    abort_threads = false;
    nthreads = (n > 0) ? n : PD_THREADPOOL_SIZE;
    threads = malloc(nthreads * sizeof(pd_thread_t));

    pd_mutex_init(&mux);
    pd_cond_init(&cond);

    for (int i = 0; i < nthreads; ++i) {
        pd_thread_create(&threads[i], pd__threadpool_exec, NULL);
    }
}
