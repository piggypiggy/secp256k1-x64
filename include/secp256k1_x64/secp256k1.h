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

#include <secp256k1_x64/common.h>

typedef struct {
    BN_ULONG X[P256_LIMBS];
    BN_ULONG Y[P256_LIMBS];
    BN_ULONG Z[P256_LIMBS];
} POINT256;

typedef struct {
    BN_ULONG X[P256_LIMBS];
    BN_ULONG Y[P256_LIMBS];
} POINT256_AFFINE;

typedef POINT256_AFFINE PRECOMP256_ROW[64];

X64_EXPORT int secp256k1_precompute_table_gen();
X64_EXPORT void secp256k1_precompute_table_free();

X64_EXPORT int secp256k1_get_p(BN_ULONG r[P256_LIMBS]);
X64_EXPORT int secp256k1_get_order(BN_ULONG r[P256_LIMBS]);
X64_EXPORT int secp256k1_get_generator(POINT256 *r);
X64_EXPORT int secp256k1_point_copy(POINT256 *to, const POINT256 *from);
/* generate random integer from interval [0, P-1] */
X64_EXPORT int secp256k1_rand(BN_ULONG r[P256_LIMBS]);

/* Functions implemented in assembly */
/*
 * Most of below mentioned functions *preserve* the property of inputs
 * being fully reduced, i.e. being in [0, modulus) range. Simply put if
 * inputs are fully reduced, then output is too. Note that reverse is
 * not true, in sense that given partially reduced inputs output can be
 * either, not unlikely reduced. And "most" in first sentence refers to
 * the fact that given the calculations flow one can tolerate that
 * addition, 1st function below, produces partially reduced result *if*
 * multiplications by 2 and 3, which customarily use addition, fully
 * reduce it. This effectively gives two options: a) addition produces
 * fully reduced result [as long as inputs are, just like remaining
 * functions]; b) addition is allowed to produce partially reduced
 * result, but multiplications by 2 and 3 perform additional reduction
 * step. Choice between the two can be platform-specific, but it was a)
 * in all cases so far...
 */
/* Modular add: res = a+b mod P   */
X64_EXPORT void secp256k1_add(BN_ULONG res[P256_LIMBS],
                     const BN_ULONG a[P256_LIMBS],
                     const BN_ULONG b[P256_LIMBS]);
/* Modular mul by 2: res = 2*a mod P */
X64_EXPORT void secp256k1_mul_by_2(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);
/* Modular mul by 3: res = 3*a mod P */
X64_EXPORT void secp256k1_mul_by_3(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);

/* Modular div by 2: res = a/2 mod P */
X64_EXPORT void secp256k1_div_by_2(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);
/* Modular sub: res = a-b mod P   */
X64_EXPORT void secp256k1_sub(BN_ULONG res[P256_LIMBS],
                     const BN_ULONG a[P256_LIMBS],
                     const BN_ULONG b[P256_LIMBS]);
/* Modular neg: res = -a mod P    */
X64_EXPORT void secp256k1_neg(BN_ULONG res[P256_LIMBS], const BN_ULONG a[P256_LIMBS]);
/* res = a mod P */
X64_EXPORT void secp256k1_reduce(BN_ULONG res[4], BN_ULONG a[P256_LIMBS]);
/* res = a*w mod P */
X64_EXPORT void secp256k1_mul_word(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS],
                          const BN_ULONG w);
/* Montgomery mul: res = a*b*2^-256 mod P */
X64_EXPORT void secp256k1_mul_mont(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS],
                          const BN_ULONG b[P256_LIMBS]);
/* Montgomery sqr: res = a*a*2^-256 mod P */
X64_EXPORT void secp256k1_sqr_mont(BN_ULONG res[P256_LIMBS],
                          const BN_ULONG a[P256_LIMBS]);
/* Convert a number from Montgomery domain, by multiplying with 1 */
X64_EXPORT void secp256k1_from_mont(BN_ULONG res[P256_LIMBS],
                           const BN_ULONG in[P256_LIMBS]);
/* Convert a number to Montgomery domain, by multiplying with 2^512 mod P*/
X64_EXPORT void secp256k1_to_mont(BN_ULONG res[P256_LIMBS],
                         const BN_ULONG in[P256_LIMBS]);
/* Functions that perform constant time access to the precomputed tables */
X64_EXPORT void secp256k1_scatter_w5(POINT256 *val,
                            const POINT256 *in_t, int idx);
X64_EXPORT void secp256k1_scatter_w7(POINT256_AFFINE *val,
                            const POINT256_AFFINE *in_t, int idx);

/* compare two points, 0 : a = b, -1 : a != b */
X64_EXPORT int secp256k1_point_cmp(const POINT256 *a, const POINT256 *b);

X64_EXPORT void secp256k1_point_dbl(POINT256 *r, const POINT256 *a);
X64_EXPORT void secp256k1_point_add(POINT256 *r, const POINT256 *a, const POINT256 *b);
X64_EXPORT void secp256k1_point_add_affine(POINT256 *r,
                                  const POINT256 *a,
                                  const POINT256_AFFINE *b);

/* r = scalar * generator */
X64_EXPORT int secp256k1_scalar_mul_gen(POINT256 *r, BN_ULONG scalar[P256_LIMBS]);
/* r = scalar * point */
X64_EXPORT int secp256k1_scalar_mul_point(POINT256 *r, BN_ULONG scalar[P256_LIMBS], POINT256 *point);
/* convert jacobian coordinate(mont) to affine coordinate */
X64_EXPORT int secp256k1_point_get_affine(BN_ULONG x[P256_LIMBS], BN_ULONG y[P256_LIMBS], const POINT256 *point);
/* convert affine coordinate to jacobian coordinate(mont) */
X64_EXPORT int secp256k1_point_set_affine(POINT256 *point, const BN_ULONG x[P256_LIMBS], const BN_ULONG y[P256_LIMBS]);
/* field inversion
 * in = aR mod p
 * r  = (a^-1)R mod p
 * (R = 2^256 mod p)
 */
X64_EXPORT void secp256k1_mod_inverse(BN_ULONG r[P256_LIMBS], const BN_ULONG in[P256_LIMBS]);
/* 1 : point is on curve, 
 * 0 : point is not on curve 
 */
X64_EXPORT int secp256k1_point_is_on_curve(const POINT256 *a);
/* print jacobian coordinate in hex */
X64_EXPORT void secp256k1_point_print(POINT256 *point);


