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

//
// Weibull distribution
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

// Weibull
float randweibf(float _alpha,
                float _beta,
                float _gamma)
{
    // validate input
    if (_alpha <= 0) {
        liquid_error(LIQUID_EICONFIG,"randweibf(), alpha must be greater than zero");
        return 0.0f;
    } else if (_beta <= 0) {
        liquid_error(LIQUID_EICONFIG,"randweibf(), beta must be greater than zero");
        return 0.0f;
    }

    float u;
    do {
        u = randf();
    } while (u==0.0f);

    return _gamma + _beta*powf( -logf(u), 1.0f/_alpha );
}

// Weibull random number probability distribution function
float randweibf_pdf(float _x,
                    float _alpha,
                    float _beta,
                    float _gamma)
{
#ifdef LIQUID_VALIDATE_INPUT
    // validate input
    if (_alpha <= 0) {
        liquid_error(LIQUID_EICONFIG,"randweibf_pdf(), alpha must be greater than zero");
        return 0.0f;
    } else if (_beta <= 0) {
        liquid_error(LIQUID_EICONFIG,"randweibf_pdf(), beta must be greater than zero");
        return 0.0f;
    }
#endif

    if (_x < _gamma)
        return 0.0f;

    float t = _x - _gamma;
    return (_alpha/_beta) * powf(t/_beta, _alpha-1.0f) * expf( -powf(t/_beta, _alpha) );
}

// Weibull random number cumulative distribution function
float randweibf_cdf(float _x,
                    float _alpha,
                    float _beta,
                    float _gamma)
{
#ifdef LIQUID_VALIDATE_INPUT
    // validate input
    if (_alpha <= 0) {
        liquid_error(LIQUID_EICONFIG,"randweibf_cdf(), alpha must be greater than zero");
        return 0.0f;
    } else if (_beta <= 0) {
        liquid_error(LIQUID_EICONFIG,"randweibf_cdf(), beta must be greater than zero");
        return 0.0f;
    }
#endif

    if (_x <= _gamma)
        return 0.0f;

    return 1.0f - expf( -powf((_x-_gamma)/_beta, _alpha) );
}

