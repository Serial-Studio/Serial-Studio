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
// gamma functions
//
// liquid_lngammaf()        :   log( Gamma(z) )
// liquid_gammaf()          :   Gamma(z)
// liquid_lnlowergammaf()   :   log( gamma(z,a) ), lower incomplete
// liquid_lnuppergammaf()   :   log( Gamma(z,a) ), upper incomplete
// liquid_lowergammaf()     :   gamma(z,a), lower incomplete
// liquid_uppergammaf()     :   Gamma(z,a), upper incomplete
// liquid_factorialf()      :   z!
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

#define NUM_LNGAMMA_ITERATIONS (256)
#define EULER_GAMMA            (0.57721566490153286)
float liquid_lngammaf(float _z)
{
    float g;
    if (_z < 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_lngammaf(), undefined for z <= 0");
        return 0.0f;
    } else if (_z < 10.0f) {
#if 0
        g = -EULER_GAMMA*_z - logf(_z);
        unsigned int k;
        for (k=1; k<NUM_LNGAMMA_ITERATIONS; k++) {
            float t0 = _z / (float)k;
            float t1 = logf(1.0f + t0);

            g += t0 - t1;
        }
#else
        // Use recursive formula:
        //    gamma(z+1) = z * gamma(z)
        // therefore:
        //    log(Gamma(z)) = log(gamma(z+1)) - ln(z)
        return liquid_lngammaf(_z + 1.0f) - logf(_z);
#endif
    } else {
        // high value approximation
        g = 0.5*( logf(2*M_PI)-log(_z) );
        g += _z*( logf(_z+(1/(12.0f*_z-0.1f/_z)))-1);
    }
    return g;
}

float liquid_gammaf(float _z)
{
    if (_z < 0) {
        // use identities
        //  (1) gamma(z)*gamma(-z) = -pi / (z*sin(pi*z))
        //  (2) z*gamma(z) = gamma(1+z)
        //
        // therefore:
        //  gamma(z) = pi / ( gamma(1-z) * sin(pi*z) )
        float t0 = liquid_gammaf(1.0 - _z);
        float t1 = sinf(M_PI*_z);
        if (t0==0 || t1==0)
            liquid_error(LIQUID_EIVAL,"liquid_gammaf(), divide by zero");
        return M_PI / (t0 * t1);
    } else {
        return expf( liquid_lngammaf(_z) );
    }
}

// ln( gamma(z,alpha) ) : lower incomplete gamma function
#define LOWERGAMMA_MIN_ITERATIONS 50    // minimum number of iterations
#define LOWERGAMMA_MAX_ITERATIONS 1000  // maximum number of iterations
float liquid_lnlowergammaf(float _z, float _alpha)
{
    float t0 = _z * logf(_alpha);
    float t1 = liquid_lngammaf(_z);
    float t2 = -_alpha;
    float t3 = 0.0f;

    unsigned int k = 0;
    float log_alpha = logf(_alpha);
    float tprime = 0.0f;
    float tmax = 0.0f;
    float t = 0.0f;
    for (k=0; k<LOWERGAMMA_MAX_ITERATIONS; k++) {
        // retain previous value for t
        tprime = t;

        // compute log( alpha^k / Gamma(_z + k + 1) )
        //         = k*log(alpha) - lnGamma(_z + k + 1)
        t = k*log_alpha - liquid_lngammaf(_z + (float)k + 1.0f);

        // accumulate e^t
        t3 += expf(t);

        // check premature stopping criteria
        if (k==0 || t > tmax)
            tmax = t;

        // conditions:
        //  1. minimum number of iterations met
        //  2. surpassed inflection point: k*log(alpha) - log(Gamma(z+k+1))
        //     has an inverted parabolic shape
        //  3. sufficiently beyond peak
        if ( k > LOWERGAMMA_MIN_ITERATIONS && tprime > t && (tmax-t) > 20.0f)
            break;
    }

    return t0 + t1 + t2 + logf(t3);
}

// ln( Gamma(z,alpha) ) : upper incomplete gamma function
float liquid_lnuppergammaf(float _z, float _alpha)
{
    return logf( liquid_gammaf(_z) - liquid_lowergammaf(_z,_alpha) );
}


// gamma(z,alpha) : lower incomplete gamma function
float liquid_lowergammaf(float _z, float _alpha)
{
    return expf( liquid_lnlowergammaf(_z,_alpha) );
}

// Gamma(z,alpha) : upper incomplete gamma function
float liquid_uppergammaf(float _z, float _alpha)
{
    return expf( liquid_lnuppergammaf(_z,_alpha) );
}

// compute _n!
float liquid_factorialf(unsigned int _n) {
    return fabsf(liquid_gammaf((float)(_n+1)));
}

