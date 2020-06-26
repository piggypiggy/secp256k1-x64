/*
 * Copyright 2014-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/******************************************************************************
 *                                                                            *
 * Copyright 2014 Intel Corporation                                           *
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
 ******************************************************************************
 *                                                                            *
 * Developers and authors:                                                    *
 * Shay Gueron (1, 2), and Vlad Krasnov (1)                                   *
 * (1) Intel Corporation, Israel Development Center                           *
 * (2) University of Haifa                                                    *
 * Reference:                                                                 *
 * S.Gueron and V.Krasnov, "Fast Prime Field Elliptic Curve Cryptography with *
 *                          256 Bit Primes"                                   *
 *                                                                            *
 ******************************************************************************/

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

// TODO : add point_mul for other point
#include <stdio.h>
#include <string.h>
#include <secp256k1_x64/crypto.h>
#include <secp256k1_x64/cpuid.h>
#include <secp256k1_x64/fp256.h>
#include <secp256k1_x64/secp256k1.h>

#if defined(__GNUC__)
# define ALIGN32        __attribute((aligned(32)))
#elif defined(_MSC_VER)
# define ALIGN32        __declspec(align(32))
#else
# define ALIGN32
#endif

#define ALIGNPTR(p,N)   ((unsigned char *)p+N-(size_t)p%N)

unsigned char *secp256k1_precomp_storage = NULL;
PRECOMP256_ROW *secp256k1_precomp = NULL;

static const BN_ULONG secp256k1_P[4] = 
{
    0xfffffffefffffc2fULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL
};

static const BN_ULONG secp256k1_N[4] = 
{
    0xbfd25e8cd0364141ULL, 0xbaaedce6af48a03bULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL
};

/* generator affine coordinate, in montgomery domain */
static const POINT256 secp256k1_G = 
{
    {0xd7362e5a487e2097ULL, 0x231e295329bc66dbULL, 0x979f48c033fd129cULL, 0x9981e643e9089f48ULL},
    {0xb15ea6d2d3dbabe2ULL, 0x8dfc5d5d1f1dc64dULL, 0x70b6b59aac19c136ULL, 0xcf3f851fd4a582d6ULL},
    {0x1000003d1ULL, 0ULL, 0ULL, 0ULL}
};

/* one converted into montgomery domain */
static const BN_ULONG ONE[P256_LIMBS] = {
    0x00000001000003d1ULL, 0ULL, 0ULL, 0ULL
};

int secp256k1_get_p(BN_ULONG r[P256_LIMBS])
{
    if (r == NULL)
       return CRYPTO_ERR;
    
    r[0] = secp256k1_P[0];
    r[1] = secp256k1_P[1];
    r[2] = secp256k1_P[2];
    r[3] = secp256k1_P[3];
    return CRYPTO_OK;
}

int secp256k1_get_order(BN_ULONG r[P256_LIMBS])
{
    if (r == NULL)
       return CRYPTO_ERR;
    
    r[0] = secp256k1_N[0];
    r[1] = secp256k1_N[1];
    r[2] = secp256k1_N[2];
    r[3] = secp256k1_N[3];
    return CRYPTO_OK;
}

int secp256k1_rand(BN_ULONG r[P256_LIMBS])
{
    if (fp256_rand(r) == CRYPTO_ERR)
        return CRYPTO_ERR;

    secp256k1_reduce(r, r);
    return CRYPTO_OK;
}

int secp256k1_point_copy(POINT256 *to, const POINT256 *from)
{
    if (to == NULL || from == NULL)
        return CRYPTO_ERR;
    fp256_copy(to->X, from->X);
    fp256_copy(to->Y, from->Y);
    fp256_copy(to->Z, from->Z);
    return CRYPTO_OK;
}

int secp256k1_get_generator(POINT256 *r)
{
    if (r == NULL)
        return CRYPTO_ERR;

    fp256_copy(r->X, secp256k1_G.X);
    fp256_copy(r->Y, secp256k1_G.Y);
    fp256_copy(r->Z, secp256k1_G.Z);
    return CRYPTO_OK;
}

