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

#include "autotest/autotest.h"
#include "liquid.internal.h"

// 
// AUTOTEST: count number of ones in an integer
//
void autotest_count_ones() {
    CONTEND_EQUALITY( liquid_count_ones(0x0000), 0 );
    CONTEND_EQUALITY( liquid_count_ones(0x0001), 1 );
    CONTEND_EQUALITY( liquid_count_ones(0x0003), 2 );
    CONTEND_EQUALITY( liquid_count_ones(0xFFFF), 16 );
    CONTEND_EQUALITY( liquid_count_ones(0x00FF), 8 );
    CONTEND_EQUALITY( liquid_count_ones(0x5555), 8 );
    CONTEND_EQUALITY( liquid_count_ones(0x0007), 3 );
    CONTEND_EQUALITY( liquid_count_ones(0x0037), 5 );
    CONTEND_EQUALITY( liquid_count_ones(0x0137), 6 );
    CONTEND_EQUALITY( liquid_count_ones(0xf137), 10 );
}

// 
// AUTOTEST: count number of ones in an integer (modulo 2)
//
void autotest_count_ones_mod2() {
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x0000), 0 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x0001), 1 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x0003), 0 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0xFFFF), 0 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x00FF), 0 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x5555), 0 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x0007), 1 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x0037), 1 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0x0137), 0 );
    CONTEND_EQUALITY( liquid_count_ones_mod2(0xf137), 0 );
}

// 
// AUTOTEST: binary dot product
//
void autotest_bdotprod()
{
    // simple checks
    CONTEND_EQUALITY( liquid_bdotprod(0x1111,0x1111), 0 );
    CONTEND_EQUALITY( liquid_bdotprod(0xffff,0xffff), 0 );
    CONTEND_EQUALITY( liquid_bdotprod(0xffff,0x0000), 0 );
    CONTEND_EQUALITY( liquid_bdotprod(0x0001,0x0001), 1 );

    // random data
    CONTEND_EQUALITY( liquid_bdotprod(0x4379, 0xf2dc), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0xc9a1, 0xc99d), 0);
    CONTEND_EQUALITY( liquid_bdotprod(0xa8ba, 0x26d9), 0);
    CONTEND_EQUALITY( liquid_bdotprod(0x5235, 0x8e1b), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0x0f85, 0xa3d1), 0);
    CONTEND_EQUALITY( liquid_bdotprod(0x23e0, 0x5869), 0);
    CONTEND_EQUALITY( liquid_bdotprod(0xc8a4, 0x32a4), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0xe1c3, 0x000c), 0);
    CONTEND_EQUALITY( liquid_bdotprod(0x4039, 0x192d), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0x2e1c, 0x55a3), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0x5a1b, 0x0241), 0);
    CONTEND_EQUALITY( liquid_bdotprod(0x440c, 0x7ddb), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0xd2e2, 0x5c98), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0xe36c, 0x5bc9), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0xaa96, 0xf233), 1);
    CONTEND_EQUALITY( liquid_bdotprod(0xab0f, 0x3912), 0);
}

// 
// AUTOTEST: count number of leading zeros in an integer
//
void autotest_count_leading_zeros() {
    // NOTE: this tests assumes a 4-byte integer

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000000), 32 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000001), 31 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000002), 30 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000004), 29 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000008), 28 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000010), 27 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000020), 26 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000040), 25 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000080), 24 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000100), 23 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000200), 22 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000400), 21 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00000800), 20 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00001000), 19 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00002000), 18 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00004000), 17 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00008000), 16 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00010000), 15 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00020000), 14 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00040000), 13 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00080000), 12 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00100000), 11 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00200000), 10 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00400000),  9 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x00800000),  8 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x01000000),  7 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x02000000),  6 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x04000000),  5 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x08000000),  4 );

    CONTEND_EQUALITY( liquid_count_leading_zeros(0x10000000),  3 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x20000000),  2 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x40000000),  1 );
    CONTEND_EQUALITY( liquid_count_leading_zeros(0x80000000),  0 );
}

// 
// AUTOTEST: find location of most-significant bit
//
void autotest_msb_index() {
    // NOTE: this tests assumes a 4-byte integer

    CONTEND_EQUALITY( liquid_msb_index(0x00000000),  0 );

    CONTEND_EQUALITY( liquid_msb_index(0x00000001),  1 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000002),  2 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000004),  3 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000008),  4 );

    CONTEND_EQUALITY( liquid_msb_index(0x00000010),  5 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000020),  6 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000040),  7 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000080),  8 );

    CONTEND_EQUALITY( liquid_msb_index(0x00000100),  9 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000200), 10 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000400), 11 );
    CONTEND_EQUALITY( liquid_msb_index(0x00000800), 12 );

    CONTEND_EQUALITY( liquid_msb_index(0x00001000), 13 );
    CONTEND_EQUALITY( liquid_msb_index(0x00002000), 14 );
    CONTEND_EQUALITY( liquid_msb_index(0x00004000), 15 );
    CONTEND_EQUALITY( liquid_msb_index(0x00008000), 16 );

    CONTEND_EQUALITY( liquid_msb_index(0x00010000), 17 );
    CONTEND_EQUALITY( liquid_msb_index(0x00020000), 18 );
    CONTEND_EQUALITY( liquid_msb_index(0x00040000), 19 );
    CONTEND_EQUALITY( liquid_msb_index(0x00080000), 20 );

    CONTEND_EQUALITY( liquid_msb_index(0x00100000), 21 );
    CONTEND_EQUALITY( liquid_msb_index(0x00200000), 22 );
    CONTEND_EQUALITY( liquid_msb_index(0x00400000), 23 );
    CONTEND_EQUALITY( liquid_msb_index(0x00800000), 24 );

    CONTEND_EQUALITY( liquid_msb_index(0x01000000), 25 );
    CONTEND_EQUALITY( liquid_msb_index(0x02000000), 26 );
    CONTEND_EQUALITY( liquid_msb_index(0x04000000), 27 );
    CONTEND_EQUALITY( liquid_msb_index(0x08000000), 28 );

    CONTEND_EQUALITY( liquid_msb_index(0x10000000), 29 );
    CONTEND_EQUALITY( liquid_msb_index(0x20000000), 30 );
    CONTEND_EQUALITY( liquid_msb_index(0x40000000), 31 );
    CONTEND_EQUALITY( liquid_msb_index(0x80000000), 32 );
}

