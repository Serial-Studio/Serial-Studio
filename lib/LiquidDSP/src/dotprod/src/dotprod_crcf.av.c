/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
// Complex floating-point dot product (altivec velocity engine)
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

#define DEBUG_DOTPROD_CRCF_AV   0

// basic dot product

// basic dot product
//  _h      :   coefficients array [size: 1 x _n]
//  _x      :   input array [size: 1 x _n]
//  _n      :   input lengths
//  _y      :   output dot product
int dotprod_crcf_run(float *         _h,
                     float complex * _x,
                     unsigned int    _n,
                     float complex * _y)
{
    float complex r=0;
    unsigned int i;
    for (i=0; i<_n; i++)
        r += _h[i] * _x[i];
    *_y = r;
    return LIQUID_OK;
}

// basic dot product, unrolling loop
//  _h      :   coefficients array [size: 1 x _n]
//  _x      :   input array [size: 1 x _n]
//  _n      :   input lengths
//  _y      :   output dot product
int dotprod_crcf_run4(float *         _h,
                      float complex * _x,
                      unsigned int    _n,
                      float complex * _y)
{
    float complex r=0;

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
// structured dot product
//

struct dotprod_crcf_s {
    // dotprod length (number of coefficients)
    unsigned int n;

    // coefficients arrays: the altivec velocity engine operates
    // on 128-bit registers which can hold four 32-bit floating-
    // point values.  We need to hold 4 copies of the coefficients
    // to meet all possible alignments to the input data.
    float *h[4];
};

// create the structured dotprod object
dotprod_crcf dotprod_crcf_create_opt(float *      _h,
                                     unsigned int _n,
                                     int          _rev)
{
    dotprod_crcf q = (dotprod_crcf)malloc(sizeof(struct dotprod_crcf_s));
    q->n = _n;

    // create 4 copies of the input coefficients (one for each
    // data alignment).  For example: _h[4] = {1,2,3,4,5,6}
    //  q->h[0] = {1,1,2,2,3,3,4,4,5,5,6,6}
    //  q->h[1] = {. 1,1,2,2,3,3,4,4,5,5,6,6}
    //  q->h[2] = {. . 1,1,2,2,3,3,4,4,5,5,6,6}
    //  q->h[3] = {. . . 1,1,2,2,3,3,4,4,5,5,6,6}
    // NOTE: double allocation size; coefficients are real, but
    //       need to be multiplied by real and complex components
    //       of input.
    unsigned int i,j;
    for (i=0; i<4; i++) {
        q->h[i] = calloc(1+(2*q->n+i-1)/4,2*sizeof(vector float));
        for (j=0; j<q->n; j++) {
            q->h[i][2*j+0+i] = _h[_rev ? q->n-j-1 : j];
            q->h[i][2*j+1+i] = _h[_rev ? q->n-j-1 : j];
        }
    }

    return q;
}

dotprod_crcf dotprod_crcf_create(float *      _h,
                                 unsigned int _n)
{
    return dotprod_crcf_create_opt(_h,_n,0);
}

dotprod_crcf dotprod_crcf_create_rev(float *      _h,
                                     unsigned int _n)
{
    return dotprod_crcf_create_opt(_h,_n,1);
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

// re-create the structured dotprod object
dotprod_crcf dotprod_crcf_recreate_rev(dotprod_crcf _q,
                                       float *      _h,
                                       unsigned int _n)
{
    // completely destroy and re-create dotprod object
    dotprod_crcf_destroy(_q);
    return dotprod_crcf_create_rev(_h,_n);
}

// destroy the structured dotprod object
int dotprod_crcf_destroy(dotprod_crcf _q)
{
    // clean up coefficients arrays
    unsigned int i;
    for (i=0; i<4; i++)
        free(_q->h[i]);

    // free allocated object memory
    free(_q);
    return LIQUID_OK;
}

// print the dotprod object
int dotprod_crcf_print(dotprod_crcf _q)
{
    printf("dotprod_crcf [altivec, %u coefficients]:\n", _q->n);
    unsigned int i;
    for (i=0; i<_q->n; i++)
        printf("  %3u : %12.9f\n", i, _q->h[0][2*i]);
    return LIQUID_OK;
}

// exectue vectorized structured inner dot product
int dotprod_crcf_execute(dotprod_crcf    _q,
                         float complex * _x,
                         float complex * _r)
{
    int al; // input data alignment

    vector float *ar,*d;
    vector float s0,s1,s2,s3;
    union { vector float v; float w[4];} s;
    unsigned int nblocks;

    ar = (vector float*)( (int)_x & ~15);
    al = ((int)_x & 15)/sizeof(float);

    d = (vector float*)_q->h[al];

    // number of blocks doubles because of complex type
    nblocks = (2*_q->n + al - 1)/4 + 1;

    // split into four vectors each with four 32-bit
    // partial sums.  Effectively each loop iteration
    // operates on 16 input samples at a time.
    s0 = s1 = s2 = s3 = (vector float)(0);
    while (nblocks >= 4) {
        s0 = vec_madd(ar[nblocks-1],d[nblocks-1],s0);
        s1 = vec_madd(ar[nblocks-2],d[nblocks-2],s1);
        s2 = vec_madd(ar[nblocks-3],d[nblocks-3],s2);
        s3 = vec_madd(ar[nblocks-4],d[nblocks-4],s3);
        nblocks -= 4;
    }

    // fold the resulting partial sums into vector s0
    s0 = vec_add(s0,s1);    // s0 = s0+s1
    s2 = vec_add(s2,s3);    // s2 = s2+s3
    s0 = vec_add(s0,s2);    // s0 = s0+s2

    // finish partial summing operations
    while (nblocks-- > 0)
        s0 = vec_madd(ar[nblocks],d[nblocks],s0);

    // move the result into the union s (effetively,
    // this loads the four 32-bit values in s0 into
    // the array w).
    s.v = vec_add(s0,(vector float)(0));

    // sum the resulting array
    //*_r = s.w[0] + s.w[1] + s.w[2] + s.w[3];
    *_r = (s.w[0] + s.w[2]) + (s.w[1] + s.w[3]) * _Complex_I;
    return LIQUID_OK;
}

