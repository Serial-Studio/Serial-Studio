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
//
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "liquid.internal.h"

#define LIQUID_VALIDATE_INPUT
#define QUANTIZER_MAX_BITS      32

// inline quantizer: 'analog' signal in [-1, 1]

unsigned int quantize_adc(float _x, unsigned int _num_bits)
{
#ifdef LIQUID_VALIDATE_INPUT
    if (_num_bits > QUANTIZER_MAX_BITS) {
        liquid_error(LIQUID_EIRANGE,"quantize_adc(), maximum bits exceeded");
        return 0;
    }
#endif

    if (_num_bits == 0)
        return 0;

    unsigned int n = _num_bits-1;   // 
    unsigned int N = 1<<n;          // 2^n

    // scale
    int neg = (_x < 0);
    unsigned int r = floorf(fabsf(_x)*N);

    // clip
    if (r >= N)
        r = N-1;

    // if negative set MSB to 1
    if (neg)
        r |= N;

    return r;
}

float quantize_dac(unsigned int _s, unsigned int _num_bits)
{
#ifdef LIQUID_VALIDATE_INPUT
    if (_num_bits > QUANTIZER_MAX_BITS) {
        liquid_error(LIQUID_EIRANGE,"quantize_dac(), maximum bits exceeded");
        return 0.0f;
    }
#endif
    if (_num_bits == 0)
        return 0.0f;

    unsigned int n = _num_bits-1;   //
    unsigned int N = 1<<n;          // 2^n
    float r = ((float)(_s & (N-1))+0.5f) / (float) (N);

    // check MSB, return negative if 1
    return (_s & N) ? -r : r;
}

