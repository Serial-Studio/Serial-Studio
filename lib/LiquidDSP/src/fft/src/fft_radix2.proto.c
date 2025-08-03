/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
// fft_radix2.c : definitions for transforms of the form 2^m
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.internal.h"

// create FFT plan for regular DFT
//  _nfft   :   FFT size
//  _x      :   input array [size: _nfft x 1]
//  _y      :   output array [size: _nfft x 1]
//  _dir    :   fft direction: {LIQUID_FFT_FORWARD, LIQUID_FFT_BACKWARD}
//  _method :   fft method
FFT(plan) FFT(_create_plan_radix2)(unsigned int _nfft,
                                   TC *         _x,
                                   TC *         _y,
                                   int          _dir,
                                   int          _flags)
{
    // allocate plan and initialize all internal arrays to NULL
    FFT(plan) q = (FFT(plan)) malloc(sizeof(struct FFT(plan_s)));

    q->nfft      = _nfft;
    q->x         = _x;
    q->y         = _y;
    q->flags     = _flags;
    q->type      = (_dir == LIQUID_FFT_FORWARD) ? LIQUID_FFT_FORWARD : LIQUID_FFT_BACKWARD;
    q->direction = (_dir == LIQUID_FFT_FORWARD) ? LIQUID_FFT_FORWARD : LIQUID_FFT_BACKWARD;
    q->method    = LIQUID_FFT_METHOD_RADIX2;

    q->execute   = FFT(_execute_radix2);

    // initialize twiddle factors, indices for radix-2 transforms
    q->data.radix2.m = liquid_msb_index(q->nfft) - 1;  // m = log2(nfft)
    
    q->data.radix2.index_rev = (unsigned int *) malloc((q->nfft)*sizeof(unsigned int));
    unsigned int i;
    for (i=0; i<q->nfft; i++)
        q->data.radix2.index_rev[i] = fft_reverse_index(i,q->data.radix2.m);

    // initialize twiddle factors
    q->data.radix2.twiddle = (TC *) malloc(q->nfft * sizeof(TC));
    
    T d = (q->direction == LIQUID_FFT_FORWARD) ? -1.0 : 1.0;
    for (i=0; i<q->nfft; i++)
        q->data.radix2.twiddle[i] = cexpf(_Complex_I*d*2*M_PI*(T)i / (T)(q->nfft));

    return q;
}

// destroy FFT plan
int FFT(_destroy_plan_radix2)(FFT(plan) _q)
{
    // free data specific to radix-2 transforms
    free(_q->data.radix2.index_rev);
    free(_q->data.radix2.twiddle);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// execute radix-2 FFT
int FFT(_execute_radix2)(FFT(plan) _q)
{
    // swap values
    unsigned int i,j,k;

    // unroll loop
    unsigned int nfft4 = (_q->nfft>>2)<<2;  // floor(_nfft/4)
    for (i=0; i<nfft4; i+=4) {
        _q->y[i  ] = _q->x[ _q->data.radix2.index_rev[i  ] ];
        _q->y[i+1] = _q->x[ _q->data.radix2.index_rev[i+1] ];
        _q->y[i+2] = _q->x[ _q->data.radix2.index_rev[i+2] ];
        _q->y[i+3] = _q->x[ _q->data.radix2.index_rev[i+3] ];
    }

#if 0
    // clean up remaining
    // NOTE : this only happens when _nfft=2 because we know (_nfft%4)==0 otherwise
    for ( ; i<_q->nfft; i++)
        _q->y[i] = _q->x[ _q->data.radix2.index_rev[i] ];
#endif

    TC yp;
    TC *y=_q->y;
    unsigned int n1 = 0;
    unsigned int n2 = 1;

    TC t;   // twiddle value
    unsigned int stride = _q->nfft;
    unsigned int twiddle_index;

    for (i=0; i<_q->data.radix2.m; i++) {
        n1 = n2;
        n2 *= 2;
        stride >>= 1;

        twiddle_index = 0;
    
        for (j=0; j<n1; j++) {
            t = _q->data.radix2.twiddle[twiddle_index];
            twiddle_index = (twiddle_index + stride) % _q->nfft;

            for (k=j; k<_q->nfft; k+=n2) {
                // NOTE: most computation is with the multiplication in next line
                yp      =  y[k+n1]*t;
                y[k+n1] =  y[k] - yp;
                y[k]    += yp;
            }
        }
    }
    return LIQUID_OK;
}

