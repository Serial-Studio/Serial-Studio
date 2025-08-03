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
// Nakagami-m distribution
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

float randnakmf(float _m,
                float _omega)
{
    // validate input
    if (_m < 0.5f) {
        liquid_error(LIQUID_EICONFIG,"randnakmf(), m cannot be less than 0.5");
        return 0.0f;
    } else if (_omega <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randnakmf(), omega must be greater than zero");
        return 0.0f;
    }

    // generate Gamma random variable
    float alpha = _m;
    float beta  = _omega / _m;
    float x = randgammaf(alpha,beta);

    // sqrt(x) ~ Nakagami(m,omega)
    return sqrtf(x);
}

// Nakagami-m distribution probability distribution function
// Nakagami-m
//  f(x) = (2/Gamma(m)) (m/omega)^m x^(2m-1) exp{-(m/omega)x^2}
// where
//      m       : shape parameter, m >= 0.5
//      omega   : spread parameter, omega > 0
//      Gamma(z): regular complete gamma function
//      x >= 0
float randnakmf_pdf(float _x,
                    float _m,
                    float _omega)
{
    // validate input
    if (_m < 0.5f) {
        liquid_error(LIQUID_EICONFIG,"randnakmf_pdf(), m cannot be less than 0.5");
        return 0.0f;
    } else if (_omega <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randnakmf_pdf(), omega must be greater than zero");
        return 0.0f;
    }

    if (_x <= 0.0f)
        return 0.0f;

    float t0 = liquid_lngammaf(_m);
    float t1 = _m * logf(_m/_omega);
    float t2 = (2*_m - 1.0f) * logf(_x);
    float t3 = -(_m/_omega)*_x*_x;

    return 2.0f * expf( -t0 + t1 + t2 + t3 );
}

// Nakagami-m distribution cumulative distribution function
//  F(x) = gamma(m, x^2 m / omega) / Gamma(m)
//  where
//      gamma(z,a) = lower incomplete gamma function
//      Gamma(z)   = regular gamma function
//
float randnakmf_cdf(float _x,
                    float _m,
                    float _omega)
{
    // validate input
    if (_m < 0.5f) {
        liquid_error(LIQUID_EICONFIG,"randnakmf_cdf(), m cannot be less than 0.5");
        return 0.0f;
    } else if (_omega <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randnakmf_cdf(), omega must be greater than zero");
        return 0.0f;
    }

    if (_x <= 0.0f)
        return 0.0f;

    float t0 = liquid_lnlowergammaf(_m, _x*_x*_m/_omega);
    float t1 = liquid_lngammaf(_m);
    return expf( t0 - t1 );
}

