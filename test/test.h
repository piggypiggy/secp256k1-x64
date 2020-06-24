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
#include <secp256k1_x64/crypto.h>
#include <secp256k1_x64/fp256.h>
#include <secp256k1_x64/rand.h>
#include <secp256k1_x64/secp256k1.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "simple_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TEST_ARGS
{
    /* test result, 1 : success, 0 : fail */
    int ok;
    /* test times */
    int64_t N;
    /* number of threads */
    int64_t num_threads;
    /* description */
    char *desp;
}TEST_ARGS;

/* start n threads */
int test_start_n_thread(void (*func)(void *), TEST_ARGS *args);

void random_string(unsigned char *s, int len);

int random_number();

int get_test_args(int argc, char **argv, TEST_ARGS *args);
int set_test_args(TEST_ARGS *args, int64_t N, int64_t t, char *desp);


#ifdef __cplusplus
}
#endif