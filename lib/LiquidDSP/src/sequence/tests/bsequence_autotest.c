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
#include "liquid.h"

// 
// test initialization of binary sequence
//
void autotest_bsequence_init()
{
    // 1111 0000 1100 1010
    unsigned char v[2] = {0xf0, 0xca};

    // create and initialize sequence
    bsequence q = bsequence_create(16);
    bsequence_init(q,v);

    // run tests
    CONTEND_EQUALITY( bsequence_index(q,15), 1 );
    CONTEND_EQUALITY( bsequence_index(q,14), 1 );
    CONTEND_EQUALITY( bsequence_index(q,13), 1 );
    CONTEND_EQUALITY( bsequence_index(q,12), 1 );
    
    CONTEND_EQUALITY( bsequence_index(q,11), 0 );
    CONTEND_EQUALITY( bsequence_index(q,10), 0 );
    CONTEND_EQUALITY( bsequence_index(q, 9), 0 );
    CONTEND_EQUALITY( bsequence_index(q, 8), 0 );
    
    CONTEND_EQUALITY( bsequence_index(q, 7), 1 );
    CONTEND_EQUALITY( bsequence_index(q, 6), 1 );
    CONTEND_EQUALITY( bsequence_index(q, 5), 0 );
    CONTEND_EQUALITY( bsequence_index(q, 4), 0 );
    
    CONTEND_EQUALITY( bsequence_index(q, 3), 1 );
    CONTEND_EQUALITY( bsequence_index(q, 2), 0 );
    CONTEND_EQUALITY( bsequence_index(q, 1), 1 );
    CONTEND_EQUALITY( bsequence_index(q, 0), 0 );
    
    // clean up memory
    bsequence_destroy(q);
}

// 
// test correlation between to sequences
//
void autotest_bsequence_correlate()
{
    // v0   :   1111 0000 1100 1010
    // v1   :   1100 1011 0001 1110
    // sim  :   1100 0100 0010 1011 (7 similar bits)
    unsigned char v0[2] = {0xf0, 0xca};
    unsigned char v1[2] = {0xcb, 0x1e};

    // create and initialize sequences
    bsequence q0 = bsequence_create(16);
    bsequence q1 = bsequence_create(16);
    bsequence_init(q0,v0);
    bsequence_init(q1,v1);

    // run tests
    CONTEND_EQUALITY( bsequence_correlate(q0,q1), 7 );
    
    // clean up memory
    bsequence_destroy(q0);
    bsequence_destroy(q1);
}


// 
// test add operations on two sequences
//
void autotest_bsequence_add()
{
    // v0   :   1111 0000 1100 1010
    // v1   :   1100 1011 0001 1110
    // sum  :   0011 1011 1101 0100
    unsigned char v0[2] = {0xf0, 0xca};
    unsigned char v1[2] = {0xcb, 0x1e};

    // create and initialize sequences
    bsequence q0 = bsequence_create(16);
    bsequence q1 = bsequence_create(16);
    bsequence_init(q0,v0);
    bsequence_init(q1,v1);

    // create result sequence
    bsequence r = bsequence_create(16);
    bsequence_add(q0, q1, r);

    // run tests
    CONTEND_EQUALITY( bsequence_index(r,15), 0 );
    CONTEND_EQUALITY( bsequence_index(r,14), 0 );
    CONTEND_EQUALITY( bsequence_index(r,13), 1 );
    CONTEND_EQUALITY( bsequence_index(r,12), 1 );
    
    CONTEND_EQUALITY( bsequence_index(r,11), 1 );
    CONTEND_EQUALITY( bsequence_index(r,10), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 9), 1 );
    CONTEND_EQUALITY( bsequence_index(r, 8), 1 );
    
    CONTEND_EQUALITY( bsequence_index(r, 7), 1 );
    CONTEND_EQUALITY( bsequence_index(r, 6), 1 );
    CONTEND_EQUALITY( bsequence_index(r, 5), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 4), 1 );
    
    CONTEND_EQUALITY( bsequence_index(r, 3), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 2), 1 );
    CONTEND_EQUALITY( bsequence_index(r, 1), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 0), 0 );
    
    // clean up memory
    bsequence_destroy(q0);
    bsequence_destroy(q1);
    bsequence_destroy(r);
}

// 
// test multiply operations on two sequences
//
void autotest_bsequence_mul()
{
    // v0   :   1111 0000 1100 1010
    // v1   :   1100 1011 0001 1110
    // prod :   1100 0000 0000 1010
    unsigned char v0[2] = {0xf0, 0xca};
    unsigned char v1[2] = {0xcb, 0x1e};

    // create and initialize sequences
    bsequence q0 = bsequence_create(16);
    bsequence q1 = bsequence_create(16);
    bsequence_init(q0,v0);
    bsequence_init(q1,v1);

    // create result sequence
    bsequence r = bsequence_create(16);
    bsequence_mul(q0, q1, r);

    // run tests
    CONTEND_EQUALITY( bsequence_index(r,15), 1 );
    CONTEND_EQUALITY( bsequence_index(r,14), 1 );
    CONTEND_EQUALITY( bsequence_index(r,13), 0 );
    CONTEND_EQUALITY( bsequence_index(r,12), 0 );
    
    CONTEND_EQUALITY( bsequence_index(r,11), 0 );
    CONTEND_EQUALITY( bsequence_index(r,10), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 9), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 8), 0 );
    
    CONTEND_EQUALITY( bsequence_index(r, 7), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 6), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 5), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 4), 0 );
    
    CONTEND_EQUALITY( bsequence_index(r, 3), 1 );
    CONTEND_EQUALITY( bsequence_index(r, 2), 0 );
    CONTEND_EQUALITY( bsequence_index(r, 1), 1 );
    CONTEND_EQUALITY( bsequence_index(r, 0), 0 );
    
    // clean up memory
    bsequence_destroy(q0);
    bsequence_destroy(q1);
    bsequence_destroy(r);
}

// 
// test accumulation of binary sequence
//
void autotest_bsequence_accumulate()
{
    // 1111 0000 1100 1010 (8 total bits)
    unsigned char v[2] = {0xf0, 0xca};

    // create and initialize sequence
    bsequence q = bsequence_create(16);
    bsequence_init(q,v);

    // run tests
    CONTEND_EQUALITY( bsequence_accumulate(q), 8 );
    
    // clean up memory
    bsequence_destroy(q);
}


