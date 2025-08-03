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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"


// Compute group delay for a FIR filter
//  _h      : filter coefficients array [size: _n x 1]
//  _n      : filter length
//  _fc     : frequency at which delay is evaluated (-0.5 < _fc < 0.5)
float fir_group_delay(float *      _h,
                      unsigned int _n,
                      float        _fc)
{
    // validate input
    if (_n == 0) {
        liquid_error(LIQUID_EICONFIG,"fir_group_delay(), length must be greater than zero");
        return 0.0f;
    } else if (_fc < -0.5 || _fc > 0.5) {
        liquid_error(LIQUID_EICONFIG,"fir_group_delay(), _fc must be in [-0.5,0.5]");
        return 0.0f;
    }

    unsigned int i;
    float complex t0=0.0f;
    float complex t1=0.0f;
    for (i=0; i<_n; i++) {
        t0 += _h[i] * cexpf(_Complex_I*2*M_PI*_fc*i) * i;
        t1 += _h[i] * cexpf(_Complex_I*2*M_PI*_fc*i);
    }

    return crealf(t0/t1);
}

// Compute group delay for an IIR filter
//  _b      : filter coefficients array (numerator), [size: _nb x 1]
//  _nb     : filter length (numerator)
//  _a      : filter coefficients array (denominator), [size: _na x 1]
//  _na     : filter length (denominator)
//  _fc     : frequency at which delay is evaluated (-0.5 < _fc < 0.5)
float iir_group_delay(float *      _b,
                      unsigned int _nb,
                      float *      _a,
                      unsigned int _na,
                      float        _fc)
{
    // validate input
    if (_nb == 0) {
        liquid_error(LIQUID_EICONFIG,"iir_group_delay(), numerator length must be greater than zero");
        return 0.0f;
    } else if (_na == 0) {
        liquid_error(LIQUID_EICONFIG,"iir_group_delay(), denominator length must be greater than zero");
        return 0.0f;
    } else if (_fc < -0.5 || _fc > 0.5) {
        liquid_error(LIQUID_EICONFIG,"iir_group_delay(), _fc must be in [-0.5,0.5]");
        return 0.0f;
    }

    // compute c = conv(b,fliplr(a))
    //         c(z) = b(z)*a(1/z)*z^(-_na)
    unsigned int nc = _na + _nb - 1;
    float c[nc];
    unsigned int i,j;
    for (i=0; i<nc; i++)
        c[i] = 0.0;

    for (i=0; i<_na; i++) {
        for (j=0; j<_nb; j++) {
            float sum = conjf(_a[_na-i-1])*_b[j];
            c[i+j] += sum;
        }
    }

    // compute 
    //      sum(c[i] * exp(j 2 pi fc i) * i)
    //      --------------------------------
    //      sum(c[i] * exp(j 2 pi fc i))
    float complex t0=0.0f;
    float complex t1=0.0f;
    float complex c0;
    for (i=0; i<nc; i++) {
        c0  = c[i] * cexpf(_Complex_I*2*M_PI*_fc*i);
        t0 += c0*i;
        t1 += c0;
    }

    // prevent divide-by-zero (check magnitude for tolerance range)
    float tol = 1e-5f;
    if (cabsf(t1)<tol)
        return 0.0f;

    // return result, scaled by length of denominator
    return crealf(t0/t1) - (_na - 1);
}

