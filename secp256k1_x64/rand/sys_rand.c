/*
 * ISC License
 *
 * Copyright (c) 2013-2020
 * Frank Denis <j at pureftpd dot org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#ifndef _WIN32
# include <unistd.h>
#endif

#include <stdlib.h>
#include "rand_lcl.h"
#ifndef _WIN32
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
#endif
#ifdef __linux__
# ifdef __dietlibc__
#  define _LINUX_SOURCE
#  include <sys/random.h>
#  define HAVE_LINUX_COMPATIBLE_GETRANDOM
# else /* __dietlibc__ */
#  include <sys/syscall.h>
#  if defined(SYS_getrandom) && defined(__NR_getrandom)
#   define getrandom(B, S, F) syscall(SYS_getrandom, (B), (int) (S), (F))
#   define HAVE_LINUX_COMPATIBLE_GETRANDOM
#  endif
# endif /* __dietlibc */
#elif defined(__FreeBSD__)
# include <sys/param.h>
# if defined(__FreeBSD_version) && __FreeBSD_version >= 1200000
#  include <sys/random.h>
#  define HAVE_LINUX_COMPATIBLE_GETRANDOM
# endif
#endif
#if !defined(NO_BLOCKING_RANDOM_POLL) && defined(__linux__)
# define BLOCK_ON_DEV_RANDOM
#endif
#ifdef BLOCK_ON_DEV_RANDOM
# include <poll.h>
#endif

#ifdef _WIN32
/* `RtlGenRandom` is used over `CryptGenRandom` on Microsoft Windows based systems:
 *  - `CryptGenRandom` requires pulling in `CryptoAPI` which causes unnecessary
 *     memory overhead if this API is not being used for other purposes
 *  - `RtlGenRandom` is thus called directly instead. A detailed explanation
 *     can be found here: https://blogs.msdn.microsoft.com/michael_howard/2005/01/14/cryptographically-secure-random-number-on-windows-without-using-cryptoapi/
 *
 * In spite of the disclaimer on the `RtlGenRandom` documentation page that was
 * written back in the Windows XP days, this function is here to stay. The CRT
 * function `rand_s()` directly depends on it, so touching it would break many
 * applications released since Windows XP.
 *
 * Also note that Rust, Firefox and BoringSSL (thus, Google Chrome and everything
 * based on Chromium) also depend on it, and that libsodium allows the RNG to be
 * replaced without patching nor recompiling the library.
 */
# include <windows.h>
# define RtlGenRandom SystemFunction036
# if defined(__cplusplus)
extern "C"
# endif
BOOLEAN NTAPI RtlGenRandom(PVOID RandomBuffer, ULONG RandomBufferLength);
# pragma comment(lib, "advapi32.lib")
#endif

#if defined(__OpenBSD__) || defined(__CloudABI__)
# define HAVE_SAFE_ARC4RANDOM 1
#endif

#ifndef SSIZE_MAX
# define SSIZE_MAX (SIZE_MAX / 2 - 1)
#endif

#ifdef HAVE_SAFE_ARC4RANDOM

static unsigned int
randombytes_sysrandom(void)
{
    return arc4random();
}

static void
randombytes_sysrandom_stir(void)
{
}

static void
randombytes_sysrandom_buf(void * const buf, const size_t size)
{
    arc4random_buf(buf, size);
}

#else /* __OpenBSD__ */

typedef struct SysRandom_ {
    int random_data_source_fd;
    int initialized;
    int getrandom_available;
} SysRandom;

static SysRandom stream = {
   -1,
   0,
   0,
};

#ifndef _WIN32
static ssize_t safe_read(const int fd, void * const buf_, size_t size)
{
    unsigned char *buf = (unsigned char *) buf_;
    ssize_t        readnb;

    assert(size > (size_t) 0U);
    assert(size <= SSIZE_MAX);
    do {
        while ((readnb = read(fd, buf, size)) < (ssize_t) 0 &&
               (errno == EINTR || errno == EAGAIN)); /* LCOV_EXCL_LINE */
        if (readnb < (ssize_t) 0) 
        {
            return readnb; /* LCOV_EXCL_LINE */
        }
        if (readnb == (ssize_t) 0) 
        {
            break; /* LCOV_EXCL_LINE */
        }
        size -= (size_t) readnb;
        buf += readnb;
    } while (size > (ssize_t) 0);

    return (ssize_t) (buf - (unsigned char *) buf_);
}
#endif

#ifndef _WIN32
# if defined(__linux__) && !defined(USE_BLOCKING_RANDOM) && !defined(NO_BLOCKING_RANDOM_POLL)
static int randombytes_block_on_dev_random(void)
{
    struct pollfd pfd;
    int           fd;
    int           pret;

    fd = open("/dev/random", O_RDONLY);
    if (fd == -1) 
    {
        return 0;
    }
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    do 
    {
        pret = poll(&pfd, 1, -1);
    } while (pret < 0 && (errno == EINTR || errno == EAGAIN));
    if (pret != 1) 
    {
        (void) close(fd);
        errno = EIO;
        return -1;
    }
    return close(fd);
}
# endif

