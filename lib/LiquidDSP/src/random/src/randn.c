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
//
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

// Gauss
float randnf()
{
    // generate two uniform random numbers
    float u1, u2;

    // ensure u1 does not equal zero
    do {
        u1 = randf();
    } while (u1 == 0.0f);

    u2 = randf();

    return sqrtf(-2*logf(u1)) * sinf(2*M_PI*u2);
    //return sqrtf(-2*logf(u1)) * cosf(2*M_PI*u2);
}

void awgn(float *_x, float _nstd)
{
    *_x += randnf()*_nstd;
}

// Complex Gauss
void crandnf(float complex * _y)
{
    // generate two uniform random numbers
    float u1, u2;

    // ensure u1 does not equal zero
    do {
        u1 = randf();
    } while (u1 == 0.0f);

    u2 = randf();

    *_y = sqrtf(-2*logf(u1)) * cexpf(_Complex_I*2*M_PI*u2);
}

// Internal complex Gauss (inline)
float complex icrandnf()
{
    float complex y;
    crandnf(&y);
    return y;
}

void cawgn(float complex *_x, float _nstd)
{
    *_x += icrandnf()*_nstd*0.707106781186547f;
}

// Gauss random number probability distribution function
float randnf_pdf(float _x,
                 float _eta,
                 float _sig)
{
    // validate input
    if (_sig <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"randnf_pdf(), standard deviation must be greater than zero");
        return 0.0f;
    }

    float t  = _x - _eta;
    float s2 = _sig * _sig;
    return expf(-t*t/(2.0f*s2)) / sqrtf(2.0f*M_PI*s2);
}

// Gauss random number cumulative distribution function
float randnf_cdf(float _x,
                 float _eta,
                 float _sig)
{
    return 0.5 + 0.5*erff( M_SQRT1_2*(_x-_eta)/_sig );
}

