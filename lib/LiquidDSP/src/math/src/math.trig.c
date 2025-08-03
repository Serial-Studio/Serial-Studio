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
// Useful mathematical formulae (trig)
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

#if 0
#define LIQUID_SINF_POLYORD (4)
static float liquid_sinf_poly[LIQUID_SINF_POLYORD] = {
  -0.113791698328739f,
  -0.069825754521815f,
   1.026821728423492f,
   0.000000000000000f
};
#endif

void liquid_sincosf(float _x,
                    float * _sinf,
                    float * _cosf)
{
    * _sinf = sinf(_x);
    * _cosf = cosf(_x);
}

float liquid_sinf(float _x)
{
    float s, c;
    liquid_sincosf(_x,&s,&c);
    return s;
}

float liquid_cosf(float _x)
{
    float s, c;
    liquid_sincosf(_x,&s,&c);
    return c;
}

float liquid_tanf(float _x)
{
    float s, c;
    liquid_sincosf(_x,&s,&c);
    return s/c;
}

float liquid_expf(float _x)
{
    return expf(_x);
}

float liquid_logf(float _x)
{
    return logf(_x);
}

