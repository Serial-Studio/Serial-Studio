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
// Floating-point dot product (ARM Neon)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

// include proper SIMD extensions for ARM Neon
#include <arm_neon.h>

#define DEBUG_DOTPROD_RRRF_NEON   0

// basic dot product (ordinal calculation) using neon extensions
int dotprod_rrrf_run(float *      _h,
                     float *      _x,
                     unsigned int _n,
                     float *      _y)
{
    float32x4_t v;   // input vector
    float32x4_t h;   // coefficients vector
    float32x4_t s;   // dot product

    // load zeros into sum register
    float zeros[4] = {0,0,0,0};
    float32x4_t sum = vld1q_f32(zeros);

    // t = 4*(floor(_n/4))
    unsigned int t = (_n >> 2) << 2;

    //
    unsigned int i;
    for (i=0; i<t; i+=4) {
        // load inputs into register (unaligned)
        v = vld1q_f32(&_x[i]);

        // load coefficients into register (aligned)
        h = vld1q_f32(&_h[i]);

        // compute multiplication
        s = vmulq_f32(h,v);
       
        // parallel addition
        sum = vaddq_f32(sum, s);
    }

    // unload packed array
    float w[4];
    vst1q_f32(w, sum);
    float total = w[0] + w[1] + w[2] + w[3];

    // cleanup
    for (; i<_n; i++)
        total += _x[i] * _h[i];

    // set return value
    *_y = total;
    return LIQUID_OK;
}

// basic dot product (ordinal calculation) with loop unrolled, neon extensions
int dotprod_rrrf_run4(float *      _h,
                      float *      _x,
                      unsigned int _n,
                      float *      _y)
{
    float32x4_t v0, v1, v2, v3;
    float32x4_t h0, h1, h2, h3;
    float32x4_t s0, s1, s2, s3;

    // load zeros into sum registers
    float zeros[4] = {0,0,0,0};
    float32x4_t sum0 = vld1q_f32(zeros);
    float32x4_t sum1 = vld1q_f32(zeros);
    float32x4_t sum2 = vld1q_f32(zeros);
    float32x4_t sum3 = vld1q_f32(zeros);

    // r = 4*floor(n/16)
    unsigned int r = (_n >> 4) << 2;

    //
    unsigned int i;
    for (i=0; i<r; i+=4) {
        // load inputs into register (unaligned)
        v0 = vld1q_f32(&_x[4*i+0]);
        v1 = vld1q_f32(&_x[4*i+4]);
        v2 = vld1q_f32(&_x[4*i+8]);
        v3 = vld1q_f32(&_x[4*i+12]);

        // load coefficients into register (aligned)
        h0 = vld1q_f32(&_h[4*i+0]);
        h1 = vld1q_f32(&_h[4*i+4]);
        h2 = vld1q_f32(&_h[4*i+8]);
        h3 = vld1q_f32(&_h[4*i+12]);

        // compute multiplication
        s0 = vmulq_f32(v0, h0);
        s1 = vmulq_f32(v1, h1);
        s2 = vmulq_f32(v2, h2);
        s3 = vmulq_f32(v3, h3);
        
        // parallel addition
        sum0 = vaddq_f32( sum0, s0 );
        sum1 = vaddq_f32( sum1, s1 );
        sum2 = vaddq_f32( sum2, s2 );
        sum3 = vaddq_f32( sum3, s3 );
    }

    // fold down into single 4-element register
    sum0 = vaddq_f32( sum0, sum1 );
    sum2 = vaddq_f32( sum2, sum3 );
    sum0 = vaddq_f32( sum0, sum2);

    // aligned output array
    float w[4] __attribute__((aligned(16)));

    // unload packed array and perform manual sum
    vst1q_f32(w, sum0);
    float total = w[0] + w[1] + w[2] + w[3];

    // cleanup
    // TODO : use intrinsics here as well
    for (i=4*r; i<_n; i++)
        total += _x[i] * _h[i];

    // set return value
    *_y = total;
    return LIQUID_OK;
}


//
// structured dot product
//

struct dotprod_rrrf_s {
    unsigned int n;     // length
    float * h;          // coefficients array
};

// create dotprod object
dotprod_rrrf dotprod_rrrf_create_opt(float *      _h,
                                     unsigned int _n,
                                     int          _rev)
{
    dotprod_rrrf q = (dotprod_rrrf)malloc(sizeof(struct dotprod_rrrf_s));
    q->n = _n;

    // allocate memory for coefficients
    q->h = (float*) malloc( q->n*sizeof(float) );

    // copy coefficients
    unsigned int i;
    for (i=0; i<q->n; i++)
        q->h[i] = _h[_rev ? q->n-i-1 : i];

    // return object
    return q;
}

dotprod_rrrf dotprod_rrrf_create(float *      _h,
                                 unsigned int _n)
{
    return dotprod_rrrf_create_opt(_h,_n,0);
}

dotprod_rrrf dotprod_rrrf_create_rev(float *      _h,
                                     unsigned int _n)
{
    return dotprod_rrrf_create_opt(_h,_n,1);
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

// re-create the structured dotprod object, reversing coefficients
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
        return liquid_error_config("dotprod_rrrf_copy().neon, object cannot be NULL");

    dotprod_rrrf q_copy = (dotprod_rrrf)malloc(sizeof(struct dotprod_rrrf_s));
    q_copy->n = q_orig->n;

    // allocate memory for coefficients
    q_copy->h = (float*) malloc( q_copy->n*sizeof(float) );

    // copy coefficients array
    memmove(q_copy->h, q_orig->h, q_orig->n*sizeof(float));

    // return object
    return q_copy;
}

// destroy dotprod object, freeing internal memory
int dotprod_rrrf_destroy(dotprod_rrrf _q)
{
    free(_q->h);
    free(_q);
    return LIQUID_OK;
}

// print dotprod internal state
int dotprod_rrrf_print(dotprod_rrrf _q)
{
    printf("dotprod_rrrf [arm-neon, %u coefficients]\n", _q->n);
    unsigned int i;
    for (i=0; i<_q->n; i++)
        printf("%3u : %12.9f\n", i, _q->h[i]);
    return LIQUID_OK;
}

// execute dot product on input vector
int dotprod_rrrf_execute(dotprod_rrrf _q,
                         float *      _x,
                         float *      _y)
{
    // switch based on size
    if (_q->n < 16) {
        return dotprod_rrrf_run(_q->h, _x, _q->n, _y);
    }
    return dotprod_rrrf_run4(_q->h, _x, _q->n, _y);
}

