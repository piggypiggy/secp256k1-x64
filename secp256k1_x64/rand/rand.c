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

#include <secp256k1_x64/rand.h>
#include "rand_lcl.h"

const RAND_IMPL *rand_impl;

void runtime_choose_rand_implementation()
{
    rand_impl = SYS_RAND_IMPL();
}

int RAND_buf(unsigned char *buf, int len)
{
    if (rand_impl == NULL || rand_impl->rand_buf == NULL || buf == NULL || len <= 0)
        return CRYPTO_ERR;

    return rand_impl->rand_buf(buf, len);
}