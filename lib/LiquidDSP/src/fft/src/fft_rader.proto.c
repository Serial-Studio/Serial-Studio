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
// fft_rader.c : definitions for transforms of prime length using
//               Rader's algorithm
//
// References:
//  [Rader:1968] Charles M. Rader, "Discrete Fourier Transforms When
//      the Number of Data Samples Is Prime," Proceedings of the IEEE,
//      vol. 56, number 6, pp. 1107--1108, June 1968
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.internal.h"

#define FFT_DEBUG_RADER 0

// create FFT plan for regular DFT
//  _nfft   :   FFT size
//  _x      :   input array [size: _nfft x 1]
//  _y      :   output array [size: _nfft x 1]
//  _dir    :   fft direction: {LIQUID_FFT_FORWARD, LIQUID_FFT_BACKWARD}
//  _method :   fft method
FFT(plan) FFT(_create_plan_rader)(unsigned int _nfft,
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
    q->method    = LIQUID_FFT_METHOD_RADER;

    q->execute   = FFT(_execute_rader);

    // allocate memory for sub-transforms
    q->data.rader.x_prime = (TC*) FFT_MALLOC((q->nfft-1)*sizeof(TC));
    q->data.rader.X_prime = (TC*) FFT_MALLOC((q->nfft-1)*sizeof(TC));

    // create sub-FFT of size nfft-1
    q->data.rader.fft = FFT(_create_plan)(q->nfft-1,
                                          q->data.rader.x_prime,
                                          q->data.rader.X_prime,
                                          LIQUID_FFT_FORWARD,
                                          q->flags);

    // create sub-IFFT of size nfft-1
    q->data.rader.ifft = FFT(_create_plan)(q->nfft-1,
                                           q->data.rader.X_prime,
                                           q->data.rader.x_prime,
                                           LIQUID_FFT_BACKWARD,
                                           q->flags);

    // compute primitive root of nfft
    unsigned int g = liquid_primitive_root_prime(q->nfft);

    // create and initialize sequence
    q->data.rader.seq = (unsigned int *)malloc((q->nfft-1)*sizeof(unsigned int));
    unsigned int i;
    for (i=0; i<q->nfft-1; i++)
        q->data.rader.seq[i] = liquid_modpow(g, i+1, q->nfft);
    
    // compute DFT of sequence { exp(-j*2*pi*g^i/nfft }, size: nfft-1
    // NOTE: R[0] = -1, |R[k]| = sqrt(nfft) for k != 0
    // (use newly-created FFT plan of length nfft-1)
    T d = (q->direction == LIQUID_FFT_FORWARD) ? -1.0 : 1.0;
    for (i=0; i<q->nfft-1; i++)
        q->data.rader.x_prime[i] = cexpf(_Complex_I*d*2*M_PI*q->data.rader.seq[i]/(T)(q->nfft));
    FFT(_execute)(q->data.rader.fft);

    // copy result to R
    q->data.rader.R = (TC*)malloc((q->nfft-1)*sizeof(TC));
    memmove(q->data.rader.R, q->data.rader.X_prime, (q->nfft-1)*sizeof(TC));
    
    // return main object
    return q;
}

// destroy FFT plan
int FFT(_destroy_plan_rader)(FFT(plan) _q)
{
    // free data specific to Rader's algorithm
    free(_q->data.rader.seq);           // sequence
    free(_q->data.rader.R);             // pre-computed transform of exp(j*2*pi*seq)
    FFT_FREE(_q->data.rader.x_prime);   // sub-transform input array
    FFT_FREE(_q->data.rader.X_prime);   // sub-transform output array

    FFT(_destroy_plan)(_q->data.rader.fft);
    FFT(_destroy_plan)(_q->data.rader.ifft);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// execute Rader's algorithm
int FFT(_execute_rader)(FFT(plan) _q)
{
    unsigned int i;

    // compute DFT of permuted sequence, size: nfft-1
    for (i=0; i<_q->nfft-1; i++) {
        // reverse sequence
        unsigned int k = _q->data.rader.seq[_q->nfft-1-i-1];
        _q->data.rader.x_prime[i] = _q->x[k];
    }
    // compute sub-FFT
    // equivalent to: FFT(_run)(_q->nfft-1, xp, Xp, LIQUID_FFT_FORWARD, 0);
    FFT(_execute)(_q->data.rader.fft);

    // compute inverse FFT of product
    for (i=0; i<_q->nfft-1; i++)
        _q->data.rader.X_prime[i] *= _q->data.rader.R[i];

    // compute sub-IFFT
    // equivalent to: FFT(_run)(_q->nfft-1, Xp, xp, LIQUID_FFT_BACKWARD, 0);
    FFT(_execute)(_q->data.rader.ifft);

    // set DC value
    _q->y[0] = 0.0f;
    for (i=0; i<_q->nfft; i++)
        _q->y[0] += _q->x[i];

    // reverse permute result, scale, and add offset x[0]
    for (i=0; i<_q->nfft-1; i++) {
        unsigned int k = _q->data.rader.seq[i];

        _q->y[k] = _q->data.rader.x_prime[i] / (T)(_q->nfft-1) + _q->x[0];
    }
    return LIQUID_OK;
}

