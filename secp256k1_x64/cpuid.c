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

#include <stddef.h>
#include <stdint.h>
#include <secp256k1_x64/cpuid.h>
#include "config.h"

#ifdef HAVE_ANDROID_GETCPUFEATURES
# include <cpu-features.h>
#endif

#ifdef HAVE_IMMINTRIN_H
# include <immintrin.h>
#endif

typedef struct CPUFeatures_ {
    int initialized;
    int has_neon;
    int has_sse2;
    int has_sse3;
    int has_ssse3;
    int has_sse41;
    int has_avx;
    int has_avx2;
    int has_bmi2;
} CPUFeatures;

static CPUFeatures _cpu_features;

unsigned int cpu_info[4];

#define CPUID_EBX_AVX2    0x00000020
#define CPUID_EBX_BMI2    0x00000100
#define CPUID_EBX_AVX512F 0x00010000

#define CPUID_ECX_SSE3    0x00000001
#define CPUID_ECX_SSSE3   0x00000200
#define CPUID_ECX_SSE41   0x00080000
#define CPUID_ECX_XSAVE   0x04000000
#define CPUID_ECX_OSXSAVE 0x08000000
#define CPUID_ECX_AVX     0x10000000

#define CPUID_EDX_SSE2    0x04000000

#define XCR0_SSE       0x00000002
#define XCR0_AVX       0x00000004

static void _cpuid(unsigned int cpu_info[4U], const unsigned int cpu_info_type)
{
#if defined(_MSC_VER) && \
    (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
    __cpuid((int *) cpu_info, cpu_info_type);
#else
    cpu_info[0] = cpu_info[1] = cpu_info[2] = cpu_info[3] = 0;
# ifdef __i386__
    __asm__ __volatile__(
        "pushfl; pushfl; "
        "popl %0; "
        "movl %0, %1; xorl %2, %0; "
        "pushl %0; "
        "popfl; pushfl; popl %0; popfl"
        : "=&r"(cpu_info[0]), "=&r"(cpu_info[1])
        : "i"(0x200000));
    if (((cpu_info[0] ^ cpu_info[1]) & 0x200000) == 0x0) {
        return; /* LCOV_EXCL_LINE */
    }
# endif
# ifdef __i386__
    __asm__ __volatile__("xchgl %%ebx, %k1; cpuid; xchgl %%ebx, %k1"
                         : "=a"(cpu_info[0]), "=&r"(cpu_info[1]),
                           "=c"(cpu_info[2]), "=d"(cpu_info[3])
                         : "0"(cpu_info_type), "2"(0U));
# elif defined(__x86_64__)
    __asm__ __volatile__("xchgq %%rbx, %q1; cpuid; xchgq %%rbx, %q1"
                         : "=a"(cpu_info[0]), "=&r"(cpu_info[1]),
                           "=c"(cpu_info[2]), "=d"(cpu_info[3])
                         : "0"(cpu_info_type), "2"(0U));
# else
    __asm__ __volatile__("cpuid"
                         : "=a"(cpu_info[0]), "=b"(cpu_info[1]),
                           "=c"(cpu_info[2]), "=d"(cpu_info[3])
                         : "0"(cpu_info_type), "2"(0U));
# endif

#endif
}

static int _runtime_intel_cpu_features(CPUFeatures * const cpu_features)
{
    unsigned int id;
    unsigned int cpu_info1[4];
    unsigned int cpu_info7[4];

    _cpuid(cpu_info, 0x0);
    if ((id = cpu_info[0]) == 0U)
        return -1; /* LCOV_EXCL_LINE */

    _cpuid(cpu_info1, 0x00000001);

    cpu_features->has_sse2 = ((cpu_info1[3] & CPUID_EDX_SSE2) != 0x0);

    cpu_features->has_sse3 = ((cpu_info1[2] & CPUID_ECX_SSE3) != 0x0);

    cpu_features->has_ssse3 = ((cpu_info1[2] & CPUID_ECX_SSSE3) != 0x0);

    cpu_features->has_sse41 = ((cpu_info1[2] & CPUID_ECX_SSE41) != 0x0);

    cpu_features->has_avx = 0;
    if ((cpu_info1[2] & (CPUID_ECX_AVX | CPUID_ECX_XSAVE | CPUID_ECX_OSXSAVE)) ==
        (CPUID_ECX_AVX | CPUID_ECX_XSAVE | CPUID_ECX_OSXSAVE)) {
        unsigned int xcr0 = 0U;
# if (defined(_MSC_VER) && defined(_XCR_XFEATURE_ENABLED_MASK) && _MSC_FULL_VER >= 160040219)
        xcr0 = (unsigned int) _xgetbv(0);
# elif defined(_MSC_VER) && defined(_M_IX86)
        /*
         * Visual Studio documentation states that eax/ecx/edx don't need to
         * be preserved in inline assembly code. But that doesn't seem to
         * always hold true on Visual Studio 2010.
         */
        __asm {
            push eax
            push ecx
            push edx
            xor ecx, ecx
            _asm _emit 0x0f _asm _emit 0x01 _asm _emit 0xd0
            mov xcr0, eax
            pop edx
            pop ecx
            pop eax
        }
# else
        __asm__ __volatile__(".byte 0x0f, 0x01, 0xd0" /* XGETBV */
                             : "=a"(xcr0)
                             : "c"((unsigned int) 0U)
                             : "%edx");
# endif
        if ((xcr0 & (XCR0_SSE | XCR0_AVX)) == (XCR0_SSE | XCR0_AVX)) {
            cpu_features->has_avx = 1;
        }
    }
    
    /* mbi(mulx, addx, etc), avx2 */
    _cpuid(cpu_info7, 0x00000007);

    cpu_features->has_bmi2 = ((cpu_info7[1] & CPUID_EBX_BMI2) != 0x0);

    cpu_features->has_avx2 = 0;
    if (cpu_features->has_avx)
        cpu_features->has_avx2 = ((cpu_info7[1] & CPUID_EBX_AVX2) != 0x0);

    cpu_info[0] = 0;
    cpu_info[1] = cpu_info1[2];
    cpu_info[2] = cpu_info7[1]; /* mulx, addx */
    cpu_info[3] = 0;

    return CRYPTO_OK;
}

int _runtime_get_cpu_features()
{
    int ret = -1;

    ret &= _runtime_intel_cpu_features(&_cpu_features);
    _cpu_features.initialized = 1;

    return ret;
}

int runtime_has_sse2(void)
{
    return _cpu_features.has_sse2;
}

int runtime_has_sse3(void)
{
    return _cpu_features.has_sse3;
}

int runtime_has_ssse3(void)
{
    return _cpu_features.has_ssse3;
}

int runtime_has_sse41(void)
{
    return _cpu_features.has_sse41;
}

int runtime_has_avx(void)
{
    return _cpu_features.has_avx;
}

int runtime_has_avx2(void)
{
    return _cpu_features.has_avx2;
}

