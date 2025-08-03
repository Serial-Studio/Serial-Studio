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
// sumsq.c : sum of squares
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "liquid.internal.h"

// sum squares, basic loop
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqf(float *      _v,
                    unsigned int _n)
{
    // initialize accumulator
    float r=0;

    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // run computation in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
        r += _v[i  ] * _v[i  ];
        r += _v[i+1] * _v[i+1];
        r += _v[i+2] * _v[i+2];
        r += _v[i+3] * _v[i+3];
    }

    // clean up remaining
    for ( ; i<_n; i++)
        r += _v[i] * _v[i];

    // return result
    return r;
}

// sum squares, basic loop
//  _v      :   input array [size: 1 x _n]
//  _n      :   input length
float liquid_sumsqcf(float complex * _v,
                     unsigned int    _n)
{
    // simple method: type cast input as real pointer, run double
    // length sumsqf method
    float * v = (float*) _v;
    return liquid_sumsqf(v, 2*_n);
}

