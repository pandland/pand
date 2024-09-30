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

#ifndef _WIN32
#include <dlfcn.h>
#endif


pd_dlib_t pd_dlopen(const char *libname) {
#ifdef _WIN32
    return LoadLibrary(libname);
#else
    return dlopen(libname, RTLD_LAZY);
#endif
}


void* pd_dlsym(pd_dlib_t lib, const char *funcname) {
#ifdef _WIN32
    return (void*) GetProcAddress(lib, funcname);
#else
    return dlsym(lib, funcname);
#endif
}


int pd_dlclose(pd_dlib_t lib) {
#ifdef _WIN32
    return FreeLibrary(lib) ? 0 : 1;
#else
    return dlclose(lib);
#endif
}


const char* pd_dlerror() {
#ifdef _WIN32
    static char error[256];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, error, 256, NULL);
    return error;
#else
    return dlerror();
#endif
}
