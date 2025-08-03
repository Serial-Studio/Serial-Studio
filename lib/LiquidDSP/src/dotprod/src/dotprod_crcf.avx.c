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
// Floating-point dot product (AVX)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <assert.h>
#include "liquid.internal.h"

#define DEBUG_DOTPROD_CRCF_AVX   0

// forward declaration of internal methods
int dotprod_crcf_execute_avx(dotprod_crcf    _q,
                             float complex * _x,
                             float complex * _y);
int dotprod_crcf_execute_avx4(dotprod_crcf    _q,
                              float complex * _x,
                              float complex * _y);

// basic dot product (ordinal calculation)
int dotprod_crcf_run(float *         _h,
                     float complex * _x,
                     unsigned int    _n,
                     float complex * _y)
{
    float complex r = 0;
    unsigned int i;
    for (i=0; i<_n; i++)
        r += _h[i] * _x[i];
    *_y = r;
    return LIQUID_OK;
}

// basic dot product (ordinal calculation) with loop unrolled
int dotprod_crcf_run4(float *         _h,
                      float complex * _x,
                      unsigned int    _n,
                      float complex * _y)
{
    float complex r = 0;

    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // compute dotprod in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
        r += _h[i]   * _x[i];
        r += _h[i+1] * _x[i+1];
        r += _h[i+2] * _x[i+2];
        r += _h[i+3] * _x[i+3];
    }

    // clean up remaining
    for ( ; i<_n; i++)
        r += _h[i] * _x[i];

    *_y = r;
    return LIQUID_OK;
}


//
// structured AVX dot product
//

struct dotprod_crcf_s {
    unsigned int n;     // length
    float * h;          // coefficients array
};

dotprod_crcf dotprod_crcf_create_opt(float *      _h,
                                     unsigned int _n,
                                     int          _rev)
{
    dotprod_crcf q = (dotprod_crcf)malloc(sizeof(struct dotprod_crcf_s));
    q->n = _n;

    // allocate memory for coefficients, 32-byte aligned
    q->h = (float*) _mm_malloc( 2*q->n*sizeof(float), 32 );

    // set coefficients, repeated
    //  h = { _h[0], _h[0], _h[1], _h[1], ... _h[n-1], _h[n-1]}
    unsigned int i;
    for (i=0; i<q->n; i++) {
        unsigned int k = _rev ? q->n-i-1 : i;
        q->h[2*i+0] = _h[k];
        q->h[2*i+1] = _h[k];
    }

    // return object
    return q;
}

dotprod_crcf dotprod_crcf_create(float *      _h,
                                 unsigned int _n)
{
    return dotprod_crcf_create_opt(_h, _n, 0);
}

dotprod_crcf dotprod_crcf_create_rev(float *      _h,
                                     unsigned int _n)
{
    return dotprod_crcf_create_opt(_h, _n, 1);
}

// re-create the structured dotprod object
dotprod_crcf dotprod_crcf_recreate(dotprod_crcf _q,
                                   float *      _h,
                                   unsigned int _n)
{
    // completely destroy and re-create dotprod object
    dotprod_crcf_destroy(_q);
    return dotprod_crcf_create(_h,_n);
}

// re-create the structured dotprod object, coefficients reversed
dotprod_crcf dotprod_crcf_recreate_rev(dotprod_crcf _q,
                                       float *      _h,
                                       unsigned int _n)
{
    // completely destroy and re-create dotprod object
    dotprod_crcf_destroy(_q);
    return dotprod_crcf_create_rev(_h,_n);
}

dotprod_crcf dotprod_crcf_copy(dotprod_crcf q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("dotprod_crcf_copy().avx, object cannot be NULL");

    dotprod_crcf q_copy = (dotprod_crcf)malloc(sizeof(struct dotprod_crcf_s));
    q_copy->n = q_orig->n;

    // allocate memory for coefficients, 32-byte aligned (repeated)
    q_copy->h = (float*) _mm_malloc( 2*q_copy->n*sizeof(float), 32 );

    // copy coefficients array (repeated)
    //  h = { _h[0], _h[0], _h[1], _h[1], ... _h[n-1], _h[n-1]}
    memmove(q_copy->h, q_orig->h, 2*q_orig->n*sizeof(float));

    // return object
    return q_copy;
}


int dotprod_crcf_destroy(dotprod_crcf _q)
{
    _mm_free(_q->h);
    free(_q);
    return LIQUID_OK;
}

