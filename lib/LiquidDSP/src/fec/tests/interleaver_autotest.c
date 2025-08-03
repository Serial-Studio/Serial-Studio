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

#include <stdlib.h>

#include "autotest/autotest.h"
#include "liquid.h"

// 
// AUTOTESTS: interleave/deinterleave
//
void interleaver_test_hard(unsigned int _n)
{
    unsigned int i;
    unsigned char x[_n];
    unsigned char y[_n];
    unsigned char z[_n];

    for (i=0; i<_n; i++)
        x[i] = rand() & 0xFF;

    // create interleaver object
    interleaver q = interleaver_create(_n);

    interleaver_encode(q,x,y);
    interleaver_decode(q,y,z);

    CONTEND_SAME_DATA(x, z, _n);

    // destroy interleaver object
    interleaver_destroy(q);
}

// 
// AUTOTESTS: interleave/deinterleave (soft)
//
void interleaver_test_soft(unsigned int _n)
{
    unsigned int i;
    unsigned char x[8*_n];
    unsigned char y[8*_n];
    unsigned char z[8*_n];

    for (i=0; i<8*_n; i++)
        x[i] = rand() & 0xFF;

    // create interleaver object
    interleaver q = interleaver_create(_n);

    interleaver_encode_soft(q,x,y);
    interleaver_decode_soft(q,y,z);

    CONTEND_SAME_DATA(x, z, 8*_n);
    
    // destroy interleaver object
    interleaver_destroy(q);
}

void autotest_interleaver_hard_8()      { interleaver_test_hard(8   ); }
void autotest_interleaver_hard_16()     { interleaver_test_hard(16  ); }
void autotest_interleaver_hard_64()     { interleaver_test_hard(64  ); }
void autotest_interleaver_hard_256()    { interleaver_test_hard(256 ); }

void autotest_interleaver_soft_8()      { interleaver_test_soft(8   ); }
void autotest_interleaver_soft_16()     { interleaver_test_soft(16  ); }
void autotest_interleaver_soft_64()     { interleaver_test_soft(64  ); }
void autotest_interleaver_soft_256()    { interleaver_test_soft(256 ); }

