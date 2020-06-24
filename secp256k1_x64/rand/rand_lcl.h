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

#ifndef HEADER_RAND_LCL_H
#define HEADER_RAND_LCL_H

#include <stddef.h>
#include <secp256k1_x64/rand.h>

#ifdef __cplusplus
extern "C" {
#endif

/* rand implementation */
typedef struct
{
    /* impl name */
    char *impl;
    /* fill buf with random bytes */
    int (*rand_buf) (unsigned char *buf, size_t len);
}RAND_IMPL;

/* chosen when library initialize */
extern const RAND_IMPL *rand_impl;

// const RAND_IMPL *SM3_RAND_IMPL();
const RAND_IMPL *SYS_RAND_IMPL();

/* choose rand implementation */
void runtime_choose_rand_implementation();

#ifdef __cplusplus
}
#endif

#endif