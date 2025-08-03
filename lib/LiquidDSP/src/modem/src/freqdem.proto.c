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
// Frequency demodulator
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "liquid.internal.h"

// freqdem
struct FREQDEM(_s) {
    // common
    float kf;   // modulation index
    T     ref;  // 1/(2*pi*kf)

    TC r_prime; // previous received sample
};

// create freqdem object
//  _kf     :   modulation factor
FREQDEM() FREQDEM(_create)(float _kf)
{
    // validate input
    if (_kf <= 0.0f)
        return liquid_error_config("freqdem%s_create(), modulation factor %12.4e must be greater than 0", EXTENSION, _kf);

    // create main object memory
    FREQDEM() q = (freqdem) malloc(sizeof(struct FREQDEM(_s)));

    // set internal modulation factor
    q->kf = _kf;

    // compute derived values
    q->ref = 1.0f / (2*M_PI*q->kf);

    // reset modem object
    FREQDEM(_reset)(q);

    // return object
    return q;
}

// destroy modem object
int FREQDEM(_destroy)(FREQDEM() _q)
{
    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print modulation internals
int FREQDEM(_print)(FREQDEM() _q)
{
    printf("<liquid.freqdem, mod_factor=%g>\n", _q->kf);
    return LIQUID_OK;
}

// reset modem object
int FREQDEM(_reset)(FREQDEM() _q)
{
    // clear complex phase term
    _q->r_prime = 0;
    return LIQUID_OK;
}

// demodulate sample
//  _q      :   FM demodulator object
//  _r      :   received signal
//  _m      :   output message signal
int FREQDEM(_demodulate)(FREQDEM() _q,
                         TC        _r,
                         T *       _m)
{
    // compute phase difference and normalize by modulation index
    *_m = cargf( conjf(_q->r_prime)*_r ) * _q->ref;

    // save previous input sample
    _q->r_prime = _r;
    return LIQUID_OK;
}

// demodulate block of samples
//  _q      :   frequency demodulator object
//  _r      :   received signal r(t) [size: _n x 1]
//  _n      :   number of input, output samples
//  _m      :   message signal m(t), [size: _n x 1]
int FREQDEM(_demodulate_block)(FREQDEM()    _q,
                               TC *         _r,
                               unsigned int _n,
                               T *          _m)
{
    // TODO: implement more efficient method
    unsigned int i;
    for (i=0; i<_n; i++)
        FREQDEM(_demodulate)(_q, _r[i], &_m[i]);
    return LIQUID_OK;
}

