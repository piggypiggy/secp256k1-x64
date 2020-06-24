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

#include "test.h"

void random_string(unsigned char *s, int len)
{
    static int en = 0;
    srand((unsigned)time(NULL) + en);
    while (len--)
        s[len] = rand() % 256;
    en++;
}

int random_number()
{
    static int en = 0;
    srand((unsigned)time(NULL) + en);
    en++;
    return rand();
}

int get_test_args(int argc, char **argv, TEST_ARGS *args)
{
    int ret = CRYPTO_ERR;
    args->num_threads = 0;
    args->N = 0;

    for (int i = 1; i < argc; i++) {
        if (memcmp(argv[i], "THREADS=", 8) == 0 || memcmp(argv[i], "threads=", 8) == 0) {
            args->num_threads = (int64_t)atoi(argv[i] + 8);
            continue;
        }

        if (memcmp(argv[i], "t=", 2) == 0 || memcmp(argv[i], "T=", 2) == 0) {
            args->num_threads = (int64_t)atoi(argv[i] + 2);
            continue;
        }

        if (memcmp(argv[i], "n=", 2) == 0 || memcmp(argv[i], "N=", 2) == 0) {
            args->N = (int64_t)atoi(argv[i] + 2);
            continue;
        }
    }

    args->ok = 0;
    args->num_threads = args->num_threads >= 0 ? args->num_threads : 4;
    args->N = args->N > 0 ? args->N : 12345; 

    return ret;
}

int set_test_args(TEST_ARGS *args, int64_t N, int64_t t, char *desp)
{
    if (args == NULL)
        return CRYPTO_ERR;

    if (N > 0)
        args->N = N;

    if (t >= 0)
        args->num_threads = t;

    args->desp = desp;
    return CRYPTO_OK;
}

