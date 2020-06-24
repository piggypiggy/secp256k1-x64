/******************************************************************************
 *                                                                            *
 * Copyright 2020 Meng-Shan Jiang                                             *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *    http://www.apache.org/licenses/LICENSE-2.0                              *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 *                                                                            *
 *****************************************************************************/

#include <secp256k1_x64/cpuid.h>
#include <secp256k1_x64/crypto.h>
#include <secp256k1_x64/secp256k1.h>
#ifdef _WIN32
# include <windows.h>
#elif defined(HAVE_PTHREAD)
# include <pthread.h>
#endif
#include "rand/rand_lcl.h"

static volatile int initialized;
static volatile int locked;

#ifdef _WIN32

static CRITICAL_SECTION _lock;
static volatile long    _lock_initialized;

int _crit_init(void)
{
    long status = 0L;

    while ((status = InterlockedCompareExchange(&_lock_initialized,
                                                1L, 0L)) == 1L) {
        Sleep(0);
    }

    switch (status) {
    case 0L:
        InitializeCriticalSection(&_lock);
        return InterlockedExchange(&_lock_initialized, 2L) == 1L ? 0 : -1;
    case 2L:
        return 0;
    default: /* should never be reached */
        return -1;
    }
}

int CRYPTO_crit_enter(void)
{
    if (_crit_init() != 0) {
        return -1; /* LCOV_EXCL_LINE */
    }
    EnterCriticalSection(&_lock);
    assert(locked == 0);
    locked = 1;

    return 0;
}

int CRYPTO_crit_leave(void)
{
    if (locked == 0) {
# ifdef EPERM
        errno = EPERM;
# endif
        return -1;
    }
    locked = 0;
    LeaveCriticalSection(&_lock);

    return 0;
}

#elif defined(HAVE_PTHREAD) && !defined(__EMSCRIPTEN__)

static pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;

int CRYPTO_crit_enter(void)
{
    int ret;

    if ((ret = pthread_mutex_lock(&_lock)) == 0) 
    {
        assert(locked == 0);
        locked = 1;
    }
    return ret;
}

int CRYPTO_crit_leave(void)
{
    if (locked == 0) {
# ifdef EPERM
        errno = EPERM;
# endif
        return -1;
    }
    locked = 0;

    return pthread_mutex_unlock(&_lock);
}

#elif defined(HAVE_ATOMIC_OPS) && !defined(__EMSCRIPTEN__)

static volatile int _lock;

int
CRYPTO_crit_enter(void)
{
# ifdef HAVE_NANOSLEEP
    struct timespec q;
    memset(&q, 0, sizeof q);
# endif
    while (__sync_lock_test_and_set(&_lock, 1) != 0) {
# ifdef HAVE_NANOSLEEP
        (void) nanosleep(&q, NULL);
# elif defined(__x86_64__) || defined(__i386__)
        __asm__ __volatile__ ("pause");
# endif
    }
    return 0;
}

int
CRYPTO_crit_leave(void)
{
    __sync_lock_release(&_lock);

    return 0;
}

#else

int
CRYPTO_crit_enter(void)
{
    return 0;
}

int
CRYPTO_crit_leave(void)
{
    return 0;
}

#endif

/* choose fastest implementation */
static int runtime_choose_best_implementation()
{
    runtime_choose_rand_implementation();
    return CRYPTO_OK;
}

static int init_globals()
{
    /* precompute table for secp256k1 generator */
    if (secp256k1_precompute_table_gen() == CRYPTO_ERR)
        goto end;

    return CRYPTO_OK;
end:
    return CRYPTO_ERR;
}

static void deinit_globals()
{

}

int CRYPTO_init()
{
    if (CRYPTO_crit_enter() != 0) 
        return CRYPTO_ERR;

    if (initialized != 0) {
        if (CRYPTO_crit_leave() != 0) 
            return CRYPTO_ERR;
        return CRYPTO_OK;
    }

    _runtime_get_cpu_features();

    if (init_globals() == CRYPTO_ERR) 
        abort();

    runtime_choose_best_implementation();

    initialized = 1;
    if (CRYPTO_crit_leave() != 0)
        return CRYPTO_ERR;

    return CRYPTO_OK;
}

int CRYPTO_deinit()
{
    if (CRYPTO_crit_enter() != 0) 
        return CRYPTO_ERR;

    if (initialized != 1) {
        if (CRYPTO_crit_leave() != 0) 
            return CRYPTO_ERR;
        return CRYPTO_OK;
    }

    deinit_globals();

    initialized = 0;
    if (CRYPTO_crit_leave() != 0)
        return CRYPTO_ERR;

    return CRYPTO_OK;
}