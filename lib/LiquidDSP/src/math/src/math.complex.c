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
// Complex math functions (trig, log, exp, etc.)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

// complex square root
float complex liquid_csqrtf(float complex _z)
{
    float r = cabsf(_z);            // magnitude of _z
    float a = crealf(_z);           // real component of _z

    float re = sqrtf(0.5f*(r+a));   // real component of return value
    float im = sqrtf(0.5f*(r-a));   // imag component of return value

    // return value, retaining sign of imaginary component
    return cimagf(_z) > 0 ? re + _Complex_I*im :
                            re - _Complex_I*im;
}

// complex exponent
float complex liquid_cexpf(float complex _z)
{
    float r = expf( crealf(_z) );
    float re = cosf( cimagf(_z) );
    float im = sinf( cimagf(_z) );

    return r * ( re + _Complex_I*im );
}

// complex logarithm
float complex liquid_clogf(float complex _z)
{
    return logf(cabsf(_z)) + _Complex_I*cargf(_z);
}

// complex arcsin
float complex liquid_casinf(float complex _z)
{
    return 0.5f*M_PI - liquid_cacosf(_z);
}

// complex arccos
float complex liquid_cacosf(float complex _z)
{
    // return based on quadrant
    int sign_i = crealf(_z) > 0;
    int sign_q = cimagf(_z) > 0;

    if (sign_i == sign_q) {
        return -_Complex_I*liquid_clogf(_z + liquid_csqrtf(_z*_z - 1.0f));
    } else {
        return -_Complex_I*liquid_clogf(_z - liquid_csqrtf(_z*_z - 1.0f));
    }

    // should never get to this state
    return 0.0f;
}

// complex arctan
float complex liquid_catanf(float complex _z)
{
    float complex t0 = 1.0f - _Complex_I*_z;
    float complex t1 = 1.0f + _Complex_I*_z;

    return 0.5f*_Complex_I*liquid_clogf( t0 / t1 );
}

#if 0
// approximation to cargf() but faster(?)
// NOTE: this is not actually very accurate
float liquid_cargf_approx(float complex _x)
{
    float theta;
    float xi = crealf(_x);
    float xq = cimagf(_x);

    if (xi == 0.0f) {
        if (xq == 0.0f)
            return 0.0f;
        return xq > 0.0f ? M_PI_2 : -M_PI_2;
    } else {
        theta = xq / fabsf(xi);
    }

    if (theta >  M_PI_2)
        theta =  M_PI_2;
    else if (theta < -M_PI_2)
        theta = -M_PI_2;
    return theta;
}
#endif