int dotprod_crcf_print(dotprod_crcf _q)
{
    // print coefficients to screen, skipping odd entries (due
    // to repeated coefficients)
    printf("dotprod_crcf [avx, %u coefficients]\n", _q->n);
    unsigned int i;
    for (i=0; i<_q->n; i++)
        printf("  %3u : %12.9f\n", i, _q->h[2*i]);
    return LIQUID_OK;
}

// 
int dotprod_crcf_execute(dotprod_crcf    _q,
                         float complex * _x,
                         float complex * _y)
{
    // switch based on size
    if (_q->n < 64) {
        return dotprod_crcf_execute_avx(_q, _x, _y);
    }
    return dotprod_crcf_execute_avx4(_q, _x, _y);
}

// use AVX extensions
int dotprod_crcf_execute_avx(dotprod_crcf    _q,
                             float complex * _x,
                             float complex * _y)
{
    // type cast input as floating point array
    float * x = (float*) _x;

    // double effective length
    unsigned int n = 2*_q->n;

    // first cut: ...
    __m256 v;   // input vector
    __m256 h;   // coefficients vector
    __m256 s;   // dot product
    __m256 sum = _mm256_setzero_ps();  // load zeros into sum register

    // t = 8*(floor(_n/8))
    unsigned int t = (n >> 3) << 3;

    //
    unsigned int i;
    for (i=0; i<t; i+=8) {
        // load inputs into register (unaligned)
        v = _mm256_loadu_ps(&x[i]);

        // load coefficients into register (aligned)
        h = _mm256_load_ps(&_q->h[i]);

        // compute multiplication
        s = _mm256_mul_ps(v, h);

        // accumulate
        sum = _mm256_add_ps(sum, s);
    }

    // aligned output array
    float w[8] __attribute__((aligned(32)));

    // unload packed array
    _mm256_store_ps(w, sum);

    // add in-phase and quadrature components
    w[0] += w[2] + w[4] + w[6];
    w[1] += w[3] + w[5] + w[7];

    // cleanup (note: n _must_ be even)
    for (; i<n; i+=2) {
        w[0] += x[i  ] * _q->h[i  ];
        w[1] += x[i+1] * _q->h[i+1];
    }

    // set return value
    *_y = w[0] + _Complex_I*w[1];
    return LIQUID_OK;
}

// use AVX extensions
int dotprod_crcf_execute_avx4(dotprod_crcf    _q,
                              float complex * _x,
                              float complex * _y)
{
    // type cast input as floating point array
    float * x = (float*) _x;

    // double effective length
    unsigned int n = 2*_q->n;

    // first cut: ...
    __m256 v0, v1, v2, v3;  // input vectors
    __m256 h0, h1, h2, h3;  // coefficients vectors
    __m256 s0, s1, s2, s3;  // dot products [re, im, re, im]

    // load zeros into sum registers
    __m256 sum = _mm256_setzero_ps();

    // r = 8*floor(n/32)
    unsigned int r = (n >> 5) << 3;

    //
    unsigned int i;
    for (i=0; i<r; i+=8) {
        // load inputs into register (unaligned)
        v0 = _mm256_loadu_ps(&x[4*i+0]);
        v1 = _mm256_loadu_ps(&x[4*i+8]);
        v2 = _mm256_loadu_ps(&x[4*i+16]);
        v3 = _mm256_loadu_ps(&x[4*i+24]);

        // load coefficients into register (aligned)
        h0 = _mm256_load_ps(&_q->h[4*i+0]);
        h1 = _mm256_load_ps(&_q->h[4*i+8]);
        h2 = _mm256_load_ps(&_q->h[4*i+16]);
        h3 = _mm256_load_ps(&_q->h[4*i+24]);

        // compute multiplication
        s0 = _mm256_mul_ps(v0, h0);
        s1 = _mm256_mul_ps(v1, h1);
        s2 = _mm256_mul_ps(v2, h2);
        s3 = _mm256_mul_ps(v3, h3);
        
        // parallel addition
        sum = _mm256_add_ps( sum, s0 );
        sum = _mm256_add_ps( sum, s1 );
        sum = _mm256_add_ps( sum, s2 );
        sum = _mm256_add_ps( sum, s3 );
    }

    // aligned output array
    float w[8] __attribute__((aligned(32)));

    // unload packed array and perform manual sum
    _mm256_store_ps(w, sum);
    w[0] += w[2] + w[4] + w[6];
    w[1] += w[3] + w[5] + w[7];

    // cleanup (note: n _must_ be even)
    for (i=4*r; i<n; i+=2) {
        w[0] += x[i  ] * _q->h[i  ];
        w[1] += x[i+1] * _q->h[i+1];
    }

    // set return value
    *_y = w[0] + w[1]*_Complex_I;
    return LIQUID_OK;
}

