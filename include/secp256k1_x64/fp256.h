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

X64_EXPORT void fp256_copy(BN_ULONG r[P256_LIMBS], const BN_ULONG a[P256_LIMBS]);
X64_EXPORT void fp256_set_one(BN_ULONG r[P256_LIMBS]);
X64_EXPORT void fp256_set_word(BN_ULONG r[P256_LIMBS], const BN_ULONG w);
X64_EXPORT int fp256_is_zero(const BN_ULONG r[P256_LIMBS]);
X64_EXPORT int fp256_cmp(const BN_ULONG a[P256_LIMBS], const BN_ULONG b[P256_LIMBS]);
X64_EXPORT void fp256_neg(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
X64_EXPORT void fp256_double(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
X64_EXPORT void fp256_div_by_2(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
X64_EXPORT void fp256_triple(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG p[4]);
X64_EXPORT void fp256_add(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG b[4], const BN_ULONG p[4]);
X64_EXPORT void fp256_add_word(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG w, const BN_ULONG p[4]);
X64_EXPORT void fp256_sub(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG b[4], const BN_ULONG p[4]);
X64_EXPORT void fp256_sub_word(BN_ULONG r[4], const BN_ULONG a[4], const BN_ULONG w, const BN_ULONG p[4]);

X64_EXPORT int fp256_set_bytes(BN_ULONG r[P256_LIMBS], unsigned char *bytes, int blen);
X64_EXPORT int fp256_get_bytes(unsigned char bytes[P256_LIMBS*8], const BN_ULONG a[P256_LIMBS]);
X64_EXPORT int fp256_set_hex(BN_ULONG r[P256_LIMBS], unsigned char *hex, int hexlen);
X64_EXPORT int fp256_get_hex(unsigned char hex[P256_LIMBS*16], const BN_ULONG a[P256_LIMBS]);
X64_EXPORT void fp256_print_hex(const BN_ULONG a[P256_LIMBS]);

X64_EXPORT int fp256_rand(BN_ULONG r[P256_LIMBS]);
