/*
 * Copyright (c) 2007 - 2025 Joseph Gaeddert
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
// sumsq.avx.c : floating-point sum of squares (AVX)
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <immintrin.h>
#include "liquid.internal.h"

// sum squares, basic loop
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqf_avx(float *      _v,
                        unsigned int _n)
{
    // first cut: ...
    __m256 v;   // input vector
    __m256 s;   // product
    __m256 sum = _mm256_setzero_ps(); // load zeros into sum register

    // t = 8*(floor(_n/8))
    unsigned int t = (_n >> 3) << 3;

    //
    unsigned int i;
    for (i=0; i<t; i+=8) {
        // load inputs into register (unaligned)
        v = _mm256_loadu_ps(&_v[i]);

        // compute multiplication
        s = _mm256_mul_ps(v, v);
       
        // parallel addition
        sum = _mm256_add_ps( sum, s );
    }

    // fold down into single value
    __m256 z = _mm256_setzero_ps();
    sum = _mm256_hadd_ps(sum, z);
    sum = _mm256_hadd_ps(sum, z);

    // aligned output array
    float w[8] __attribute__((aligned(32)));

    _mm256_store_ps(w, sum);
    float total = w[0] + w[4];

    // cleanup
    for (; i<_n; i++)
        total += _v[i] * _v[i];

    // set return value
    return total;
}

// sum squares, unrolled loop
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqf_avxu(float *      _v,
                         unsigned int _n)
{
    // first cut: ...
    __m256 v0, v1, v2, v3;   // input vector
    __m256 s0, s1, s2, s3;   // product
    __m256 sum = _mm256_setzero_ps(); // load zeros into sum register

    // t = 8*(floor(_n/32))
    unsigned int t = (_n >> 5) << 3;

    //
    unsigned int i;
    for (i=0; i<t; i+=8) {
        // load inputs into register (unaligned)
        v0 = _mm256_loadu_ps(&_v[4*i+ 0]);
        v1 = _mm256_loadu_ps(&_v[4*i+ 8]);
        v2 = _mm256_loadu_ps(&_v[4*i+16]);
        v3 = _mm256_loadu_ps(&_v[4*i+24]);

        // compute multiplication
        s0 = _mm256_mul_ps(v0, v0);
        s1 = _mm256_mul_ps(v1, v1);
        s2 = _mm256_mul_ps(v2, v2);
        s3 = _mm256_mul_ps(v3, v3);
       
        // parallel addition
        sum = _mm256_add_ps( sum, s0 );
        sum = _mm256_add_ps( sum, s1 );
        sum = _mm256_add_ps( sum, s2 );
        sum = _mm256_add_ps( sum, s3 );
    }

    // fold down into single value
    __m256 z = _mm256_setzero_ps();
    sum = _mm256_hadd_ps(sum, z);
    sum = _mm256_hadd_ps(sum, z);

    // aligned output array
    float w[8] __attribute__((aligned(32)));

    _mm256_store_ps(w, sum);
    float total = w[0] + w[4];

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
    if (_n < 32) {
        return liquid_sumsqf_avx(_v, _n);
    }
    return liquid_sumsqf_avxu(_v, _n);
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
