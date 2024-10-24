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

#ifdef WIN32
#include <wincrypt.h>

HCRYPTPROV hProv = 0;

pd_once_t init_control = PD_ONCE_INIT;
void pd__init_random_once(void) {
    // TODO: handle this error
    CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
}

int pd_random(void *bytes, size_t size) {
    if (pd_once(&init_control, pd__init_random_once) == -1)
        return PD_UNKNOWN;

    if (size == 0)
        return 0;

    if (CryptGenRandom(hProv, size, bytes))
        return 0;

    return PD_EIO;
}

#else
#include <fcntl.h>
#include <unistd.h>

int pd_random(void *bytes, size_t size) {
    if (size == 0)
        return 0;

    // TODO: maybe use pd_once as well to get fd only once?
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        return pd_errno();
    }

    ssize_t status;
    do {
        status = read(fd, bytes, size);
    } while (status == -1 && errno == EINTR);

    if (status == -1) {
        close(fd);
        return pd_errno();
    }

    close(fd);
    return 0;
}
#endif
