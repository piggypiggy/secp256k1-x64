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

#ifdef  __cplusplus
extern "C" {
#endif

/* initialization, it must be call once before calling any other functions */
X64_EXPORT int CRYPTO_init(void);
/* free all data initialized in CRYPTO_init */
X64_EXPORT int CRYPTO_deinit(void);

X64_EXPORT void* CRYPTO_malloc(size_t size);
X64_EXPORT void* CRYPTO_zalloc(size_t size);
X64_EXPORT void CRYPTO_free(void *ptr);
X64_EXPORT void CRYPTO_memzero(void *ptr, size_t size);
X64_EXPORT void CRYPTO_clear_free(void *ptr, size_t len);

X64_EXPORT int CRYPTO_crit_enter(void);
X64_EXPORT int CRYPTO_crit_leave(void);

#ifdef  __cplusplus
}
#endif