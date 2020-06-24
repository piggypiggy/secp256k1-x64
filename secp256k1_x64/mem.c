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

#include <string.h>
#include <secp256k1_x64/crypto.h>

void* CRYPTO_zalloc(size_t size)
{
    void *ret = malloc(size);
    if (ret == NULL)
        return NULL;
    memset(ret, 0, size);
    return ret;
}

void* CRYPTO_malloc(size_t size)
{
    return malloc(size);
}

void CRYPTO_free(void *ptr)
{
    free(ptr);
}

void CRYPTO_clear_free(void *ptr, size_t size)
{
    memset(ptr, 0, size);
    free(ptr);
}