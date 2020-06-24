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

#pragma once
/* ------------------------------------------------------------------------- */

#if defined(__GNUC__) && defined(__x86_64__)
#include "sys/time.h"
#define u64_fmt "%lu"                                                        \

#define BENCH_VARS                                                            \
  unsigned int start_lo, start_hi;                                                     \
  unsigned int ticks_lo, ticks_hi;                                             \
  uint64_t start_time, total_time                                                \

#define COUNTER_START()                                                         \
  __asm__ volatile                                                            \
    ("\n        rdtsc"                                                        \
     : "=a" (start_lo), "=d" (start_hi))

#define COUNTER_STOP()                                                          \
  __asm__ volatile                                                            \
    ("\n        rdtsc"                                                        \
     "\n        subl %2, %%eax"                                               \
     "\n        sbbl %3, %%edx"                                               \
     : "=&a" (ticks_lo), "=&d" (ticks_hi)                                     \
     : "g" (start_lo), "g" (start_hi))

static uint64_t gettime_u64(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_usec + (uint64_t)tv.tv_sec * 1000000UL;

#define TIMER_START()                                                           \
    start_time = gettime_u64()

#define TIMER_STOP()                                                           \
    total_time = gettime_u64() - start_time   
}

#define TICKS() ((uint64_t)ticks_lo + 4294967296UL * (uint64_t)ticks_hi)

/* ------------------------------------------------------------------------- */

#elif defined(_MSC_VER) && (defined(_M_AMD64) || defined(_M_X64))
#include <intrin.h>
#pragma intrinsic(__rdtsc)

#define BENCH_VARS                                                                            \
unsigned __int64 start_ticks, end_ticks;                                                      \
unsigned __int64 start_time, total_time                                                                  

#define COUNTER_START() start_ticks = __rdtsc()                                                   

#define COUNTER_STOP() end_ticks = __rdtsc()                                                     

static unsigned __int64 gettime_i64()
{
	FILETIME ft;
	unsigned __int64 ret;
	GetSystemTimeAsFileTime(&ft);
	ret = (unsigned __int64)ft.dwLowDateTime | ((unsigned __int64)ft.dwHighDateTime) << 32;
	return ret;
}

#define TIMER_START()                                                              \
    start_time = gettime_i64()

#define TIMER_STOP()                                                              \
    total_time = (gettime_i64() - start_time) / 10

#define TICKS() (end_ticks - start_ticks)

/* ------------------------------------------------------------------------- */

#else

#error "add timer"

/* ------------------------------------------------------------------------- */

#endif