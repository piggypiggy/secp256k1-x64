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

#include <secp256k1_x64/secp256k1.h>

/* set r = a */
X64_EXPORT void fp256_copy(BN_ULONG r[P256_LIMBS], const BN_ULONG a[P256_LIMBS]);
/* set r = 1 */
X64_EXPORT void fp256_set_one(BN_ULONG r[P256_LIMBS]);
/* set r = w */
X64_EXPORT void fp256_set_word(BN_ULONG r[P256_LIMBS], const BN_ULONG w);
/* 1 : r = 0
 * 0 : r != 0
 */ 
X64_EXPORT int fp256_is_zero(const BN_ULONG r[P256_LIMBS]);
/* compare two big integers
 * 1  : a > b
 * 0  : a = b
 * -1 : a < b
 */ 
X64_EXPORT int fp256_cmp(const BN_ULONG a[P256_LIMBS], const BN_ULONG b[P256_LIMBS]);
/* r = -a mod p */
X64_EXPORT void fp256_neg(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
/* r = 2a mod p */
X64_EXPORT void fp256_double(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
/* r = a/2 mod p */
X64_EXPORT void fp256_div_by_2(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
/* r = 3a mod p */
X64_EXPORT void fp256_triple(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
/* r = a + b mod p */
X64_EXPORT void fp256_add(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG b[4], const BN_ULONG p[4]);
/* r = a + w mod p, w is a 64bit integer */
X64_EXPORT void fp256_add_word(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG w, const BN_ULONG p[4]);
/* r = a - b mod p */
X64_EXPORT void fp256_sub(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG b[4], const BN_ULONG p[4]);
/* r = a - w mod p, w is a 64bit integer */
X64_EXPORT void fp256_sub_word(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG w, const BN_ULONG p[4]);
/* convert byte array to big integer */ 
X64_EXPORT int fp256_set_bytes(BN_ULONG r[P256_LIMBS], unsigned char *bytes, int blen);
/* convert big integer to byte array */
X64_EXPORT int fp256_get_bytes(unsigned char bytes[P256_LIMBS*8], const BN_ULONG a[P256_LIMBS]);
/* convert hex string to big integer */
X64_EXPORT int fp256_set_hex(BN_ULONG r[P256_LIMBS], unsigned char *hex, int hexlen);
/* convert big integer to hex string */
X64_EXPORT int fp256_get_hex(unsigned char hex[P256_LIMBS*16], const BN_ULONG a[P256_LIMBS]);
/* convert big integer to hex string, then print it */
X64_EXPORT void fp256_print_hex(const BN_ULONG a[P256_LIMBS]);
/* random integer from interval [0, 2^256 - 1] */
X64_EXPORT int fp256_rand(BN_ULONG r[P256_LIMBS]);
