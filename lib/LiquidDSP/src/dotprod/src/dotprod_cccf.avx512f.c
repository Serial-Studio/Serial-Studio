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
// Floating-point dot product (AVX512-F)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

// include proper SIMD extensions for x86 platforms
#include <immintrin.h>  // AVX

#define DEBUG_DOTPROD_CCCF_AVX   0

// forward declaration of internal methods
int dotprod_cccf_execute_avx512f(dotprod_cccf    _q,
                             float complex * _x,
                             float complex * _y);

int dotprod_cccf_execute_avx512f4(dotprod_cccf    _q,
                              float complex * _x,
                              float complex * _y);

// basic dot product (ordinal calculation)
int dotprod_cccf_run(float complex * _h,
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
int dotprod_cccf_run4(float complex * _h,
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
// structured AVX512-F dot product
//

struct dotprod_cccf_s {
    unsigned int n;     // length
    float * hi;         // in-phase
    float * hq;         // quadrature
};

dotprod_cccf dotprod_cccf_create_opt(float complex * _h,
                                     unsigned int    _n,
                                     int             _rev)
{
    dotprod_cccf q = (dotprod_cccf)malloc(sizeof(struct dotprod_cccf_s));
    q->n = _n;

    // allocate memory for coefficients, 64-byte aligned
    q->hi = (float*) _mm_malloc( 2*q->n*sizeof(float), 64 );
    q->hq = (float*) _mm_malloc( 2*q->n*sizeof(float), 64 );

    // set coefficients, repeated
    //  hi = { crealf(_h[0]), crealf(_h[0]), ... crealf(_h[n-1]), crealf(_h[n-1])}
    //  hq = { cimagf(_h[0]), cimagf(_h[0]), ... cimagf(_h[n-1]), cimagf(_h[n-1])}
    unsigned int i;
    for (i=0; i<q->n; i++) {
        unsigned int k = _rev ? q->n-i-1 : i;
        q->hi[2*i+0] = crealf(_h[k]);
        q->hi[2*i+1] = crealf(_h[k]);

        q->hq[2*i+0] = cimagf(_h[k]);
        q->hq[2*i+1] = cimagf(_h[k]);
    }

    // return object
    return q;
}

dotprod_cccf dotprod_cccf_create(float complex * _h,
                                 unsigned int    _n)
{
    return dotprod_cccf_create_opt(_h, _n, 0);
}

dotprod_cccf dotprod_cccf_create_rev(float complex * _h,
                                     unsigned int    _n)
{
    return dotprod_cccf_create_opt(_h, _n, 1);
}

// re-create the structured dotprod object
dotprod_cccf dotprod_cccf_recreate(dotprod_cccf    _q,
                                   float complex * _h,
                                   unsigned int    _n)
{
    // completely destroy and re-create dotprod object
    dotprod_cccf_destroy(_q);
    return dotprod_cccf_create(_h,_n);
}

// re-create the structured dotprod object, coefficients reversed
dotprod_cccf dotprod_cccf_recreate_rev(dotprod_cccf    _q,
                                       float complex * _h,
                                       unsigned int    _n)
{
    // completely destroy and re-create dotprod object
    dotprod_cccf_destroy(_q);
    return dotprod_cccf_create_rev(_h,_n);
}

dotprod_cccf dotprod_cccf_copy(dotprod_cccf q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("dotprod_cccf_copy().avx512f, object cannot be NULL");

    dotprod_cccf q_copy = (dotprod_cccf)malloc(sizeof(struct dotprod_cccf_s));
    q_copy->n = q_orig->n;

    // allocate memory for coefficients, 64-byte aligned (repeated)
    q_copy->hi = (float*) _mm_malloc( 2*q_copy->n*sizeof(float), 64 );
    q_copy->hq = (float*) _mm_malloc( 2*q_copy->n*sizeof(float), 64 );

    // copy coefficients array (repeated)
    //  hi = { crealf(_h[0]), crealf(_h[0]), ... crealf(_h[n-1]), crealf(_h[n-1])}
    //  hq = { cimagf(_h[0]), cimagf(_h[0]), ... cimagf(_h[n-1]), cimagf(_h[n-1])}
    memmove(q_copy->hi, q_orig->hi, 2*q_orig->n*sizeof(float));
    memmove(q_copy->hq, q_orig->hq, 2*q_orig->n*sizeof(float));

    // return object
    return q_copy;
}

int dotprod_cccf_destroy(dotprod_cccf _q)
{
    _mm_free(_q->hi);
    _mm_free(_q->hq);
    free(_q);
    return LIQUID_OK;
}

int dotprod_cccf_print(dotprod_cccf _q)
{
    printf("dotprod_cccf [avx512f, %u coefficients]\n", _q->n);
    unsigned int i;
    for (i=0; i<_q->n; i++)
        printf("  %3u : %12.9f +j%12.9f\n", i, _q->hi[i], _q->hq[i]);
    return LIQUID_OK;
}

// execute structured dot product
//  _q      :   dotprod object
//  _x      :   input array
//  _y      :   output sample
int dotprod_cccf_execute(dotprod_cccf    _q,
                         float complex * _x,
                         float complex * _y)
{
    // switch based on size
    if (_q->n < 128) {
        return dotprod_cccf_execute_avx512f(_q, _x, _y);
    }
    return dotprod_cccf_execute_avx512f4(_q, _x, _y);
}

// use AVX512-F extensions
//
// (a + jb)(c + jd) = (ac - bd) + j(ad + bc)
//
// mm_x  = { x[0].real, x[0].imag, x[1].real, x[1].imag, x[2].real, x[2].imag, x[3].real, x[3].imag }
// mm_hi = { h[0].real, h[0].real, h[1].real, h[1].real, h[2].real, h[2].real, h[3].real, h[3].real }
// mm_hq = { h[0].imag, h[0].imag, h[1].imag, h[1].imag, h[2].imag, h[2].imag, h[3].imag, h[3].imag }
//
// mm_y0 = mm_x * mm_hi
//       = { x[0].real * h[0].real,
//           x[0].imag * h[0].real,
//           x[1].real * h[1].real,
//           x[1].imag * h[1].real,
//           x[2].real * h[2].real,
//           x[2].imag * h[2].real,
//           x[3].real * h[3].real,
//           x[3].imag * h[3].real };
//
// mm_y1 = mm_x * mm_hq
//       = { x[0].real * h[0].imag,
//           x[0].imag * h[0].imag,
//           x[1].real * h[1].imag,
//           x[1].imag * h[1].imag,
//           x[2].real * h[2].imag,
//           x[2].imag * h[2].imag,
//           x[3].real * h[3].imag,
//           x[3].imag * h[3].imag };
//
int dotprod_cccf_execute_avx512f(dotprod_cccf    _q,
                             float complex * _x,
                             float complex * _y)
{
    // type cast input as floating point array
    float * x = (float*) _x;

    // double effective length
    unsigned int n = 2*_q->n;

    // temporary buffers
    __m512 v;   // input vector
    __m512 hi;  // coefficients vector (real)
    __m512 hq;  // coefficients vector (imag)
    __m512 ci;  // output multiplication (v * hi)
    __m512 cq;  // output multiplication (v * hq)

    __m512 s;   // dot product
    __m512 sum = _mm512_setzero_ps(); // load zeros into sum register
    __m512 one = _mm512_set1_ps(1.0f); // load ones into register

    // t = 16*(floor(_n/16))
    unsigned int t = (n >> 4) << 4;

    //
    unsigned int i;
    for (i=0; i<t; i+=16) {
        // load inputs into register (unaligned)
        // {x[0].real, x[0].imag, x[1].real, x[1].imag, x[2].real, x[2].imag, x[3].real, x[3].imag}
        v = _mm512_loadu_ps(&x[i]);

        // load coefficients into register (aligned)
        hi = _mm512_load_ps(&_q->hi[i]);
        hq = _mm512_load_ps(&_q->hq[i]);

        // compute parallel multiplications
        ci = _mm512_mul_ps(v, hi);
        cq = _mm512_mul_ps(v, hq);

        // shuffle values
        cq = _mm512_shuffle_ps( cq, cq, _MM_SHUFFLE(2,3,0,1) );

        // combine using addsub_ps()
        s = _mm512_fmaddsub_ps( ci, one, cq );

        // accumulate
        sum = _mm512_add_ps(sum, s);
    }

    // output array
    float w[2];

    // fold down I/Q components into single value
    w[0] = _mm512_mask_reduce_add_ps(0x5555, sum);
    w[1] = _mm512_mask_reduce_add_ps(0xAAAA, sum);

    //float complex total = *((float complex*)w);
    float complex total = w[0] + w[1] * _Complex_I;

    // cleanup
    for (i=t/2; i<_q->n; i++)
        total += _x[i] * ( _q->hi[2*i] + _q->hq[2*i]*_Complex_I );

    // set return value
    *_y = total;
    return LIQUID_OK;
}

// use AVX512-F extensions
int dotprod_cccf_execute_avx512f4(dotprod_cccf    _q,
                              float complex * _x,
                              float complex * _y)
{
    // type cast input as floating point array
    float * x = (float*) _x;

    // double effective length
    unsigned int n = 2*_q->n;

    // first cut: ...
    __m512 v0,  v1,  v2,  v3;   // input vectors
    __m512 hi0, hi1, hi2, hi3;  // coefficients vectors (real)
    __m512 hq0, hq1, hq2, hq3;  // coefficients vectors (imag)
    __m512 ci0, ci1, ci2, ci3;  // output multiplications (v * hi)
    __m512 cq0, cq1, cq2, cq3;  // output multiplications (v * hq)

    // load zeros into sum registers
    __m512 sumi = _mm512_setzero_ps();
    __m512 sumq = _mm512_setzero_ps();


    __m512 one = _mm512_set1_ps(1.0f); // load ones into register

    // r = 16*floor(n/64)
    unsigned int r = (n >> 6) << 4;

    //
    unsigned int i;
    for (i=0; i<r; i+=16) {
        // load inputs into register (unaligned)
        v0 = _mm512_loadu_ps(&x[4*i+0]);
        v1 = _mm512_loadu_ps(&x[4*i+16]);
        v2 = _mm512_loadu_ps(&x[4*i+32]);
        v3 = _mm512_loadu_ps(&x[4*i+48]);

        // load real coefficients into registers (aligned)
        hi0 = _mm512_load_ps(&_q->hi[4*i+0]);
        hi1 = _mm512_load_ps(&_q->hi[4*i+16]);
        hi2 = _mm512_load_ps(&_q->hi[4*i+32]);
        hi3 = _mm512_load_ps(&_q->hi[4*i+48]);

        // load real coefficients into registers (aligned)
        hq0 = _mm512_load_ps(&_q->hq[4*i+0]);
        hq1 = _mm512_load_ps(&_q->hq[4*i+16]);
        hq2 = _mm512_load_ps(&_q->hq[4*i+32]);
        hq3 = _mm512_load_ps(&_q->hq[4*i+48]);
        
        // compute parallel multiplications (real)
        ci0 = _mm512_mul_ps(v0, hi0);
        ci1 = _mm512_mul_ps(v1, hi1);
        ci2 = _mm512_mul_ps(v2, hi2);
        ci3 = _mm512_mul_ps(v3, hi3);

        // compute parallel multiplications (imag)
        cq0 = _mm512_mul_ps(v0, hq0);
        cq1 = _mm512_mul_ps(v1, hq1);
        cq2 = _mm512_mul_ps(v2, hq2);
        cq3 = _mm512_mul_ps(v3, hq3);

        // accumulate
        sumi = _mm512_add_ps(sumi, ci0);   sumq = _mm512_add_ps(sumq, cq0);
        sumi = _mm512_add_ps(sumi, ci1);   sumq = _mm512_add_ps(sumq, cq1);
        sumi = _mm512_add_ps(sumi, ci2);   sumq = _mm512_add_ps(sumq, cq2);
        sumi = _mm512_add_ps(sumi, ci3);   sumq = _mm512_add_ps(sumq, cq3);
    }

    // shuffle values
    sumq = _mm512_shuffle_ps( sumq, sumq, _MM_SHUFFLE(2,3,0,1) );

    // combine using addsub_ps()
    sumi = _mm512_fmaddsub_ps( sumi, one, sumq );

    // output array
    float w[2];

    // fold down I/Q components into single value
    w[0] = _mm512_mask_reduce_add_ps(0x5555, sumi);
    w[1] = _mm512_mask_reduce_add_ps(0xAAAA, sumi);

    float complex total = w[0] + w[1] * _Complex_I;

    // cleanup (note: n _must_ be even)
    // TODO : clean this method up
    for (i=2*r; i<_q->n; i++) {
        total += _x[i] * ( _q->hi[2*i] + _q->hq[2*i]*_Complex_I );
    }

    // set return value
    *_y = total;
    return LIQUID_OK;
}

