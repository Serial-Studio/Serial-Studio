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
// fft_rader2.c : definitions for transforms of prime length using
//                Rader's alternate algorithm
//
// References:
//  [Rader:1968] Charles M. Rader, "Discrete Fourier Transforms When
//      the Number of Data Samples Is Prime," Proceedings of the IEEE,
//      vol. 56, number 6, pp. 1107--1108, June 1968
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.internal.h"

#define FFT_DEBUG_RADER 0

// create FFT plan
//  _nfft   :   FFT size
//  _x      :   input array [size: _nfft x 1]
//  _y      :   output array [size: _nfft x 1]
//  _dir    :   fft direction: {LIQUID_FFT_FORWARD, LIQUID_FFT_BACKWARD}
//  _method :   fft method
FFT(plan) FFT(_create_plan_rader2)(unsigned int _nfft,
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
    q->method    = LIQUID_FFT_METHOD_RADER2;

    q->execute   = FFT(_execute_rader2);

    unsigned int i;

    // compute primitive root of nfft
    unsigned int g = liquid_primitive_root_prime(q->nfft);

    // create and initialize sequence
    q->data.rader2.seq = (unsigned int *)malloc((q->nfft-1)*sizeof(unsigned int));
    for (i=0; i<q->nfft-1; i++)
        q->data.rader2.seq[i] = liquid_modpow(g, i+1, q->nfft);

#if 0
    // compute larger FFT length greater than 2*nfft-4
    // NOTE: while any length greater than 2*nfft-4 will work, use
    //       nfft_prime as smallest 'simple' FFT (mostly small factors)
    //
    // TODO: devise better score (fewer factors is better)
    //       score(n) = n / sum(factors(n).^2)
    float gamma_max = 0.0f; // score
    unsigned int nfft_prime_opt = 0;
    unsigned int num_steps = 10;// + q->nfft;
    for (i=1; i<=num_steps; i++) {
        unsigned int n_hat = 2*q->nfft - 4 + i;

        // compute factors
        unsigned int k;
        unsigned int num_factors = 0;
        unsigned int m = n_hat;
        float gamma = 0.0f;
        do {
            for (k=2; k<=m; k++) {
                if ( (m % k) == 0) {
                    m /= k;
                    num_factors++;
                    gamma += k*k;
                    break;
                }
            }
        } while (m > 1);

        // compute score:
        //float gamma = (float)n_hat / (float)num_factors;
        //float gamma = 1e3f * (float)num_factors / (float)n_hat;
        gamma = (float)n_hat / gamma;

        if (gamma > gamma_max) {
            gamma_max = gamma;
            nfft_prime_opt = n_hat;
        }
    }
    q->data.rader2.nfft_prime = nfft_prime_opt;
#else
    // compute larger FFT length greater than 2*nfft-4
    // NOTE: while any length greater than 2*nfft-4 will work, use
    //       nfft_prime = 2 ^ nextpow2( 2*nfft - 4 ) to enable
    //       radix-2 transform
    unsigned int m=0;
    q->data.rader2.nfft_prime = (2*q->nfft-4)-1;
    while (q->data.rader2.nfft_prime > 0) {
        q->data.rader2.nfft_prime >>= 1;
        m++;
    }
    q->data.rader2.nfft_prime = 1 << m;
#endif
    //printf("nfft_prime = %u\n", q->data.rader2.nfft_prime);
    // assert(nfft_prime > 2*nfft-4)

    // allocate memory for sub-transforms
    q->data.rader2.x_prime = (TC*) FFT_MALLOC((q->data.rader2.nfft_prime)*sizeof(TC));
    q->data.rader2.X_prime = (TC*) FFT_MALLOC((q->data.rader2.nfft_prime)*sizeof(TC));

    // create sub-FFT of size nfft-1
    q->data.rader2.fft = FFT(_create_plan)(q->data.rader2.nfft_prime,
                                           q->data.rader2.x_prime,
                                           q->data.rader2.X_prime,
                                           LIQUID_FFT_FORWARD,
                                           q->flags);

    // create sub-IFFT of size nfft-1
    q->data.rader2.ifft = FFT(_create_plan)(q->data.rader2.nfft_prime,
                                            q->data.rader2.X_prime,
                                            q->data.rader2.x_prime,
                                            LIQUID_FFT_BACKWARD,
                                            q->flags);

    // compute DFT of sequence { exp(-j*2*pi*g^i/nfft }, size: nfft_prime
    // NOTE: R[0] = -1, |R[k]| = sqrt(nfft) for k != 0
    // (use newly-created FFT plan of length nfft_prime)
    T d = (q->direction == LIQUID_FFT_FORWARD) ? -1.0 : 1.0;
    for (i=0; i<q->data.rader2.nfft_prime; i++)
        q->data.rader2.x_prime[i] = cexpf(_Complex_I*d*2*M_PI*q->data.rader2.seq[i%(q->nfft-1)]/(T)(q->nfft));
    FFT(_execute)(q->data.rader2.fft);
    
    // copy result to R
    q->data.rader2.R = (TC*)malloc(q->data.rader2.nfft_prime*sizeof(TC));
    memmove(q->data.rader2.R, q->data.rader2.X_prime, q->data.rader2.nfft_prime*sizeof(TC));

    // return main object
    return q;
}

// destroy FFT plan
int FFT(_destroy_plan_rader2)(FFT(plan) _q)
{
    // free data specific to Rader's algorithm
    free(_q->data.rader2.seq);          // sequence
    free(_q->data.rader2.R);            // pre-computed transform of exp(j*2*pi*seq)

    FFT_FREE(_q->data.rader2.x_prime);  // sub-transform input array
    FFT_FREE(_q->data.rader2.X_prime);  // sub-transform output array

    FFT(_destroy_plan)(_q->data.rader2.fft);
    FFT(_destroy_plan)(_q->data.rader2.ifft);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// execute Rader's algorithm
int FFT(_execute_rader2)(FFT(plan) _q)
{
    unsigned int i;

    // set pointers to internal buffers
    TC * xp = _q->data.rader2.x_prime;
    TC * Xp = _q->data.rader2.X_prime;
    TC * R  = _q->data.rader2.R;
    unsigned int * seq = _q->data.rader2.seq;

    // set constant values
    unsigned int nfft_prime = _q->data.rader2.nfft_prime;

    // compute nfft_prime-length DFT of permuted sequence with
    // nfft_prime-nfft+1 zeros inserted after first element

    xp[0] = _q->x[ seq[_q->nfft-2] ];
    for (i=0; i<nfft_prime-_q->nfft+1; i++)
        xp[i+1] = 0.0f;
    for (i=1; i<_q->nfft-1; i++) {
        // reverse sequence
        unsigned int k = seq[_q->nfft-1-i-1];
        xp[i+nfft_prime-_q->nfft+1] = _q->x[k];
    }
    FFT(_execute)(_q->data.rader2.fft);

    // compute inverse FFT of product
    // compute nfft_prime-length inverse FFT of product
    for (i=0; i<nfft_prime; i++)
        Xp[i] *= R[i];

    // call radix-2 function (IFFT)
    FFT(_execute)(_q->data.rader2.ifft);

    // set DC value
    _q->y[0] = 0.0f;
    for (i=0; i<_q->nfft; i++)
        _q->y[0] += _q->x[i];

    // reverse permute result, scale, and add offset x[0]
    for (i=0; i<_q->nfft-1; i++) {
        unsigned int k = seq[i];

        _q->y[k] = xp[i] / (T)(nfft_prime) + _q->x[0];
    }
    return LIQUID_OK;
}