int secp256k1_point_is_at_infinity(const POINT256 *a)
{
    return fp256_is_zero(a->Z);
}

/* ret 0  : a = b
 * ret -1 : a != b
 */
int secp256k1_point_cmp(const POINT256 *a, const POINT256 *b)
{
    BN_ULONG t1[P256_LIMBS], t2[P256_LIMBS];
    BN_ULONG u1[P256_LIMBS], u2[P256_LIMBS];

    if (secp256k1_point_is_at_infinity(a) == 1) {
        if (secp256k1_point_is_at_infinity(b) == 1)
            return 0;
        else
            return -1;
    }

    if (secp256k1_point_is_at_infinity(b) == 1)
        return -1;

    secp256k1_sqr_mont(u1, a->Z);
    secp256k1_sqr_mont(u2, b->Z);
    secp256k1_mul_mont(t1, a->X, u2);
    secp256k1_mul_mont(t2, b->X, u1);
    if (fp256_cmp(t1, t2) != 0)
        return -1;

    secp256k1_mul_mont(u1, u1, a->Z);
    secp256k1_mul_mont(u2, u2, b->Z);
    secp256k1_mul_mont(t1, a->Y, u2);
    secp256k1_mul_mont(t2, b->Y, u1);
    if (fp256_cmp(t1, t2) != 0)
        return -1;

    return 0;
}

/* ret 1 : point is on curve
 * ret 0 : point is not on curve
 */
int secp256k1_point_is_on_curve(const POINT256 *a)
{
    BN_ULONG X3[P256_LIMBS];
    BN_ULONG Y2[P256_LIMBS];
    BN_ULONG Z6[P256_LIMBS];

    if (a == NULL)
        return 0;

    if (secp256k1_point_is_at_infinity(a))
        return 1;

    /* X^3 */
    secp256k1_sqr_mont(X3, a->X);
    secp256k1_mul_mont(X3, X3, a->X);

    /* Y^2 */
    secp256k1_sqr_mont(Y2, a->Y);

    /* b * Z^6 */
    secp256k1_sqr_mont(Z6, a->Z);
    secp256k1_mul_mont(Z6, Z6, a->Z);
    secp256k1_sqr_mont(Z6, Z6);
    secp256k1_mul_word(Z6, Z6, 7); // b = 7

    /* X^3 + b*Z^6 */
    secp256k1_add(Z6, Z6, X3);

    /* cmp */
    return (fp256_cmp(Y2, Z6) == 0);
}

/* Recode window to a signed digit, see ecp_nistputil.c for details */
static inline unsigned int _booth_recode_w5(unsigned int in)
{
    unsigned int s, d;

    s = ~((in >> 5) - 1);
    d = (1 << 6) - in - 1;
    d = (d & s) | (in & ~s);
    d = (d >> 1) + (d & 1);

    return (d << 1) + (s & 1);
}

static inline unsigned int _booth_recode_w7(unsigned int in)
{
    unsigned int s, d;

    s = ~((in >> 7) - 1);
    d = (1 << 8) - in - 1;
    d = (d & s) | (in & ~s);
    d = (d >> 1) + (d & 1);

    return (d << 1) + (s & 1);
}

static inline BN_ULONG is_zero(BN_ULONG in)
{
    return in == 0UL;
}

/* in = aR mod p
 * r  = (a^-1)R mod p
 * TODO : implement lehmer exgcd in assembly ?
 */
