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

#include "../test/test.h"
#include "speed_lcl.h"

static void secp256k1_point_add_speed(void *p)
{
    int64_t N;
    POINT256 r, a, b;

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_get_generator(&a);
    secp256k1_get_generator(&b);
    secp256k1_point_dbl(&a, &a);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_point_add(&r, &a, &b);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_point_add : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_point_add_affine_speed(void *p)
{
    int64_t N;
    POINT256 r, a, b;
    POINT256_AFFINE c;

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_get_generator(&a);
    secp256k1_get_generator(&b);
    secp256k1_point_dbl(&a, &a);
    secp256k1_point_get_affine(c.X, c.Y, &b);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_point_add_affine(&r, &a, &c);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_point_add_affine : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_point_dbl_speed(void *p)
{
    int64_t N;
    POINT256 r, a;

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_get_generator(&a);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_point_dbl(&r, &a);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_point_dbl : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_scalar_mul_gen_speed(void *p)
{
    int64_t N;
    BN_ULONG scalar[P256_LIMBS];
    POINT256 r;

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_rand(scalar);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_scalar_mul_gen(&r, scalar);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_scalar_mul_gen : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_scalar_mul_point_speed(void *p)
{
    int64_t N;
    BN_ULONG scalar[P256_LIMBS];
    POINT256 r, point;

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_rand(scalar);
    secp256k1_get_generator(&point);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_scalar_mul_point(&r, scalar, &point);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_scalar_mul_point : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_point_get_affine_speed(void *p)
{
    int64_t N;
    BN_ULONG x[P256_LIMBS], y[P256_LIMBS];
    POINT256 r;

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_get_generator(&r);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_point_get_affine(x, y, &r);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_point_get_affine : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_mod_inverse_speed(void *p)
{
    int64_t N;
    BN_ULONG r[P256_LIMBS], a[P256_LIMBS];

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_rand(a);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_mod_inverse(r, a);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_mod_inverse : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_mul_mont_speed(void *p)
{
    int64_t N;
    BN_ULONG r[P256_LIMBS], x[P256_LIMBS], y[P256_LIMBS];

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    secp256k1_rand(x);
    secp256k1_rand(y);

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_mul_mont(r, x, y);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_mul_mont : %lu  op/s\n\n", N*1000000/total_time);
}

static void secp256k1_sqr_mont_speed(void *p)
{
    int64_t N;
    BN_ULONG r[P256_LIMBS], x[P256_LIMBS];

    TEST_ARGS *args = (TEST_ARGS*)p;
    N = args->N;

    /* setup */
    RAND_buf((unsigned char*)x, sizeof(x));

    BENCH_VARS;

    COUNTER_START();
    TIMER_START();
    
    for (int64_t i = 0; i < N; i++)
        secp256k1_sqr_mont(r, x);
    
    COUNTER_STOP();
    TIMER_STOP();

    printf("average cycles per op : %lu \n", (TICKS()/N));
    printf("secp256k1_sqr_mont : %lu  op/s\n\n", N*1000000/total_time);
}

void run_speed(void(*func)(void *), TEST_ARGS *args)
{
    printf("=========== %s ===========\n", args->desp);
    /* single thread */
    printf("----- SINGLE THREAD N = %lu -----\n\n", args->N);
    func((void*)args);

    /* multi thread */
    if (args->num_threads > 0) {
        printf("----- MULTI THREAD, threads = %lu -----\n\n", args->num_threads);
        test_start_n_thread(func, (void*)args);
    }
}

int main(int argc, char **argv)
{
    TEST_ARGS args;

    CRYPTO_init();

    // get_test_args(argc, argv, &args);
    args.ok = CRYPTO_OK;

    set_test_args(&args, 20000, 0, "secp256k1 point add affine");
    run_speed(secp256k1_point_add_affine_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 point add");
    run_speed(secp256k1_point_add_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 point dbl");
    run_speed(secp256k1_point_dbl_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 scalar mul gen");
    run_speed(secp256k1_scalar_mul_gen_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 scalar mul point");
    run_speed(secp256k1_scalar_mul_point_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 point get affine");
    run_speed(secp256k1_point_get_affine_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 mod inverse");
    run_speed(secp256k1_mod_inverse_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 mul mont");
    run_speed(secp256k1_mul_mont_speed, &args);

    set_test_args(&args, 20000, 0, "secp256k1 sqr mont");
    run_speed(secp256k1_sqr_mont_speed, &args);

    CRYPTO_deinit();
    return 0;
}