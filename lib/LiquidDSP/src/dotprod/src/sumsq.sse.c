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

//
// sumsq.sse.c : floating-point sum of squares (SSE)
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <immintrin.h>

#include "liquid.internal.h"

// sum squares, basic loop
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqf_sse(float *      _v,
                        unsigned int _n)
{
    // first cut: ...
    __m128 v;   // input vector
    __m128 s;   // dot product
    __m128 sum = _mm_setzero_ps(); // load zeros into sum register

    // t = 4*(floor(_n/4))
    unsigned int t = (_n >> 2) << 2;

    //
    unsigned int i;
    for (i=0; i<t; i+=4) {
        // load inputs into register (unaligned)
        v = _mm_loadu_ps(&_v[i]);

        // compute multiplication
        s = _mm_mul_ps(v, v);
       
        // parallel addition
        sum = _mm_add_ps( sum, s );
    }

    // aligned output array
    float w[4] __attribute__((aligned(16)));

#if HAVE_SSE3
    // fold down into single value
    __m128 z = _mm_setzero_ps();
    sum = _mm_hadd_ps(sum, z);
    sum = _mm_hadd_ps(sum, z);
   
    // unload single (lower value)
    _mm_store_ss(w, sum);
    float total = w[0];
#else
    // unload packed array
    _mm_store_ps(w, sum);
    float total = w[0] + w[1] + w[2] + w[3];
#endif

    // cleanup
    for (; i<_n; i++)
        total += _v[i] * _v[i];

    // set return value
    return total;
}

// sum squares, unrolled loop
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqf_sseu(float *      _v,
                         unsigned int _n)
{
    // first cut: ...
    __m128 v0, v1, v2, v3;   // input vector
    __m128 s0, s1, s2, s3;   // product
    __m128 sum = _mm_setzero_ps(); // load zeros into sum register

    // t = 4*(floor(_n/16))
    unsigned int t = (_n >> 4) << 2;

    //
    unsigned int i;
    for (i=0; i<t; i+=4) {
        // load inputs into register (unaligned)
        v0 = _mm_loadu_ps(&_v[4*i+ 0]);
        v1 = _mm_loadu_ps(&_v[4*i+ 4]);
        v2 = _mm_loadu_ps(&_v[4*i+ 8]);
        v3 = _mm_loadu_ps(&_v[4*i+12]);

        // compute multiplication
        s0 = _mm_mul_ps(v0, v0);
        s1 = _mm_mul_ps(v1, v1);
        s2 = _mm_mul_ps(v2, v2);
        s3 = _mm_mul_ps(v3, v3);

        // parallel addition
        sum = _mm_add_ps( sum, s0 );
        sum = _mm_add_ps( sum, s1 );
        sum = _mm_add_ps( sum, s2 );
        sum = _mm_add_ps( sum, s3 );
    }

    // aligned output array
    float w[4] __attribute__((aligned(16)));

#if HAVE_SSE3
    // fold down into single value
    __m128 z = _mm_setzero_ps();
    sum = _mm_hadd_ps(sum, z);
    sum = _mm_hadd_ps(sum, z);
   
    // unload single (lower value)
    _mm_store_ss(w, sum);
    float total = w[0];
#else
    // unload packed array
    _mm_store_ps(w, sum);
    float total = w[0] + w[1] + w[2] + w[3];
#endif

    // cleanup
    for (i=4*t; i<_n; i++)
        total += _v[i] * _v[i];

    // set return value
    return total;
}

// sum squares
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqf(float *      _v,
                    unsigned int _n)
{
    // switch based on size
    if (_n < 16) {
        return liquid_sumsqf_sse(_v, _n);
    }
    return liquid_sumsqf_sseu(_v, _n);
}

// sum squares, complex
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqcf(float complex * _v,
                     unsigned int    _n)
{
    // simple method: type cast input as real pointer, run double
    // length sumsqf method
    float * v = (float*) _v;
    return liquid_sumsqf(v, 2*_n);
}