static int randombytes_sysrandom_random_dev_open(void)
{
/* LCOV_EXCL_START */
    struct stat        st;
    static const char *devices[] = {
# ifndef USE_BLOCKING_RANDOM
        "/dev/urandom",
# endif
        "/dev/random", NULL
    };
    const char       **device = devices;
    int                fd;

# if defined(__linux__) && !defined(USE_BLOCKING_RANDOM) && !defined(NO_BLOCKING_RANDOM_POLL)
    if (randombytes_block_on_dev_random() != 0) 
    {
        return -1;
    }
# endif
    do 
    {
        fd = open(*device, O_RDONLY);
        if (fd != -1) {
            if (fstat(fd, &st) == 0 &&
# ifdef __COMPCERT__
                1
# elif defined(S_ISNAM)
                (S_ISNAM(st.st_mode) || S_ISCHR(st.st_mode))
# else
                S_ISCHR(st.st_mode)
# endif
               ) {
# if defined(F_SETFD) && defined(FD_CLOEXEC)
                (void) fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
# endif
                return fd;
            }
            (void) close(fd);
        } else if (errno == EINTR) 
        {
            continue;
        }
        device++;
    } while (*device != NULL);

    errno = EIO;
    return -1;
/* LCOV_EXCL_STOP */
}

# if defined(__dietlibc__) || (defined(SYS_getrandom) && defined(__NR_getrandom))
static int _randombytes_linux_getrandom(void * const buf, const size_t size)
{
    int readnb;

    assert(size <= 256U);
    do {
#  ifdef __dietlibc__
        readnb = getrandom(buf, size, 0);
#  else
        readnb = syscall(SYS_getrandom, buf, (int) size, 0);
#  endif
    } while (readnb < 0 && (errno == EINTR || errno == EAGAIN));
    
    return (readnb == (int) size) - 1;
}

static int randombytes_linux_getrandom(void * const buf_, size_t size)
{
    unsigned char *buf = (unsigned char *) buf_;
    size_t         chunk_size = 256U;

    do {
        if (size < chunk_size) 
        {
            chunk_size = size;
            assert(chunk_size > (size_t) 0U);
        }
        if (_randombytes_linux_getrandom(buf, chunk_size) != 0) 
        {
            return -1;
        }
        size -= chunk_size;
        buf += chunk_size;
    } while (size > (size_t) 0U);

    return 0;
}
# endif

static void randombytes_sysrandom_init(void)
{
    const int errno_save = errno;

# if defined(SYS_getrandom) && defined(__NR_getrandom)
    {
        unsigned char fodder[16];

        if (randombytes_linux_getrandom(fodder, sizeof fodder) == 0) 
        {
            stream.getrandom_available = 1;
            errno = errno_save;
            return;
        }
        stream.getrandom_available = 0;
    }
# endif

    if ((stream.random_data_source_fd =
         randombytes_sysrandom_random_dev_open()) == -1) 
    {
        //sodium_misuse(); /* LCOV_EXCL_LINE */
    }
    errno = errno_save;
}

#else /* _WIN32 */

static void randombytes_sysrandom_init(void)
{
}
#endif

static void randombytes_sysrandom_stir(void)
{
    if (stream.initialized == 0) 
    {
        randombytes_sysrandom_init();
        stream.initialized = 1;
    }
}

static void randombytes_sysrandom_stir_if_needed(void)
{
    if (stream.initialized == 0) 
    {
        randombytes_sysrandom_stir();
    }
}

static int randombytes_sysrandom_buf(void * const buf, const size_t size)
{
    randombytes_sysrandom_stir_if_needed();
#if defined(ULONG_LONG_MAX) && defined(SIZE_MAX)
# if SIZE_MAX > ULONG_LONG_MAX
    /* coverity[result_independent_of_operands] */
    assert(size <= ULONG_LONG_MAX);
# endif
#endif
#ifndef _WIN32
# if defined(SYS_getrandom) && defined(__NR_getrandom)
    if (stream.getrandom_available != 0) 
    {
        if (randombytes_linux_getrandom(buf, size) != 0) 
        {
            return -1;
        }
        return 0;
    }
# endif
    if (stream.random_data_source_fd == -1 ||
        safe_read(stream.random_data_source_fd, buf, size) != (ssize_t) size) 
    {
        return -1;
    }
#else
   // COMPILER_ASSERT(randombytes_BYTES_MAX <= 0xffffffffUL);
    if (size > (size_t) 0xffffffffUL) {
        return -1;
    }
    if (! RtlGenRandom((PVOID) buf, (ULONG) size)) {
        return -1;
    }
#endif
    return 0;
}

int sys_rand(unsigned char *rnd, size_t rnd_len)
{
    return 1 + randombytes_sysrandom_buf(rnd, rnd_len);
}

#endif /* __OpenBSD__ */

/* IMPLEMENTAION */
static const RAND_IMPL _SYS_RAND_IMPL = {
    "system api",
    sys_rand,
};

const RAND_IMPL *SYS_RAND_IMPL()
{
    return &_SYS_RAND_IMPL;
}