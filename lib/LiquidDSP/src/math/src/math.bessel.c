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
// Bessel Functions
//
// liquid_lnbesselif    :   log modified Bessel function of the first kind
// liquid_besselif      :   modified Bessel function of the first kind
// liquid_besseli0f     :   modified Bessel function of the first kind (order 0)
// liquid_besseljf      :   Bessel function of the first kind
// liquid_besselj0f     :   Bessel function of the first kind (order 0)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "liquid.internal.h"

// log(I_v(z)) : log Modified Bessel function of the first kind
#define NUM_BESSELI_ITERATIONS 64
float liquid_lnbesselif(float _nu,
                        float _z)
{
    // TODO : validate input

    // special case: check for zeros; besseli_nu(0) = (nu = 0 ? 1 : 0)
    if (_z == 0) {
        return _nu == 0.0f ? 0.0f : -FLT_MAX;
    }

    // special case: _nu = 1/2, besseli(z) = sqrt(2/pi*z)*sinh(z)
    if (_nu == 0.5f) {
        return 0.5f*logf(2.0f/(M_PI*_z)) + logf(sinhf(_z));
    }

    // low signal approximation
    if (_z < 1e-3f*sqrtf(_nu + 1.0f)) {
        return -liquid_lngammaf(_nu + 1.0f) + _nu*logf(0.5f*_z);
    }

#if 0
    // high-signal approximation: _z >> _nu
    //     I_v(z) ~ exp(z) / sqrt(2*pi*z)
    //  ln I_v(z) ~ z - 0.5*ln(2*pi*z)
    if ( _nu > 0.0f && logf(_z/_nu) > 8.0f )
        return _z - 0.5f*logf(2*M_PI*_z);
#endif

    float t0 = _nu*logf(0.5f*_z);
    float t1 = 0.0f;
    float t2 = 0.0f;
    float t3 = 0.0f;
    float y = 0.0f;

    unsigned int k;
    for (k=0; k<NUM_BESSELI_ITERATIONS; k++) {
        // compute log( (z^2/4)^k )
        t1 = 2.0f * k * logf(0.5f*_z);

        // compute: log( k! * Gamma(nu + k +1) )
        t2 = liquid_lngammaf((float)k + 1.0f);
        t3 = liquid_lngammaf(_nu + (float)k + 1.0f);

        // accumulate y
        y += expf( t1 - t2 - t3 );
        //printf("%3u : t1: %12.3e, t2: %12.4e, t3: %12.4e, y: %12.4e\n", k, t1, t2, t3, y);
    }

    return t0 + logf(y);
}

// I_v(z) : Modified Bessel function of the first kind
float liquid_besselif(float _nu,
                      float _z)
{
    // special case: check for zeros; besseli_nu(0) = (nu = 0 ? 1 : 0)
    if (_z == 0) {
        return _nu == 0.0f ? 1.0f : 0.0f;
    }

    // special case: _nu = 1/2, besseli(z) = sqrt(2/pi*z)*sinh(z)
    if (_nu == 0.5f) {
        return sqrtf(2.0f/(M_PI*_z)) * sinhf(_z);
    }

    // low signal approximation
    if (_z < 1e-3f*sqrtf(_nu + 1.0f)) {
        return powf(0.5f*_z,_nu) / liquid_gammaf(_nu + 1.0f);
    }

    // derive from logarithmic expansion
    return expf( liquid_lnbesselif(_nu, _z) );
}

// I_0(z) : Modified bessel function of the first kind (order zero)
float liquid_besseli0f(float _z)
{
    return liquid_besselif(0,_z);
}

// J_v(z) : Bessel function of the first kind
#define NUM_BESSELJ_ITERATIONS 128
float liquid_besseljf(float _nu,
                      float _z)
{
    // TODO : validate input

    // special case: check for zeros; besselj_nu(0) = (nu = 0 ? 1 : 0)
    if (_z == 0) {
        return _nu == 0.0f ? 1.0f : 0.0f;
    }

    // low signal approximation
    if (_z < 1e-3f*sqrtf(_nu + 1.0f)) {
        return powf(0.5f*_z,_nu) / liquid_gammaf(_nu + 1.0f);
    }

    float J = 0.0f;

    float abs_nu = fabsf(_nu);

    unsigned int k;
    for (k=0; k<NUM_BESSELJ_ITERATIONS; k++) {
        // compute: (2k + |nu|)
        float t0 = (2.0f*k + abs_nu);

        // compute: (2k + |nu|)*log(z)
        float t1 = t0 * logf(_z);

        // compute: (2k + |nu|)*log(2)
        float t2 = t0 * logf(2.0f);

        // compute: log(Gamma(k+1))
        float t3 = liquid_lngammaf((float)k + 1.0f);

        // compute: log(Gamma(|nu|+k+1))
        float t4 = liquid_lngammaf(abs_nu + (float)k + 1.0f);

        // accumulate J
        if ( (k%2) == 0) J += expf(t1 - t2 - t3 - t4);
        else             J -= expf(t1 - t2 - t3 - t4);
    }

    return J;
}

// J_0(z) : Bessel function of the first kind (order zero)
#define NUM_BESSELJ0_ITERATIONS 16
float liquid_besselj0f(float _z)
{
    // large signal approximation, see
    // Gross, F. B "New Approximations to J0 and J1 Bessel Functions,"
    //   IEEE Trans. on Antennas and Propagation, vol. 43, no. 8,
    //   August, 1995
    if (fabsf(_z) > 10.0f)
        return sqrtf(2/(M_PI*fabsf(_z)))*cosf(fabsf(_z)-M_PI/4);

    unsigned int k;
    float t, y=0.0f;
    for (k=0; k<NUM_BESSELJ0_ITERATIONS; k++) {
        t = powf(_z/2, (float)k) / tgamma((float)k+1);
        y += (k%2) ? -t*t : t*t;
    }
    return y;
}

