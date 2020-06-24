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

#ifndef HEADER_CPUID_H
# define HEADER_CPUID_H

# include <secp256k1_x64/common.h>

# ifdef __cplusplus
extern "C" {
# endif

X64_EXPORT int runtime_has_sse2(void);

X64_EXPORT int runtime_has_sse3(void);

X64_EXPORT int runtime_has_ssse3(void);

X64_EXPORT int runtime_has_sse41(void);

X64_EXPORT int runtime_has_avx(void);

X64_EXPORT int runtime_has_avx2(void);

X64_EXPORT int runtime_has_bmi2(void);

X64_EXPORT int _runtime_get_cpu_features(void);
/* TODO : ... */
extern unsigned int cpu_info[4];


# ifdef __cplusplus
}
# endif

#endif