void secp256k1_mod_inverse(BN_ULONG r[P256_LIMBS], const BN_ULONG in[P256_LIMBS])
{
    BN_ULONG a1[P256_LIMBS];
    BN_ULONG a2[P256_LIMBS];
    BN_ULONG a3[P256_LIMBS];
    BN_ULONG a4[P256_LIMBS];
    BN_ULONG a5[P256_LIMBS];
    BN_ULONG a6[P256_LIMBS];
    int i;

    a6[0] = in[0]; a6[1] = in[1]; a6[2] = in[2]; a6[3] = in[3];
    secp256k1_sqr_mont(a1, in);     // a1 = 2
    secp256k1_mul_mont(a2, a1, in); // a2 = 2^2 - 1
    secp256k1_sqr_mont(a3, a2);     
    secp256k1_sqr_mont(a3, a3);
    secp256k1_mul_mont(a3, a3, a2); // a3 = 2^4 - 1
    secp256k1_sqr_mont(a4, a3);
    secp256k1_mul_mont(r, a4, a3);  // 0x2d
    secp256k1_sqr_mont(a4, a4);
    secp256k1_sqr_mont(a4, a4);
    secp256k1_sqr_mont(a4, a4);
    secp256k1_mul_mont(a4, a4, a3); // a4 = 2^8 - 1
    secp256k1_sqr_mont(a5, a4);
    for (i = 0; i < 7; i++) {
        secp256k1_sqr_mont(a5, a5);
    }
    secp256k1_mul_mont(a5, a5, a4); // a5 = 2^16 - 1

    secp256k1_sqr_mont(a4, a4);
    secp256k1_mul_mont(a4, a4, a6);
    secp256k1_sqr_mont(a4, a4);     // a4 = 2^10 - 2

    secp256k1_sqr_mont(a6, a5);
    secp256k1_sqr_mont(a6, a6);
    secp256k1_sqr_mont(a6, a6);
    secp256k1_sqr_mont(a6, a6);
    secp256k1_mul_mont(a6, a6, a3); // 2^20 - 1
    secp256k1_sqr_mont(a6, a6);
    secp256k1_sqr_mont(a6, a6);
    secp256k1_mul_mont(a6, a6, a2); // 2^22 - 1
    for (i = 0; i < 10; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 0xfffffc00
    secp256k1_mul_mont(a6, a6, a4); // 2^32 - 2 
    secp256k1_mul_mont(a5, a6, a1); // 2^32
    for (i = 0; i < 32; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 0xffffffffe00000000
    secp256k1_mul_mont(a6, a6, a5); // 2^64 - 2^32 
    for (i = 0; i < 32; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 2^96 - 2^64
    for (i = 0; i < 32; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 2^128 - 2^96
    for (i = 0; i < 32; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 2^160 - 2^128
    for (i = 0; i < 32; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 2^192 - 2^160
    for (i = 0; i < 32; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 2^224 - 2^192
    for (i = 0; i < 32; i++) {
        secp256k1_sqr_mont(a6, a6);
    }
    secp256k1_mul_mont(r, r, a6);   // 2^256 - 2^224
}

int secp256k1_precompute_table_gen()
{
    /*
     * We precompute a table for a Booth encoded exponent (wNAF) based
     * computation. Each table holds 64 values for safe access, with an
     * implicit value of infinity at index zero. We use window of size 7, and
     * therefore require ceil(256/7) = 37 tables.
     */
    POINT256 P, T;
    int ret = CRYPTO_ERR;
    int i, j, k;

    /* precompute table has been initiated */
    if (secp256k1_precomp_storage != NULL)
        return CRYPTO_OK;

    if ((secp256k1_precomp_storage =
        malloc(37 * 64 * sizeof(POINT256_AFFINE) + 64)) == NULL) {
        goto end;
    }
    secp256k1_precomp = (void *)ALIGNPTR(secp256k1_precomp_storage, 64);

    /*
     * The zero entry is implicitly infinity, and we skip it, storing other
     * values with -1 offset.
     */
    secp256k1_point_copy(&T, &secp256k1_G);

    for (k = 0; k < 64; k++) {
        secp256k1_point_copy(&P, &T);
        for (j = 0; j < 37; j++) {
            POINT256_AFFINE temp;

            secp256k1_point_get_affine(temp.X, temp.Y, &P);
            secp256k1_to_mont(temp.X, temp.X);
            secp256k1_to_mont(temp.Y, temp.Y);
            secp256k1_scatter_w7(secp256k1_precomp[j], &temp, k);
            for (i = 0; i < 7; i++) {
                secp256k1_point_dbl(&P, &P);
            }
        }
        secp256k1_point_add(&T, &T, &secp256k1_G);
    }

    ret = CRYPTO_OK;
end:
    return ret;
}

/* r = scalar*G */
int secp256k1_scalar_mul_gen(POINT256 *r, BN_ULONG scalar[P256_LIMBS])
{
    int i = 0;
    int ret = CRYPTO_ERR;
    unsigned char p_str[33] = { 0 };
    BN_ULONG s[P256_LIMBS];
    unsigned int idx = 0;
    const unsigned int window_size = 7;
    const unsigned int mask = (1 << (window_size + 1)) - 1;
    unsigned int wvalue;
    ALIGN32 union {
        POINT256 p;
        POINT256_AFFINE a;
    } t, p;

    /* s = scalar mod p */
    secp256k1_reduce(s, scalar);

    for (i = 0; i < 32; i += 8) {
        BN_ULONG d = s[i / 8];

        p_str[i + 0] = (unsigned char)d;
        p_str[i + 1] = (unsigned char)(d >> 8);
        p_str[i + 2] = (unsigned char)(d >> 16);
        p_str[i + 3] = (unsigned char)(d >>= 24);
        d >>= 8;
        p_str[i + 4] = (unsigned char)d;
        p_str[i + 5] = (unsigned char)(d >> 8);
        p_str[i + 6] = (unsigned char)(d >> 16);
        p_str[i + 7] = (unsigned char)(d >> 24);
    }
    p_str[32] = 0;

    {
        BN_ULONG infty;

        /* First window */
        wvalue = (p_str[0] << 1) & mask;
        idx += window_size;

        wvalue = _booth_recode_w7(wvalue);

        if (wvalue > 1)
            memcpy(&p.a, secp256k1_precomp[0] + (wvalue >> 1) - 1, 64);
        else
            memset(&p.a, 0, 64);
        
        if (wvalue & 1)  
            secp256k1_neg(p.p.Y, p.p.Y);

        
        infty = (p.p.X[0] | p.p.X[1] | p.p.X[2] | p.p.X[3] |
                    p.p.Y[0] | p.p.Y[1] | p.p.Y[2] | p.p.Y[3]);

        infty = 0 - is_zero(infty);
        infty = ~infty;

        p.p.Z[0] = ONE[0] & infty;
        p.p.Z[1] = ONE[1] & infty;
        p.p.Z[2] = ONE[2] & infty;
        p.p.Z[3] = ONE[3] & infty;

        for (i = 1; i < 37; i++) {
            unsigned int off = (idx - 1) / 8;
            wvalue = p_str[off] | p_str[off + 1] << 8;
            wvalue = (wvalue >> ((idx - 1) % 8)) & mask;
            idx += window_size;

            wvalue = _booth_recode_w7(wvalue);

            if (wvalue > 1)
                memcpy(&t.a, secp256k1_precomp[i] + (wvalue >> 1) - 1, 64);
            else
                memset(&t.a, 0, 64);

            if (wvalue & 1)  
                secp256k1_neg(t.p.Y, t.p.Y);

            secp256k1_point_add_affine(&p.p, &p.p, &t.a);
        }
    }
    *r = p.p;

    ret = CRYPTO_OK;
    return ret;
}

/* r = scalar * point */
int secp256k1_scalar_mul_point(POINT256 *r, BN_ULONG scalar[P256_LIMBS], POINT256 *point)
{
    int i;
    int ret = CRYPTO_ERR;
    unsigned int idx;
    unsigned char p_str[33] = { 0 };
    const unsigned int window_size = 5;
    const unsigned int mask = (1 << (window_size + 1)) - 1;
    unsigned int wvalue;
    BN_ULONG s[P256_LIMBS];
    POINT256 *temp;
    POINT256 *table = NULL;
    unsigned char table_storage[2080];

    table = (void*)ALIGNPTR(table_storage, 64);
    temp = table + 16;

    POINT256 *row = table;

    /* s = scalar mod p */
    secp256k1_reduce(s, scalar);

    for (i = 0; i < 32; i += 8) {
        BN_ULONG d = s[i / 8];

        p_str[i + 0] = (unsigned char)d;
        p_str[i + 1] = (unsigned char)(d >> 8);
        p_str[i + 2] = (unsigned char)(d >> 16);
        p_str[i + 3] = (unsigned char)(d >>= 24);
        d >>= 8;
        p_str[i + 4] = (unsigned char)d;
        p_str[i + 5] = (unsigned char)(d >> 8);
        p_str[i + 6] = (unsigned char)(d >> 16);
        p_str[i + 7] = (unsigned char)(d >> 24);
    }
    p_str[32] = 0;
    
    if (secp256k1_point_copy(temp, point) == CRYPTO_ERR) 
        goto end;

    /*
        * row[0] is implicitly (0,0,0) (the point at infinity), therefore it
        * is not stored. All other values are actually stored with an offset
        * of -1 in table.
        */

    secp256k1_scatter_w5  (row, &temp[0], 1);
    secp256k1_point_dbl(&temp[1], &temp[0]);              /*1+1=2  */
    secp256k1_scatter_w5  (row, &temp[1], 2);
    secp256k1_point_add   (&temp[2], &temp[1], &temp[0]);    /*2+1=3  */
    secp256k1_scatter_w5  (row, &temp[2], 3);
    secp256k1_point_dbl(&temp[1], &temp[1]);              /*2*2=4  */
    secp256k1_scatter_w5  (row, &temp[1], 4);
    secp256k1_point_dbl(&temp[2], &temp[2]);              /*2*3=6  */
    secp256k1_scatter_w5  (row, &temp[2], 6);
    secp256k1_point_add   (&temp[3], &temp[1], &temp[0]);    /*4+1=5  */
    secp256k1_scatter_w5  (row, &temp[3], 5);
    secp256k1_point_add   (&temp[4], &temp[2], &temp[0]);    /*6+1=7  */
    secp256k1_scatter_w5  (row, &temp[4], 7);
    secp256k1_point_dbl(&temp[1], &temp[1]);              /*2*4=8  */
    secp256k1_scatter_w5  (row, &temp[1], 8);
    secp256k1_point_dbl(&temp[2], &temp[2]);              /*2*6=12 */
    secp256k1_scatter_w5  (row, &temp[2], 12);
    secp256k1_point_dbl(&temp[3], &temp[3]);              /*2*5=10 */
    secp256k1_scatter_w5  (row, &temp[3], 10);
    secp256k1_point_dbl(&temp[4], &temp[4]);              /*2*7=14 */
    secp256k1_scatter_w5  (row, &temp[4], 14);
    secp256k1_point_add   (&temp[2], &temp[2], &temp[0]);    /*12+1=13*/
    secp256k1_scatter_w5  (row, &temp[2], 13);
    secp256k1_point_add   (&temp[3], &temp[3], &temp[0]);    /*10+1=11*/
    secp256k1_scatter_w5  (row, &temp[3], 11);
    secp256k1_point_add   (&temp[4], &temp[4], &temp[0]);    /*14+1=15*/
    secp256k1_scatter_w5  (row, &temp[4], 15);
    secp256k1_point_add   (&temp[2], &temp[1], &temp[0]);    /*8+1=9  */
    secp256k1_scatter_w5  (row, &temp[2], 9);
    secp256k1_point_dbl(&temp[1], &temp[1]);              /*2*8=16 */
    secp256k1_scatter_w5  (row, &temp[1], 16);

    idx = 255;
    wvalue = p_str[(idx - 1) / 8];
    wvalue = (wvalue >> ((idx - 1) % 8)) & mask;

    /*
     * We gather to temp[0], because we know it's position relative
     * to table
     */
    if (wvalue > 1)
        memcpy(&temp[0], table + (_booth_recode_w5(wvalue) >> 1) - 1, 96);
    else
        memset(&temp[0], 0, 96);
    memcpy(r, &temp[0], sizeof(temp[0]));
    secp256k1_point_dbl(r, r);
    secp256k1_point_dbl(r, r);
    secp256k1_point_dbl(r, r);
    secp256k1_point_dbl(r, r);
    secp256k1_point_dbl(r, r);
    idx -= window_size;

    while (idx >= 5) {
        unsigned int off = (idx - 1) / 8;

        wvalue = p_str[off] | p_str[off + 1] << 8;
        wvalue = (wvalue >> ((idx - 1) % 8)) & mask;

        wvalue = _booth_recode_w5(wvalue);

        if (wvalue > 1)
            memcpy(&temp[0], table + (wvalue >> 1) - 1, 96);
        else
            memset(&temp[0], 0, 96);

        if (wvalue & 1)  
            secp256k1_neg(temp[0].Y, temp[0].Y);

        secp256k1_point_add(r, r, &temp[0]);

        idx -= window_size;

        secp256k1_point_dbl(r, r);
        secp256k1_point_dbl(r, r);
        secp256k1_point_dbl(r, r);
        secp256k1_point_dbl(r, r);
        secp256k1_point_dbl(r, r);
    }

    /* Final window */
    wvalue = p_str[0];
    wvalue = (wvalue << 1) & mask;
    wvalue = _booth_recode_w5(wvalue);

    if (wvalue > 1)
        memcpy(&temp[0], table + (wvalue >> 1) - 1, 96);
    else
        memset(&temp[0], 0, 96);

    if (wvalue & 1)  
        secp256k1_neg(temp[0].Y, temp[0].Y);

    secp256k1_point_add(r, r, &temp[0]);

    ret = CRYPTO_OK;
end:
    return ret;
}

int secp256k1_point_get_affine(BN_ULONG x[P256_LIMBS], BN_ULONG y[P256_LIMBS], const POINT256 *point)
{
    BN_ULONG z_inv2[P256_LIMBS];
    BN_ULONG z_inv3[P256_LIMBS];
    BN_ULONG x_aff[P256_LIMBS];
    BN_ULONG y_aff[P256_LIMBS];

    if (point == NULL || x == NULL || y == NULL)
        return CRYPTO_ERR;

    if (secp256k1_point_is_at_infinity(point))
        return CRYPTO_ERR;

    secp256k1_mod_inverse(z_inv3, point->Z);
    secp256k1_sqr_mont(z_inv2, z_inv3);
    secp256k1_mul_mont(x_aff, z_inv2, point->X);

    if (x != NULL)
        secp256k1_from_mont(x, x_aff);

    if (y != NULL) {
        secp256k1_mul_mont(z_inv3, z_inv3, z_inv2);
        secp256k1_mul_mont(y_aff, z_inv3, point->Y);
        secp256k1_from_mont(y, y_aff);
    }

    return CRYPTO_OK;
}

int secp256k1_point_set_affine(POINT256 *point, const BN_ULONG x[P256_LIMBS], const BN_ULONG y[P256_LIMBS])
{
    if (point == NULL || x == NULL || y == NULL)
        return CRYPTO_ERR;

    secp256k1_to_mont(point->X, x);
    secp256k1_to_mont(point->Y, y);
    fp256_copy(point->Z, ONE);
    if (secp256k1_point_is_on_curve(point) == 0)
        return CRYPTO_ERR;

    return CRYPTO_OK;
}

void secp256k1_precompute_table_free()
{
    CRYPTO_free(secp256k1_precomp_storage);
}

void secp256k1_point_print(POINT256 *point)
{
    if (point == NULL)
        return;

    printf("X : ");
    fp256_print_hex(point->X);
    printf("Y : ");
    fp256_print_hex(point->Y);
    printf("Z : ");
    fp256_print_hex(point->Z);
}