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
// Uniform random number generator definitions
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

// uniform random number generator
float randf() {
    return randf_inline();
}

// uniform random number probability distribution function
float randf_pdf(float _x)
{
    return (_x < 0.0f || _x > 1.0f) ? 0.0f : 1.0f;
}

// uniform random number cumulative distribution function
float randf_cdf(float _x)
{
    if (_x < 0.0f) {
        return 0.0f;
    } else if (_x > 1.0f) {
        return 1.0f;
    }

    return _x;
}

// uniform random number generator with arbitrary bounds
float randuf(float _a, float _b)
{
    // check bounds
    if (_a >= _b) {
        liquid_error(LIQUID_EIRANGE,"randuf(%g,%g) has invalid range", _a, _b);
        return 0;
    }

    return _a + (_b - _a)*randf_inline();
}

// uniform random number probability distribution function
float randuf_pdf(float _x,
                 float _a,
                 float _b)
{
    // check bounds
    if (_a >= _b) {
        liquid_error(LIQUID_EIRANGE,"randuf_pdf(%g,%g,%g) has invalid range", _x, _a, _b);
        return 0;
    }

    return (_x < _a || _x > _b) ? 0.0f : 1.0f / (_b - _a);
}

// uniform random number cumulative distribution function
float randuf_cdf(float _x,
                 float _a,
                 float _b)
{
    // check bounds
    if (_a >= _b) {
        liquid_error(LIQUID_EIRANGE,"randuf_cdf(%g,%g,%g) has invalid range", _x, _a, _b);
        return 0;
    }

    if (_x < _a) {
        return 0.0f;
    } else if (_x > _b) {
        return 1.0f;
    }

    return (_x - _a) / (_b - _a);
}

