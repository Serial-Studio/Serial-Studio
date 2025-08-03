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
// Gamma distribution
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

float randgammaf(float _alpha,
                 float _beta)
{
    // validate input
    if (_alpha <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randgammaf(), alpha must be greater than zero");
        return 0.0f;
    } else if (_beta <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randgammaf(), beta must be greater than zero");
        return 0.0f;
    }

    unsigned int n = (unsigned int) floorf(_alpha);

    // residual
    float delta = _alpha - (float)n;

    // generate x' ~ Gamma(n,1)
    float x_n = 0.0f;
    unsigned int i;
    for (i=0; i<n; i++) {
        float u = randf();
        x_n += - logf(u);
    }

    // generate x'' ~ Gamma(delta,1) using rejection method
    float x_delta = randgammaf_delta(delta);

    // 
    return _beta * (x_delta + x_n);
}

// Gamma distribution cumulative distribution function
//          x^(a-1) exp{-x/b)
//  f(x) = -------------------
//            Gamma(a) b^a
//  where
//      a = alpha, a > 0
//      b = beta,  b > 0
//      Gamma(z) = regular gamma function
//      x >= 0
float randgammaf_pdf(float _x,
                     float _alpha,
                     float _beta)
{
    // validate input
    if (_alpha <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randgammaf_pdf(), alpha must be greater than zero");
        return 0.0f;
    } else if (_beta <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randgammaf_pdf(), beta must be greater than zero");
        return 0.0f;
    }

    if (_x <= 0.0f)
        return 0.0f;

    float t0 = powf(_x, _alpha-1.0f);
    float t1 = expf(-_x / _beta);
    float t2 = liquid_gammaf(_alpha);
    float t3 = powf(_beta, _alpha);

    return (t0*t1)/(t2*t3);
}

// Gamma distribution cumulative distribution function
//  F(x) = gamma(a,x/b) / Gamma(a)
//  where
//      a = alpha,  alpha > 0
//      b = beta,   beta > 0
//      gamma(a,z) = lower incomplete gamma function
//      Gamma(z)   = regular gamma function
//
float randgammaf_cdf(float _x,
                     float _alpha,
                     float _beta)
{
    // validate input
    if (_alpha <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randgammaf_cdf(), alpha must be greater than zero");
        return 0.0f;
    } else if (_beta <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randgammaf_cdf(), beta must be greater than zero");
        return 0.0f;
    }

    if (_x <= 0.0f)
        return 0.0f;

    return liquid_lowergammaf(_alpha, _x/_beta) / liquid_gammaf(_alpha);
}


// 
// internal methods
//

// generate x ~ Gamma(delta,1)
float randgammaf_delta(float _delta)
{
    // validate input
    if ( _delta < 0.0f || _delta >= 1.0f ) {
        liquid_error(LIQUID_EICONFIG,"randgammaf_delta(), delta must be in [0,1)");
        return 0.0f;
    }

    // initialization
    float delta_inv = 1.0f / _delta;
    float e = expf(1.0f);
    float v0 = e / (e + _delta);

    float V0 = 0.0f;
    float V1 = 0.0f;
    float V2 = 0.0f;

    //unsigned int m = 1;

    float xi = 0.0f;
    float eta = 0.0f;

    while (1) {
        // step 2
        V0 = randf();
        V1 = randf();
        V2 = randf();

        if (V2 <= v0) {
            // step 4
            xi = powf(V1, delta_inv);
            eta = V0 * powf(xi, _delta - 1.0f);
        } else {
            // step 5
            xi = 1.0f - logf(V1);
            eta = V0 * expf(-xi);
        }

        // step 6
        if ( eta > powf(xi,_delta-1.0f)*expf(-xi) ) {
            //m++;
        } else {
            break;
        }
    }

    // xi ~ Gamma(delta,1)
    return xi;
}

