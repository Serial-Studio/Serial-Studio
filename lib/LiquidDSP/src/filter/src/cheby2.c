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
// Chebeshev type-II filter design
//

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "liquid.internal.h"

#define LIQUID_DEBUG_CHEBY2_PRINT   0

// Compute analog zeros, poles, gain of low-pass Chebyshev
// Type II filter, grouping complex conjugates together. If
// the filter order is odd, the single real pole is at the
// end of the array.
//  _n      :   filter order
//  _ep     :   epsilon, related to stop-band ripple
//  _za     :   output analog zeros [length: 2*floor(_n/2)]
//  _pa     :   output analog poles [length: _n]
//  _ka     :   output analog gain
int cheby2_azpkf(unsigned int           _n,
                 float                  _es,
                 liquid_float_complex * _za,
                 liquid_float_complex * _pa,
                 liquid_float_complex * _ka)
{
    // temporary values
    float t0 = sqrt(1.0 + 1.0/(_es*_es));
    float tp = powf( t0 + 1.0/_es, 1.0/(float)(_n) );
    float tm = powf( t0 - 1.0/_es, 1.0/(float)(_n) );

    float b = 0.5*(tp + tm);    // ellipse major axis
    float a = 0.5*(tp - tm);    // ellipse minor axis

#if LIQUID_DEBUG_CHEBY2_PRINT
    printf("ep : %12.8f\n", _es);
    printf("b  : %12.8f\n", b);
    printf("a  : %12.8f\n", a);
#endif

    // filter order variables
    unsigned int r = _n % 2;        // odd order?
    unsigned int L = (_n - r)/2;    // half order
    
    // compute poles
    unsigned int i;
    unsigned int k=0;
    for (i=0; i<L; i++) {
        float theta = (float)(2*(i+1) + _n - 1)*M_PI/(float)(2*_n);
        _pa[k++] = 1.0f / (a*cosf(theta) - _Complex_I*b*sinf(theta));
        _pa[k++] = 1.0f / (a*cosf(theta) + _Complex_I*b*sinf(theta));
    }

    // if filter order is odd, there is an additional pole on the
    // real axis
    if (r) _pa[k++] = -1.0f / a;

    // ensure we have written exactly _n poles
    assert(k==_n);

    // compute zeros
    k=0;
    for (i=0; i<L; i++) {
        float theta = (float)(0.5f*M_PI*(2*(i+1)-1)/(float)(_n));
        _za[k++] = -1.0f / (_Complex_I*cosf(theta));
        _za[k++] =  1.0f / (_Complex_I*cosf(theta));
    }

    // ensure we have written exactly 2*L poles
    assert(k==2*L);

    // compute analog gain (ignored in digital conversion)
    *_ka = 1.0f;
    for (i=0; i<_n; i++)
        *_ka *= _pa[i];
    for (i=0; i<2*L; i++)
        *_ka /= _za[i];

    return LIQUID_OK;
}

