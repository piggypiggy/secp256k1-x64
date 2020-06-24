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

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "config.h"

# ifdef __cplusplus
extern "C" {
# endif

# define CRYPTO_OK      1
# define CRYPTO_ERR     0

# if defined(SIXTY_FOUR_BIT_LONG)
#  define BN_ULONG unsigned long
# elif defined(SIXTY_FOUR_BIT)
#  define BN_ULONG unsigned long long
# else
#  error ""
# endif

# define P256_LIMBS 4
# define BN_BYTES   8

/* _inline: portable inline definition usable in public headers */
# if !defined(inline) && !defined(__cplusplus)
#  if defined(__STDC_VERSION__) && __STDC_VERSION__>=199901L
   /* just use inline */
#   define _inline inline
#  elif defined(__GNUC__) && __GNUC__>=2
#   define _inline __inline__
#  elif defined(_MSC_VER)
  /*
   * Visual Studio: inline is available in C++ only, however
   * __inline is available for C, see
   * http://msdn.microsoft.com/en-us/library/z8y1yy88.aspx
   */
#   define _inline __inline
#  else
#   define _inline
#  endif
# else
#  define _inline inline
# endif

# ifndef BUILD_STATIC
#  ifdef _WIN32
#    ifdef BUILD_SHARED
#      define X64_EXPORT __declspec(dllexport)
#   else
#     define X64_EXPORT __declspec(dllimport)
    #endif
# else
#   if (defined(__GNUC__) && __GNUC__>= 4)
#     define X64_EXPORT __attribute__ ((visibility ("default")))
#   endif
# endif
#else
# define X64_EXPORT
#endif

# define ORDER_BIG_ENDIAN     0
# define ORDER_LITTLE_ENDIAN  1

# if defined(__GNUC__) && __GNUC__>=2
#  if defined(__x86_64) || defined(__x86_64__)
#   define BSWAP8(x) ({ uint64_t ret_=(x);                   \
                        asm ("bswapq %0"                \
                        : "+r"(ret_));   ret_;          })
#   define BSWAP4(x) ({ unsigned int ret_=(x);                   \
                        asm ("bswapl %0"                \
                        : "+r"(ret_));   ret_;          })
#  elif (defined(__i386) || defined(__i386__)) && !defined(I386_ONLY)
#   define BSWAP8(x) ({ unsigned int lo_=(uint64_t)(x)>>32,hi_=(x);   \
                        asm ("bswapl %0; bswapl %1"     \
                        : "+r"(hi_),"+r"(lo_));         \
                        (uint64_t)hi_<<32|lo_;               })
#   define BSWAP4(x) ({ unsigned int ret_=(x);                   \
                        asm ("bswapl %0"                \
                        : "+r"(ret_));   ret_;          })
#  elif defined(__aarch64__)
#   define BSWAP8(x) ({ uint64_t ret_;                       \
                        asm ("rev %0,%1"                \
                        : "=r"(ret_) : "r"(x)); ret_;   })
#   define BSWAP4(x) ({ unsigned int ret_;                       \
                        asm ("rev %w0,%w1"              \
                        : "=r"(ret_) : "r"(x)); ret_;   })
#  elif (defined(__arm__) || defined(__arm))
#   define BSWAP8(x) ({ unsigned int lo_=(uint64_t)(x)>>32,hi_=(x);   \
                        asm ("rev %0,%0; rev %1,%1"     \
                        : "+r"(hi_),"+r"(lo_));         \
                        (uint64_t)hi_<<32|lo_;               })
#   define BSWAP4(x) ({ unsigned int ret_;                       \
                        asm ("rev %0,%1"                \
                        : "=r"(ret_) : "r"((unsigned int)(x)));  \
                        ret_;                           })
#  endif
# elif defined(_MSC_VER)
#  if _MSC_VER>=1300
#   include <stdlib.h>
#   pragma intrinsic(_byteswap_uint64,_byteswap_ulong)
#   define BSWAP8(x)    _byteswap_uint64((uint64_t)(x))
#   define BSWAP4(x)    _byteswap_ulong((unsigned int)(x))
#  elif defined(_M_IX86)
__inline unsigned int _bswap4(unsigned int val)
{
_asm mov eax, val _asm bswap eax}
#   define BSWAP4(x)    _bswap4(x)
#  endif
# endif

# define ROTL32(a, n) (((a) << n) | ((a) >> (32-n)))
# define ROTR32(a, n) (((a) >> n) | ((a) << (32-n)))
# define ROTL64(a, n) (((a) << n) | ((a) >> (64-n)))
# define ROTR64(a, n) (((a) >> n) | ((a) << (64-n)))

# if !defined(BSWAP8)
#  define BSWAP8(x)  ((ROTL64(x,  8) & 0x000000ff000000ff) | \
                      (ROTL64(x, 24) & 0x0000ff000000ff00) | \
                      (ROTR64(x, 24) & 0x00ff000000ff0000) | \
                      (ROTR64(x,  8) & 0xff000000ff000000))
# endif

# if !defined(BSWAP4)
#  define BSWAP4(x)  ((ROTL32(x, 8) & 0x00ff00ff) | (ROTR32(x, 8) & 0xff00ff00))
# endif

X64_EXPORT unsigned int to_be32(const unsigned int in);
X64_EXPORT unsigned int to_le32(const unsigned int in);

/* convert u8 array to one u32 integer, inlen <= 4 */
X64_EXPORT int u8_to_u32(unsigned int *out, const unsigned char *in, int inlen, int order);

/* convert one u32 integer to u8 array */
X64_EXPORT int u32_to_u8(unsigned char out[4], const unsigned int in, int order);

/* convert u8 array to one u64 integer, inlen <= 8 */
X64_EXPORT int u8_to_u64(uint64_t *out, const unsigned char *in, int inlen, int order);

/* convert one u64 integer to u8 array */
X64_EXPORT int u64_to_u8(unsigned char out[8], const uint64_t in, int order);

/* convert hex array to one u64 integer, inlen <= 16 */
X64_EXPORT int hex_to_u64(uint64_t *out, const unsigned char *in, int inlen, int order);

/* convert one u32 integer to hex array */
X64_EXPORT int u32_to_hex(unsigned char out[8], const unsigned int in, int order);

/* convert one u64 integer to hex array */
X64_EXPORT int u64_to_hex(unsigned char out[16], const uint64_t in, int order);

/* convert one u8 array to hex array */
X64_EXPORT int u8_to_hex(unsigned char *out, const unsigned char *in, int in_len);

/* convert one hex array to u8 array */
X64_EXPORT int hex_to_u8(unsigned char *out, const unsigned char *in, int in_len);

X64_EXPORT void print_hex(const char *name, unsigned char *s, int slen);

#ifdef __cplusplus
}
#endif