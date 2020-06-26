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

#include <stdio.h>
#include <secp256k1_x64/fp256.h>
#include <secp256k1_x64/rand.h>

/* TODO : add bn256 arith, including addition, muliplication, division, etc. */

void fp256_copy(BN_ULONG r[P256_LIMBS], const BN_ULONG a[P256_LIMBS])
{
    r[0] = a[0];
    r[1] = a[1];
    r[2] = a[2];
    r[3] = a[3];
}

void fp256_set_one(BN_ULONG r[P256_LIMBS])
{
    r[0] = 1ULL;
    r[1] = 0ULL;
    r[2] = 0ULL;
    r[3] = 0ULL;
}

void fp256_set_word(BN_ULONG r[P256_LIMBS], const BN_ULONG w)
{
    r[0] = w;
    r[1] = 0ULL;
    r[2] = 0ULL;
    r[3] = 0ULL;
}

/* 1 : r = 0
 * 0 : r != 0
 */ 
int fp256_is_zero(const BN_ULONG r[P256_LIMBS])
{
    return (r[0] | r[1] | r[2] | r[3]) == 0ULL;
}

/* 1  : a > b
 * 0  : a = b
 * -1 : a < b
 */ 
int fp256_cmp(const BN_ULONG a[P256_LIMBS], const BN_ULONG b[P256_LIMBS])
{
    int i;

    for (i = 3; i >= 0; i--) {
        if (a[i] > b[i])
            return 1;
        if (a[i] < b[i])
            return -1;
    }
    return 0;
}

/* convert hex string to 256bit number */
int fp256_set_hex(BN_ULONG r[P256_LIMBS], unsigned char *hex, int hexlen)
{
    int i;
    r[0] = 0ULL;
    r[1] = 0ULL;
    r[2] = 0ULL;
    r[3] = 0ULL;

    if (hexlen < 0 || hexlen > 64)
        return CRYPTO_ERR;

    i = 0;
    while (hexlen >= 16) {
        if (hex_to_u64(&r[i], hex + hexlen - 16, 16, ORDER_BIG_ENDIAN) == CRYPTO_ERR)
            return CRYPTO_ERR;
        hexlen -= 16;
        i++;
    }

    if (hexlen > 0) {
        if (hex_to_u64(&r[i], hex, hexlen, ORDER_BIG_ENDIAN) == CRYPTO_ERR)
            return CRYPTO_ERR;
    }

    return CRYPTO_OK;
}

/* convert 256bit number to hex string */
int fp256_get_hex(unsigned char hex[P256_LIMBS*16], const BN_ULONG a[P256_LIMBS])
{
    if (hex == NULL)
        return CRYPTO_ERR;

    u64_to_hex(hex,      a[3], ORDER_BIG_ENDIAN);
    u64_to_hex(hex + 16, a[2], ORDER_BIG_ENDIAN);
    u64_to_hex(hex + 32, a[1], ORDER_BIG_ENDIAN);
    u64_to_hex(hex + 48, a[0], ORDER_BIG_ENDIAN);
    return CRYPTO_OK;
}

/* convert 256bit number to hex string and print it */
void fp256_print_hex(const BN_ULONG r[P256_LIMBS])
{
    unsigned char hex[65];
    fp256_get_hex(hex, r);
    hex[64] = '\0';
    printf("%s\n", hex);
}

/* convert byte array to 256bit number */
int fp256_set_bytes(BN_ULONG r[P256_LIMBS], unsigned char *bytes, int blen)
{
    int i;
    r[0] = 0ULL;
    r[1] = 0ULL;
    r[2] = 0ULL;
    r[3] = 0ULL;

    if (blen < 0 || blen > 32)
        return CRYPTO_ERR;

    i = 0;
    while (blen >= 8) {
        if (u8_to_u64(&r[i], bytes + blen - 8, 8, ORDER_BIG_ENDIAN) == CRYPTO_ERR)
            return CRYPTO_ERR;
        blen -= 8;
        i++;
    }

    if (blen > 0) {
        if (u8_to_u64(&r[i], bytes, blen, ORDER_BIG_ENDIAN) == CRYPTO_ERR)
            return CRYPTO_ERR;
    }

    return CRYPTO_OK;
}

/* convert 256bit number to byte array */
int fp256_get_bytes(unsigned char bytes[P256_LIMBS*8], const BN_ULONG a[P256_LIMBS])
{
    if (bytes == NULL)
        return CRYPTO_ERR;

    u64_to_u8(bytes,      a[3], ORDER_BIG_ENDIAN);
    u64_to_u8(bytes +  8, a[2], ORDER_BIG_ENDIAN);
    u64_to_u8(bytes + 16, a[1], ORDER_BIG_ENDIAN);
    u64_to_u8(bytes + 24, a[0], ORDER_BIG_ENDIAN);
    return CRYPTO_OK;
}

/* generate random 256bit number */
int fp256_rand(BN_ULONG r[P256_LIMBS])
{
    return RAND_buf((unsigned char*)r, BN_BYTES*P256_LIMBS);
}