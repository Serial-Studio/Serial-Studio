/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
// Floating-point dot product (SSE)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <immintrin.h>

#include "liquid.internal.h"

#define DEBUG_DOTPROD_RRRF_SSE   0

// internal methods
int dotprod_rrrf_execute_sse(dotprod_rrrf _q,
                             float *      _x,
                             float *      _y);
int dotprod_rrrf_execute_sse4(dotprod_rrrf _q,
                              float *      _x,
                              float *      _y);

// basic dot product (ordinal calculation)
int dotprod_rrrf_run(float *      _h,
                     float *      _x,
                     unsigned int _n,
                     float *      _y)
{
    float r=0;
    unsigned int i;
    for (i=0; i<_n; i++)
        r += _h[i] * _x[i];
    *_y = r;
    return LIQUID_OK;
}

// basic dot product (ordinal calculation) with loop unrolled
int dotprod_rrrf_run4(float *      _h,
                      float *      _x,
                      unsigned int _n,
                      float *      _y)
{
    float r=0;

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
// structured SSE dot product
//

struct dotprod_rrrf_s {
    unsigned int n;     // length
    float * h;          // coefficients array
};

dotprod_rrrf dotprod_rrrf_create_opt(float *      _h,
                                     unsigned int _n,
                                     int          _rev)
{
    dotprod_rrrf q = (dotprod_rrrf)malloc(sizeof(struct dotprod_rrrf_s));
    q->n = _n;

    // allocate memory for coefficients, 16-byte aligned
    q->h = (float*) _mm_malloc( q->n*sizeof(float), 16);

    // set coefficients
    unsigned int i;
    for (i=0; i<q->n; i++)
        q->h[i] = _h[_rev ? q->n-i-1 : i];

    // return object
    return q;
}

dotprod_rrrf dotprod_rrrf_create(float *      _h,
                                 unsigned int _n)
{
    return dotprod_rrrf_create_opt(_h, _n, 0);
}

dotprod_rrrf dotprod_rrrf_create_rev(float *      _h,
                                     unsigned int _n)
{
    return dotprod_rrrf_create_opt(_h, _n, 1);
}

// re-create the structured dotprod object
dotprod_rrrf dotprod_rrrf_recreate(dotprod_rrrf _q,
                                   float *      _h,
                                   unsigned int _n)
{
    // completely destroy and re-create dotprod object
    dotprod_rrrf_destroy(_q);
    return dotprod_rrrf_create(_h,_n);
}

// re-create the structured dotprod object, coefficients reversed
dotprod_rrrf dotprod_rrrf_recreate_rev(dotprod_rrrf _q,
                                       float *      _h,
                                       unsigned int _n)
{
    // completely destroy and re-create dotprod object
    dotprod_rrrf_destroy(_q);
    return dotprod_rrrf_create_rev(_h,_n);
}

dotprod_rrrf dotprod_rrrf_copy(dotprod_rrrf q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("dotprod_rrrf_copy().sse, object cannot be NULL");

    dotprod_rrrf q_copy = (dotprod_rrrf)malloc(sizeof(struct dotprod_rrrf_s));
    q_copy->n = q_orig->n;

    // allocate memory for coefficients, 16-byte aligned
    q_copy->h = (float*) _mm_malloc( q_copy->n*sizeof(float), 16 );

    // copy coefficients array
    memmove(q_copy->h, q_orig->h, q_orig->n*sizeof(float));

    // return object
    return q_copy;
}

int dotprod_rrrf_destroy(dotprod_rrrf _q)
{
    _mm_free(_q->h);
    free(_q);
    return LIQUID_OK;
}

int dotprod_rrrf_print(dotprod_rrrf _q)
{
    printf("dotprod_rrrf [sse, %u coefficients]\n", _q->n);
    unsigned int i;
    for (i=0; i<_q->n; i++)
        printf("%3u : %12.9f\n", i, _q->h[i]);
    return LIQUID_OK;
}

// 
int dotprod_rrrf_execute(dotprod_rrrf _q,
                          float *      _x,
                          float *      _y)
{
    // switch based on size
    if (_q->n < 16) {
        return dotprod_rrrf_execute_sse(_q, _x, _y);
    }
    return dotprod_rrrf_execute_sse4(_q, _x, _y);
}

// use SSE extensions
int dotprod_rrrf_execute_sse(dotprod_rrrf _q,
                             float *      _x,
                             float *      _y)
{
    // first cut: ...
    __m128 v;   // input vector
    __m128 h;   // coefficients vector
    __m128 s;   // dot product
    __m128 sum = _mm_setzero_ps(); // load zeros into sum register

    // t = 4*(floor(_n/4))
    unsigned int t = (_q->n >> 2) << 2;

    //
    unsigned int i;
    for (i=0; i<t; i+=4) {
        // load inputs into register (unaligned)
        v = _mm_loadu_ps(&_x[i]);

        // load coefficients into register (aligned)
        h = _mm_load_ps(&_q->h[i]);

        // compute multiplication
        s = _mm_mul_ps(v, h);

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
    for (; i<_q->n; i++)
        total += _x[i] * _q->h[i];

    // set return value
    *_y = total;
    return LIQUID_OK;
}

// use SSE extensions, unrolled loop
int dotprod_rrrf_execute_sse4(dotprod_rrrf _q,
                              float *      _x,
                              float *      _y)
{
    // first cut: ...
    __m128 v0, v1, v2, v3;
    __m128 h0, h1, h2, h3;
    __m128 s0, s1, s2, s3;

    // load zeros into sum registers
    __m128 sum = _mm_setzero_ps();

    // r = 4*floor(n/16)
    unsigned int r = (_q->n >> 4) << 2;

    //
    unsigned int i;
    for (i=0; i<r; i+=4) {
        // load inputs into register (unaligned)
        v0 = _mm_loadu_ps(&_x[4*i+0]);
        v1 = _mm_loadu_ps(&_x[4*i+4]);
        v2 = _mm_loadu_ps(&_x[4*i+8]);
        v3 = _mm_loadu_ps(&_x[4*i+12]);

        // load coefficients into register (aligned)
        h0 = _mm_load_ps(&_q->h[4*i+0]);
        h1 = _mm_load_ps(&_q->h[4*i+4]);
        h2 = _mm_load_ps(&_q->h[4*i+8]);
        h3 = _mm_load_ps(&_q->h[4*i+12]);

        // compute multiplication
        s0 = _mm_mul_ps(v0, h0);
        s1 = _mm_mul_ps(v1, h1);
        s2 = _mm_mul_ps(v2, h2);
        s3 = _mm_mul_ps(v3, h3);
        
        // parallel addition
        sum = _mm_add_ps( sum, s0 );
        sum = _mm_add_ps( sum, s1 );
        sum = _mm_add_ps( sum, s2 );
        sum = _mm_add_ps( sum, s3 );
    }

    // aligned output array
    float w[4] __attribute__((aligned(16)));

#if HAVE_SSE3
    // SSE3: fold down to single value using _mm_hadd_ps()
    __m128 z = _mm_setzero_ps();
    sum = _mm_hadd_ps(sum, z);
    sum = _mm_hadd_ps(sum, z);
   
    // unload single (lower value)
    _mm_store_ss(w, sum);
    float total = w[0];
#else
    // SSE2 and below: unload packed array and perform manual sum
    _mm_store_ps(w, sum);
    float total = w[0] + w[1] + w[2] + w[3];
#endif

    // cleanup
    // TODO : use intrinsics here as well
    for (i=4*r; i<_q->n; i++)
        total += _x[i] * _q->h[i];

    // set return value
    *_y = total;
    return LIQUID_OK;
}

