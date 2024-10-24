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
#include "pandio/err.h"
#include <process.h>


void pd_sleep(unsigned msec) {
    Sleep(msec);
}


void pd_mutex_init(pd_mutex_t *mux) {
    InitializeCriticalSection(mux);
}


void pd_mutex_lock(pd_mutex_t *mux) {
    EnterCriticalSection(mux);
}


void pd_mutex_unlock(pd_mutex_t *mux) {
    LeaveCriticalSection(mux);
}


void pd_mutex_destroy(pd_mutex_t *mux) {
    DeleteCriticalSection(mux);
}


void pd_cond_init(pd_cond_t *cond) {
    InitializeConditionVariable(cond);
}


void pd_cond_wait(pd_cond_t *cond, pd_mutex_t *mux) {
    SleepConditionVariableCS(cond, mux, INFINITE);
}


void pd_cond_signal(pd_cond_t *cond) {
    WakeConditionVariable(cond);
}


void pd_cond_broadcast(pd_cond_t *cond) {
    WakeAllConditionVariable(cond);
}


// On Windows it is not necessary.
void pd_cond_destroy(pd_cond_t *cond) {}


struct pd__thread_runner {
    void*(*func)(void*);
    void *args;
};

unsigned __stdcall pd__thread_run(void *arg) {
    struct pd__thread_runner *runner = arg;
    runner->func(runner->args);
    free(runner);

    return 0;
}

void pd_thread_create(pd_thread_t *thread, void* (func)(void*), void *arg) {
    struct pd__thread_runner *runner = malloc(sizeof (struct pd__thread_runner));
    if (runner == NULL) {
        *thread = NULL;
        return;
    }

    runner->func = func;
    runner->args = arg;

    HANDLE _thread;
    _thread = (HANDLE)_beginthreadex(NULL, 0, pd__thread_run, runner, 0, NULL);
    if (_thread == NULL) {
        free(runner);
    }

    *thread = _thread;
}


void pd_thread_join(pd_thread_t *thread) {
    WaitForSingleObject(*thread, INFINITE);
}

BOOL pd__init_once(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *Context) {
    void (*init_routine)(void) = Parameter;
    init_routine();
    return TRUE;
}

int pd_once(pd_once_t *once_control, void (*init_routine)(void)) {
    if (InitOnceExecuteOnce(once_control, pd__init_once, init_routine, NULL))
        return 0;

    return pd_errno();
}
