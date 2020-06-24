# secp256k1-x64
Very efficient (NOT SECURE) implementation of arithmetic on curve secp256k1 on x86_64

This library aims to provide the most efficient implementation of secp256k1 curve arithmetic. To achieve this:
* I borrow the idea from [GmSSL](https://github.com/guanzhi/GmSSL/blob/master/crypto/ec/asm/ecp_sm2z256-x86_64.pl)'s SM2 assembly code and make all code path work on secp256k1, inluding sse, avx and bmi2.

* Remove many constant time code.

* Implement a simple 256bit big number library in assembly to help boost speed(Although it doesn't help currently, I hope it would in the future :) ).

# Implementation
* No heap allocation.
* Use 4×64bit to represent 256bit number.
* Montmomery multiplication for secp256k1 modular p.
* Efficient field inversion(257 sqr + 19 mul).
* Precomputed table for generator, only 37 point addition is needed to do a point multiplication.
* Replace gather function with memory copy, memory access is **NOT** uniform and **NOT** constant-time.
* Some branch-less code become **Not** branch-less(e.g. conditional move in point_add).

# Build
CMake is used to build this library, see BUILD_UNIX.txt and BUILD_WINDOWS.txt.

# Security
As stated above, I sacrifice security for higher efficiency, ~~so DO NOT use it in serious situation~~ it's suitable for "daily" use.

# Benchmark
### Intel Core i7-7700k 3.4GHz(Skylake), Ubuntu 18.04 LTS, gcc-7.5.0
arithmetic              |      cycles / op      |      op / s      |
------------------------|-----------------------|------------------|
montgomery square       |            43         |    78125000      |
montgomery mul          |            49         |    68649885      |
point add affine        |           557         |     6121199      |
point add               |           791         |     4305396      |
point double            |           424         |     8027829      |
scalar mul generator    |         21527         |      158310      |
scalar mul point        |        148610         |       22932      |
jacobian to affine      |         14621         |      233080      |
modular inverse         |         14785         |      230505      |

Note: scalar mul generator is about 6x faster than [secp256k1](https://github.com/bitcoin-core/secp256k1)'s single-scalar multiplication with window size 15. When secp256k1 uses multi-scalar muliplication with batch size 32768, it can only reach the same speed as single-scalar multiplication of this library, i.e. scalar mul generator 158310 op/s.

### Intel Core i3-2328M 2.2GHz(Sandy Bridge), Ubuntu 16.04 LTS, gcc-7.4.0
arithmetic              |      cycles / op      |      op / s      |
------------------------|-----------------------|------------------|
montgomery square       |            90         |    24232623      |
montgomery mul          |            93         |    23492560      |
point add affine        |          1269         |     1728007      |
point add               |          1670         |     1313370      |
point double            |           882         |     2487355      |
scalar mul generator    |         45214         |       48533      |
scalar mul point        |        304994         |        7195      |
jacobian to affine      |         27813         |       80730      |
modular inverse         |         26668         |       82291      |

# License
Apache 2.0
