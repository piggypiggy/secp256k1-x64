###############################################################################
#                                                                             #
#  Copyright 2020 Meng-Shan Jiang                                             #
#                                                                             #
#  Licensed under the Apache License, Version 2.0 (the "License");            #
#  you may not use this file except in compliance with the License.           #
#  You may obtain a copy of the License at                                    #
#                                                                             #
#     http://www.apache.org/licenses/LICENSE-2.0                              #
#                                                                             #
#  Unless required by applicable law or agreed to in writing, software        #
#  distributed under the License is distributed on an "AS IS" BASIS,          #
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
#  See the License for the specific language governing permissions and        #
#  limitations under the License.                                             #
#                                                                             #
###############################################################################

import math

# modular
N = 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f
# 2^(-256) mod N
ri = 0xc9bd1905155383999c46c2c295f2b761bcb223fedc24a059d838091d0868192a

def point_add(x1, y1, z1, x2, y2, z2):
    # z22 = (z2 * z2 * ri) % N 
    # z12 = (z1 * z1 * ri) % N 
    # z23 = (z22 * z2 * ri) % N
    # z13 = (z12 * z1 * ri) % N
    # s1 = (y1 * z23 * ri) % N
    # s2 = (y2 * z13 * ri) % N
    # r = (s2 - s1) % N
    # u1 = (x1 * z22 * ri) % N
    # u2 = (x2 * z12 * ri) % N
    # h = (u2 - u1) % N
    # r2 = (r * r * ri) % N
    # z3 = (h * z1 * ri) % N
    # h2 = (h * h * ri) % N
    # z3 = (z3 * z2 * ri) % N
    # h3 = (h2 * h * ri) % N
    # u2 = (u1 * h2 * ri) % N
    # t = (u2 + u2) % N
    # x3 = (r2 - t) % N
    # x3 = (x3 - h3) % N
    # y3 = (u2 - x3) % N
    # s2 = (s1 * h3 * ri) % N
    # y3 = (r * y3 * ri) % N
    # y3 = (y3 - s2) % N

    u1 = (x1 * z2 * z2 * ri * ri) % N
    u2 = (x2 * z1 * z1 * ri * ri) % N
    s1 = (y1 * z2 * z2 * z2 * ri * ri * ri) % N
    s2 = (y2 * z1 * z1 * z1 * ri * ri * ri) % N
    h = (u2 - u1) % N
    r = (s2 - s1) % N
    r2 = (r * r * ri) % N
    h2 = (h * h * ri) % N
    h3 = (h2 * h * ri) % N
    t = (2 * u1 * h2 * ri) % N
    x3 = (r2 - h3 - t) % N
    y3 = (u1 * h2 * ri - x3) % N
    y3 = (r * y3 * ri) % N
    t = (s1 * h3 * ri) % N
    y3 = (y3 - t) % N
    z3 = (h * z1 * z2 * ri * ri) % N

    return x3, y3, z3

def point_double(x, y, z):
    s = (4 * x * y * y * ri * ri) % N
    m = (3 * x * x * ri) % N
    x3 = (m * m * ri) % N
    x3 = (x3 - s - s) % N
    y3 = (s - x3) % N
    y3 = (y3 * m * ri) % N
    t = (8 * y * y * y * y * ri * ri * ri) % N
    y3 = (y3 - t) % N
    z3 = (2 * y * z * ri) % N

    return x3, y3, z3

def gcd(a, b):
    while a != 0:
        a, b = b % a, a
    return b

def modinverse(a, m):
    if gcd(a, m) != 1:
        return None
    u1, u2, u3 = 1, 0, a
    v1, v2, v3 = 0, 1, m
    while v3 != 0:
        q = u3 // v3
        v1, v2, v3, u1, u2, u3 = (u1 -q * v1), (u2 - q * v2), (u3 - q * v3), v1, v2, v3
    return u1 % m

def get_affine(x, y, z):
    v = modinverse(z, N)
    v = (v * 2**512) % N
    v2 = (v * v * ri) % N
    v3 = (v * v2 * ri) % N
    x3 = (x * v2 * ri) % N
    y3 = (y * v3 * ri) % N
    x3 = (x3 * ri) % N
    y3 = (y3 * ri) % N
    return x3, y3

if __name__=="__main__":
    # point in montomery domain
    # gx = 0x9981e643e9089f48979f48c033fd129c231e295329bc66dbd7362e5a487e2097
    # gy = 0xcf3f851fd4a582d670b6b59aac19c1368dfc5d5d1f1dc64db15ea6d2d3dbabe2
    # gz = 0x00000000000000000000000000000000000000000000000000000001000003d1


    gx = 0x8bfdde00ea61950fb83d8a3764f84f8b902d59fb05705e90cb152ffff178da27
    gy = 0x3c1bdbb650ad240dcbca4b292ec79357f9c5e2ddc5c89a6771ec80aef7441c67
    gz = 0xfd088648e4b25c296c58584716a521d8c22b61c8092bb781b3a9a0e5712d4616

    x = gx
    y = gy
    z = gz

    # scalar
    k  = 0x826fffff656879890900000000000000000000000000000000fffffe643

    # int to bit array
    bits = []
    while k > 0:
        if k%2 == 1:
            bits.append(1)
        else:
            bits.append(0)
        k = k >> 1
    bits.reverse()
    print(bits)

    # double-add
    for i in range(len(bits)-1):
        x, y, z = point_double(x, y, z)
        if bits[i+1] == 1:
            x, y, z = point_add(x, y, z, gx, gy, gz)

    print('X = %#x'%x)
    print('Y = %#x'%y)
    print('Z = %#x'%z)

    ax, ay = get_affine(gx, gy, gz)
    print('x = %#x'%ax)
    print('y = %#x'%ay)