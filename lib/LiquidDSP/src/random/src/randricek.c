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
// Rice-K distribution
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

// Rice-K
float randricekf(float _K, float _omega)
{
    float complex x, y;
    float s = sqrtf((_omega*_K)/(_K+1));
    float sig = sqrtf(0.5f*_omega/(_K+1));
    crandnf(&x);
    y = _Complex_I*( crealf(x)*sig + s ) +
                   ( cimagf(x)*sig     );
    return cabsf(y);
}


// Rice-K random number probability distribution function
//  f(x) = (x/sigma^2) exp{ -(x^2+s^2)/(2sigma^2) } I0( x s / sigma^2 )
// where
//  s     = sqrt( omega*K/(K+1) )
//  sigma = sqrt(0.5 omega/(K+1))
// and
//  K     = shape parameter
//  omega = spread parameter
//  I0    = modified Bessel function of the first kind
//  x >= 0
float randricekf_pdf(float _x,
                     float _K,
                     float _omega)
{
    if (_x < 0.0f)
        return 0.0f;

    float s = sqrtf((_omega*_K)/(_K+1));
    float sig = sqrtf(0.5f*_omega/(_K+1));

    float t = _x*_x + s*s;

    float sig2 = sig*sig;

    // check high tail condition
    if ( (_x*s/sig2) > 80.0f )
        return 0.0f;

    float t0 = logf(_x) - logf(sig2);
    float t1 = -t / (2*sig2);
    float t2 = liquid_lnbesselif(0.0f, _x*s/sig2);
    return expf(t0 + t1 + t2);

    //return (_x / sig2) * expf(-t / (2*sig2)) * liquid_besseli0f(_x*s/sig2);
}

// Rice-K random number cumulative distribution function
float randricekf_cdf(float _x,
                     float _K,
                     float _omega)
{
    if (_x <= 0.0f)
        return 0.0f;

    float s = sqrtf((_omega*_K)/(_K+1));
    float sig = sqrtf(0.5f*_omega/(_K+1));

    // test arguments of Q1 function
    float alpha = s/sig;
    float beta  = _x/sig;
    //float Q1 = liquid_MarcumQ1f(alpha, beta);
    //printf("  Q1(%12.4e, %12.4e) = %12.4e\n", alpha, beta, Q1);

    if ( (alpha / beta) > 3.0f )
        return 0.0f;
    if ( (beta / alpha) > 3.0f )
        return 1.0f;

    float F = 1.0f - liquid_MarcumQ1f(alpha, beta);

    // check for precision error
    if (F < 0.0f) return 0.0f;
    if (F > 1.0f) return 1.0f;

    return F;
}


