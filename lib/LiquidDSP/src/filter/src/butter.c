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
// Butterworth filter design
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "liquid.internal.h"

#define LIQUID_DEBUG_BUTTER_PRINT   0

// Compute analog zeros, poles, gain of low-pass Butterworth
// filter, grouping complex conjugates together. If filter
// order is odd, the single real pole (-1) is at the end of
// the array.  There are no zeros for the analog Butterworth
// filter.  The gain is unity.
//  _n      :   filter order
//  _za     :   output analog zeros [length:  0]
//  _pa     :   output analog poles [length: _n]
//  _ka     :   output analog gain
int butter_azpkf(unsigned int           _n,
                 liquid_float_complex * _za,
                 liquid_float_complex * _pa,
                 liquid_float_complex * _ka)
{
    unsigned int r = _n%2;
    unsigned int L = (_n - r)/2;
    
    unsigned int i;
    unsigned int k=0;
    for (i=0; i<L; i++) {
        float theta = (float)(2*(i+1) + _n - 1)*M_PI/(float)(2*_n);
        _pa[k++] = cexpf( _Complex_I*theta);
        _pa[k++] = cexpf(-_Complex_I*theta);
    }

    if (r) _pa[k++] = -1.0f;

    if (k != _n)
        return liquid_error(LIQUID_EINT,"butter_azpkf(), filter order mismatch");

    *_ka = 1.0;
    return LIQUID_OK;
}

