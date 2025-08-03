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
// Chebeshev type-I filter design
//

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "liquid.internal.h"

#define LIQUID_DEBUG_CHEBY1_PRINT   0

// Compute analog zeros, poles, gain of low-pass Chebyshev
// Type I filter, grouping complex conjugates together. If
// the filter order is odd, the single real pole is at the
// end of the array.  There are no zeros for the analog
// Chebyshev Type I filter.
//  _n      :   filter order
//  _ep     :   epsilon, related to pass-band ripple
//  _za     :   output analog zeros [length:  0]
//  _pa     :   output analog poles [length: _n]
//  _ka     :   output analog gain
int cheby1_azpkf(unsigned int           _n,
                 float                  _ep,
                 liquid_float_complex * _za,
                 liquid_float_complex * _pa,
                 liquid_float_complex * _ka)
{
    // temporary values
    float t0 = sqrt(1.0 + 1.0/(_ep*_ep));
    float tp = powf( t0 + 1.0/_ep, 1.0/(float)(_n) );
    float tm = powf( t0 - 1.0/_ep, 1.0/(float)(_n) );

    float b = 0.5*(tp + tm);    // ellipse major axis
    float a = 0.5*(tp - tm);    // ellipse minor axis

#if LIQUID_DEBUG_CHEBY1_PRINT
    printf("  ep : %12.8f\n", _ep);
    printf("  b  : %12.8f\n", b);
    printf("  a  : %12.8f\n", a);
#endif

    // filter order variables
    unsigned int r = _n%2;          // odd order?
    unsigned int L = (_n - r)/2;    // half order
    
    // compute poles
    unsigned int i;
    unsigned int k=0;
    for (i=0; i<L; i++) {
        float theta = (float)(2*(i+1) + _n - 1)*M_PI/(float)(2*_n);
        _pa[k++] = a*cosf(theta) - _Complex_I*b*sinf(theta);
        _pa[k++] = a*cosf(theta) + _Complex_I*b*sinf(theta);
    }

    // if filter order is odd, there is an additional pole on the
    // real axis
    if (r) _pa[k++] = -a;

    // ensure we have written exactly _n poles
    assert(k==_n);

    // compute analog gain (ignored in digital conversion)
    *_ka = r ? 1.0f : 1.0f / sqrtf(1.0f + _ep*_ep);
    for (i=0; i<_n; i++)
        *_ka *= _pa[i];

    return LIQUID_OK;
}

