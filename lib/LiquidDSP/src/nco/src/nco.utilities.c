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
// nco.utilities.c
//
// Numerically-controlled oscillator (nco) utilities
//

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// NOTE: this is defined in multiple places, but should be included here
//       until an appropriate place for literals can be found
#define LIQUID_PI   (3.14159265358979323846264338327950288f)

// unwrap phase of array (basic)
void liquid_unwrap_phase(float * _theta,
                         unsigned int _n)
{
    unsigned int i;
    for (i=1; i<_n; i++) {
        while ( (_theta[i] - _theta[i-1]) >  LIQUID_PI ) _theta[i] -= 2*LIQUID_PI;
        while ( (_theta[i] - _theta[i-1]) < -LIQUID_PI ) _theta[i] += 2*LIQUID_PI;
    }
}

// unwrap phase of array (advanced)
// TODO: verify this method
void liquid_unwrap_phase2(float * _theta,
                          unsigned int _n)
{
    unsigned int i;

    // make an initial estimate of phase difference
    float dphi = 0.0f;
    for (i=1; i<_n; i++)
        dphi += _theta[i] - _theta[i-1];

    dphi /= (float)(_n-1);

    for (i=1; i<_n; i++) {
        while ( (_theta[i] - _theta[i-1]) >  LIQUID_PI+dphi ) _theta[i] -= 2*LIQUID_PI;
        while ( (_theta[i] - _theta[i-1]) < -LIQUID_PI+dphi ) _theta[i] += 2*LIQUID_PI;
    }
}



