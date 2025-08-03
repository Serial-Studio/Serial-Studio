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
// DCT : Discrete Cosine Transforms
//

#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

// DCT-I
void FFT(_execute_REDFT00)(FFT(plan) _q)
{
    // ugly, slow method
    unsigned int i,k;
    float n_inv = 1.0f / (float)(_q->nfft-1);
    float phi;
    for (i=0; i<_q->nfft; i++) {
        T x0 = _q->xr[0];       // first element
        T xn = _q->xr[_q->nfft-1]; // last element
        _q->yr[i] = 0.5f*( x0 + (i%2 ? -xn : xn));
        for (k=1; k<_q->nfft-1; k++) {
            phi = M_PI*n_inv*((float)k)*((float)i);
            _q->yr[i] += _q->xr[k]*cosf(phi);
        }

        // compensate for discrepancy
        _q->yr[i] *= 2.0f;
    }
}

// DCT-II (regular 'dct')
void FFT(_execute_REDFT10)(FFT(plan) _q)
{
    // ugly, slow method
    unsigned int i,k;
    float n_inv = 1.0f / (float)_q->nfft;
    float phi;
    for (i=0; i<_q->nfft; i++) {
        _q->yr[i] = 0.0f;
        for (k=0; k<_q->nfft; k++) {
            phi = M_PI*n_inv*((float)k + 0.5f)*i;
            _q->yr[i] += _q->xr[k]*cosf(phi);
        }

        // compensate for discrepancy
        _q->yr[i] *= 2.0f;
    }
}

// DCT-III (regular 'idct')
void FFT(_execute_REDFT01)(FFT(plan) _q)
{
    // ugly, slow method
    unsigned int i,k;
    float n_inv = 1.0f / (float)_q->nfft;
    float phi;
    for (i=0; i<_q->nfft; i++) {
        _q->yr[i] = _q->xr[0]*0.5f;
        for (k=1; k<_q->nfft; k++) {
            phi = M_PI*n_inv*((float)i + 0.5f)*k;
            _q->yr[i] += _q->xr[k]*cosf(phi);
        }

        // compensate for discrepancy
        _q->yr[i] *= 2.0f;
    }
}

// DCT-IV
void FFT(_execute_REDFT11)(FFT(plan) _q)
{
    // ugly, slow method
    unsigned int i,k;
    float n_inv = 1.0f / (float)(_q->nfft);
    float phi;
    for (i=0; i<_q->nfft; i++) {
        _q->yr[i] = 0.0f;
        for (k=0; k<_q->nfft; k++) {
            phi = M_PI*n_inv*((float)k+0.5f)*((float)i+0.5f);
            _q->yr[i] += _q->xr[k]*cosf(phi);
        }

        // compensate for discrepancy
        _q->yr[i] *= 2.0f;
    }
}
