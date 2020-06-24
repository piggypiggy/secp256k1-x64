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

#ifdef HAVE_PTHREAD

int test_start_n_thread(void (*func)(void *), TEST_ARGS *args)
{
    int ret = CRYPTO_OK;
    int64_t i;
    int64_t num_threads = args->num_threads;
    pthread_t *t;

    t = (pthread_t*)malloc(sizeof(pthread_t) * args->num_threads);
    TEST_ARGS *test_args = calloc(1, num_threads*sizeof(TEST_ARGS));

    /* copy arguments */
    for (i = 0; i < num_threads; i++)
        memcpy(&test_args[i], args, sizeof(TEST_ARGS));

    /* start threads */
    for (i = 0; i < num_threads; i++)
        if (pthread_create(&t[i], NULL, (void*)func, (void*)&test_args[i])) {
            ret = CRYPTO_ERR;
            goto end;
        }

    for (i = 0; i < num_threads; i++) {
        pthread_join(t[i], NULL);
        if (test_args[i].ok == CRYPTO_ERR)
            ret = CRYPTO_ERR;
    }

    ret = CRYPTO_OK;
end:
    free(t);
    return ret;
}

#elif defined HAVE_WIN32_THREAD

int test_start_n_thread(void(*func)(void *), TEST_ARGS *args)
{
    int ret = CRYPTO_OK;
    int64_t i;
    int64_t num_threads = args->num_threads;
    HANDLE *thread_handle = (HANDLE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, num_threads * sizeof(HANDLE));
    TEST_ARGS *test_args = calloc(1, num_threads * sizeof(TEST_ARGS));

    for (i = 0; i < num_threads; i++) {
        memcpy(&test_args[i], args, sizeof(TEST_ARGS));
        thread_handle[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)&test_args[i], 0, NULL);
    }

    for (i = 0; i < num_threads; i++) {
        WaitForSingleObject(thread_handle[i], INFINITE);
        CloseHandle(thread_handle[i]);
        if (test_args[i].ok == CRYPTO_ERR)
            ret = CRYPTO_ERR;
    }
    HeapFree(GetProcessHeap(), 0, thread_handle);

    return ret;
}

#else

int test_start_n_thread(void (*func)(void *), void *arg, int n)
{
    printf("multithread not supported \n");
    return CRYPTO_ERR;
}

#endif