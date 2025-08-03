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
// Exponential distribution
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

// Exponential
float randexpf(float _lambda)
{
    // validate input
    if (_lambda <= 0) {
        liquid_error(LIQUID_EIRANGE,"randexpf(%g) has invalid range", _lambda);
        return 0.0f;
    }

    // compute a non-zero uniform random variable in (0,1]
    float u;
    do {
        u = randf();
    } while (u==0.0f);

    // perform variable transformation
    return -logf( u ) / _lambda;
}

// Exponential random number probability distribution function
float randexpf_pdf(float _x,
                   float _lambda)
{
    // validate input
    if (_lambda <= 0) {
        liquid_error(LIQUID_EIRANGE,"randexpf_pdf(%g,%g) has invalid range", _x, _lambda);
        return 0.0f;
    }

    if (_x < 0.0f)
        return 0.0f;

    return _lambda * expf(-_lambda*_x);
}

// Exponential random number cumulative distribution function
float randexpf_cdf(float _x,
                   float _lambda)
{
    // validate input
    if (_lambda <= 0) {
        liquid_error(LIQUID_EIRANGE,"randexpf_cdf(%g,%g) has invalid range", _x, _lambda);
        return 0.0f;
    }

    if (_x < 0.0f)
        return 0.0f;

    return 1.0f - expf(-_lambda*_x);
}

