/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// fft_mixed_radix.c : definitions for mixed-radix transforms using
//                     the Cooley-Tukey algorithm
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.internal.h"

#define FFT_DEBUG_MIXED_RADIX 0

// create FFT plan for regular DFT
//  _nfft   :   FFT size
//  _x      :   input array [size: _nfft x 1]
//  _y      :   output array [size: _nfft x 1]
//  _dir    :   fft direction: {LIQUID_FFT_FORWARD, LIQUID_FFT_BACKWARD}
//  _method :   fft method
FFT(plan) FFT(_create_plan_mixed_radix)(unsigned int _nfft,
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
    q->method    = LIQUID_FFT_METHOD_MIXED_RADIX;

    q->execute   = FFT(_execute_mixed_radix);

    // find first 'prime' factor of _nfft
    unsigned int i;
    unsigned int Q = FFT(_estimate_mixed_radix)(_nfft);
    if (Q==0)
        return liquid_error_config("fft_create_plan_mixed_radix(), _nfft=%u is prime", _nfft);
    if ( (_nfft % Q) != 0 )
        return liquid_error_config("fft_create_plan_mixed_radix(), _nfft=%u is not divisible by Q=%u", _nfft, Q);

    // set mixed-radix data
    unsigned int P = q->nfft / Q;
    q->data.mixedradix.Q = Q;
    q->data.mixedradix.P = P;

    // allocate memory for buffers
    unsigned int t_len = Q > P ? Q : P;
    q->data.mixedradix.t0 = (TC *) FFT_MALLOC(t_len * sizeof(TC));
    q->data.mixedradix.t1 = (TC *) FFT_MALLOC(t_len * sizeof(TC));

    // allocate memory for input buffers
    q->data.mixedradix.x = (TC *) malloc(q->nfft * sizeof(TC));

    // create P-point FFT plan
    q->data.mixedradix.fft_P = FFT(_create_plan)(q->data.mixedradix.P,
                                                 q->data.mixedradix.t0,
                                                 q->data.mixedradix.t1,
                                                 q->direction,
                                                 q->flags);

    // create Q-point FFT plan
    q->data.mixedradix.fft_Q = FFT(_create_plan)(q->data.mixedradix.Q,
                                                 q->data.mixedradix.t0,
                                                 q->data.mixedradix.t1,
                                                 q->direction,
                                                 q->flags);

    // initialize twiddle factors, indices for mixed-radix transforms
    // TODO : only allocate necessary twiddle factors
    q->data.mixedradix.twiddle = (TC *) malloc(q->nfft * sizeof(TC));
    
    T d = (q->direction == LIQUID_FFT_FORWARD) ? -1.0 : 1.0;
    for (i=0; i<q->nfft; i++)
        q->data.mixedradix.twiddle[i] = cexpf(_Complex_I*d*2*M_PI*(T)i / (T)(q->nfft));

    return q;
}

// destroy FFT plan
int FFT(_destroy_plan_mixed_radix)(FFT(plan) _q)
{
    // destroy sub-plans
    FFT(_destroy_plan)(_q->data.mixedradix.fft_P);
    FFT(_destroy_plan)(_q->data.mixedradix.fft_Q);

    // free data specific to mixed-radix transforms
    FFT_FREE(_q->data.mixedradix.t0);
    FFT_FREE(_q->data.mixedradix.t1);
    free(_q->data.mixedradix.x);
    free(_q->data.mixedradix.twiddle);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// execute mixed-radix FFT
int FFT(_execute_mixed_radix)(FFT(plan) _q)
{
    // set internal constants
    unsigned int P = _q->data.mixedradix.P; // first FFT size
    unsigned int Q = _q->data.mixedradix.Q; // second FFT size

    // set pointers
    TC * t0      = _q->data.mixedradix.t0;  // small FFT input buffer
    TC * t1      = _q->data.mixedradix.t1;  // small FFT output buffer
    TC * x       = _q->data.mixedradix.x;   // full input buffer (data copied)
    TC * twiddle = _q->data.mixedradix.twiddle; // twiddle factors

    // copy input to internal buffer
    memmove(x, _q->x, _q->nfft*sizeof(TC));

    unsigned int i;
    unsigned int k;

    // compute 'Q' DFTs of size 'P'
#if FFT_DEBUG_MIXED_RADIX
    printf("computing %u DFTs of size %u\n", Q, P);
#endif
    for (i=0; i<Q; i++) {
        // copy to temporary buffer
        for (k=0; k<P; k++)
            t0[k] = x[Q*k+i];

        // run internal P-point DFT
        FFT(_execute)(_q->data.mixedradix.fft_P);

        // copy back to input, applying twiddle factors
        for (k=0; k<P; k++)
            x[Q*k+i] = t1[k] * twiddle[i*k];

#if FFT_DEBUG_MIXED_RADIX
        printf("i=%3u/%3u\n", i, Q);
        for (k=0; k<P; k++)
            printf("  %12.6f %12.6f\n", crealf(x[Q*k+i]), cimagf(x[Q*k+i]));
#endif
    }

    // compute 'P' DFTs of size 'Q' and transpose
#if DEBUG
    printf("computing %u DFTs of size %u\n", P, Q);
#endif
    for (i=0; i<P; i++) {
        // copy to temporary buffer
        for (k=0; k<Q; k++)
            t0[k] = x[Q*i+k];

        // run internal Q-point DFT
        FFT(_execute)(_q->data.mixedradix.fft_Q);

        // copy and transpose
        for (k=0; k<Q; k++)
            _q->y[k*P+i] = t1[k];
        
#if DEBUG
        printf("i=%3u/%3u\n", i, P);
        for (k=0; k<Q; k++)
            printf("  %12.6f %12.6f\n", crealf(_q->y[k*P+i]), cimagf(_q->y[k*P+i]));
#endif
    }
    return LIQUID_OK;
}

// strategize as to best radix to use
unsigned int FFT(_estimate_mixed_radix)(unsigned int _nfft)
{
    // compute factors of _nfft
    unsigned int factors[LIQUID_MAX_FACTORS];
    unsigned int num_factors;
    liquid_factor(_nfft, factors, &num_factors);

    // check if _nfft is prime
    if (num_factors < 2) {
        liquid_error(LIQUID_EICONFIG,"fft_estimate_mixed_radix(), %u is prime", _nfft);
        return 0;
    }

    // if _nfft has many factors of 2, retain for later in favor of
    // radix2 sub-fft method
    unsigned int num_factors_2 = 0;
    unsigned int i;
    for (i=0; i<num_factors; i++) {
        if (factors[i] != 2)
            break;
    }
    num_factors_2 = i;
    //printf("nfft: %u / 2^%u = %u\n", _nfft, num_factors_2, _nfft / (1<<num_factors_2));

    // prefer aggregate radix-2 form if possible
    if (num_factors_2 > 0) {
#if 0
        // check if there are _only_ factors of 2
        if (num_factors_2 == num_factors) {
            // return Q = 2^(ceil(num_factors_2 / 2))
            // example: nfft = 128 = 2^7, return Q=2^4 = 16
            return 1 << ((num_factors_2 + (num_factors_2%2))/2);
        }

        // return 2^num_factors_2
        return 1 << num_factors_2;
#else
        // use codelets
        if      ( (_nfft%16)==0 ) return 16;
        if      ( (_nfft% 8)==0 ) return  8;
        else if ( (_nfft% 4)==0 ) return  4;
        else                      return  2;
#endif
    }

    // return next largest prime factor
    return factors[0];
}

