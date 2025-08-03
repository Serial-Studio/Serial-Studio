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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

#define LIQUID_VALIDATE_INPUT

float compress_mulaw(float _x, float _mu)
{
#ifdef LIQUID_VALIDATE_INPUT
    if ( _mu <= 0.0f ) {
        liquid_error(LIQUID_EIRANGE,"compress_mulaw(), mu out of range");
        return 0.0f;
    }
#endif
    float y = logf(1 + _mu*fabsf(_x)) / logf(1 + _mu);
    return copysignf(y, _x);
}

float expand_mulaw(float _y, float _mu)
{
#ifdef LIQUID_VALIDATE_INPUT
    if ( _mu <= 0.0f ) {
        liquid_error(LIQUID_EIRANGE,"expand_mulaw(), mu out of range");
        return 0.0f;
    }
#endif
    float x = (1/_mu)*( powf(1+_mu,fabsf(_y)) - 1);
    return copysign(x, _y);
}

int compress_cf_mulaw(float complex _x, float _mu, float complex * _y)
{
#ifdef LIQUID_VALIDATE_INPUT
    if ( _mu <= 0.0f )
        return liquid_error(LIQUID_EIRANGE,"compress_mulaw(), mu out of range");
#endif
    *_y = cexpf(_Complex_I*cargf(_x)) * logf(1 + _mu*cabsf(_x)) / logf(1 + _mu);
    return LIQUID_OK;
}

int expand_cf_mulaw(float complex _y, float _mu, float complex * _x)
{
#ifdef LIQUID_VALIDATE_INPUT
    if ( _mu <= 0.0f )
        return liquid_error(LIQUID_EIRANGE,"expand_mulaw(), mu out of range");
#endif
    *_x = cexpf(_Complex_I*cargf(_y)) * (1/_mu)*( powf(1+_mu,cabsf(_y)) - 1);
    return LIQUID_OK;
}

/*
float compress_alaw(float _x, float _a)
{

}

float expand_alaw(float _x, float _a)
{

}
*/

