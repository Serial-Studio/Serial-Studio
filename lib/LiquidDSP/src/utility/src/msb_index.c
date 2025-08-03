/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "liquid.internal.h"

// number of leading zeros in a byte
//  0   0000 0000   :   8
//  1   0000 0001   :   7
//  2   0000 0010   :   6
//  3   0000 0011   :   6
//  4   0000 0100   :   5
//  ...
//  126 0111 1110   :   1
//  127 0111 1111   :   1
//  128 1000 0000   :   0
//  129 1000 0001   :   0
//  ...
//  253 1111 1101   :   0
//  254 1111 1110   :   0
//  255 1111 1111   :   0
unsigned int liquid_c_leading_zeros[256] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// count leading zeros in an integer (like leading_zeros[], but
// extending to integer)
unsigned int liquid_count_leading_zeros(unsigned int _x) 
{
#if 0
    // look for first non-zero byte in _x

    int i, B, clz=0;

    for (i=(SIZEOF_INT-1)*8; i>=0; i-=8) {
        B = (_x >> i) & 0xff;
        if ( B ) // B != 0
            return clz + leading_zeros[B];
        else
            clz += 8;
    }   
    return (SIZEOF_INT*8);
#else
    return SIZEOF_INT*8 - liquid_msb_index(_x);
#endif
}

// computes the index of the most-significant bit
//
// Examples:
//  0x00000000  :   0
//  0x00000001  :   1
//  0x00000002  :   2
//  ...
//  0x00010000  :   17
//  0x00020000  :   17
//  0x00040000  :   17
//  ...
//  0x80000000  :   32
//
//  0   0000 0000   :   0
//  1   0000 0001   :   1
//  2   0000 0010   :   2
//  3   0000 0011   :   2
//  4   0000 0100   :   3
//  ...
//  126 0111 1110   :   7
//  127 0111 1111   :   7
//  128 1000 0000   :   8
//  129 1000 0001   :   8
//  ...
//  253 1111 1101   :   8
//  254 1111 1110   :   8
//  255 1111 1111   :   8

unsigned int liquid_msb_index(unsigned int _x)
{
    unsigned int bits;

#if defined __i386__ || defined __amd64__ || defined __x86_64__
    if (!_x) return 0;
    __asm volatile("bsrl %1,%0\n"
        : "=r" (bits)
        : "c" (_x)
    );
    return bits + 1;
#elif 0
    // slow method; look one bit at a time
    for (bits = 0; _x != 0 && bits < 32; _x >>= 1, ++bits)
        ;
    return bits;
#else
    // look for first non-zero byte
    unsigned int i, b;
    bits = 8*SIZEOF_INT;
    for (i=SIZEOF_INT*8; i>0; i-=8) {
        b = (_x >> (i-8)) & 0xff;
        if ( b )
            return bits - liquid_c_leading_zeros[b];
        else
            bits -= 8;
    }
    return 0;

#endif
}


