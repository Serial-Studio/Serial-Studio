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
// Frequency modulator
//

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "liquid.internal.h"

// freqmod
struct FREQMOD(_s) {
    float kf;   // modulation factor for FM
    T     ref;  // phase reference: kf*2^16

    // look-up table
    unsigned int sincos_table_len;      // table length: 10 bits
    uint16_t     sincos_table_phase;    // accumulated phase: 16 bits
    TC *         sincos_table;          // sin|cos look-up table: 2^10 entries
};

// create freqmod object
//  _kf     :   modulation factor
FREQMOD() FREQMOD(_create)(float _kf)
{
    // validate input
    if (_kf <= 0.0f)
        return liquid_error_config("freqmod%s_create(), modulation factor %12.4e must be greater than 0", EXTENSION, _kf);

    // create main object memory
    FREQMOD() q = (freqmod) malloc(sizeof(struct FREQMOD(_s)));

    // set modulation factor
    q->kf  = _kf;
    q->ref = q->kf * (1<<16);

    // initialize look-up table
    q->sincos_table_len = 1024;
    q->sincos_table     = (TC*) malloc( q->sincos_table_len*sizeof(TC) );
    unsigned int i;
    for (i=0; i<q->sincos_table_len; i++) {
        q->sincos_table[i] = cexpf(_Complex_I*2*M_PI*(float)i / (float)(q->sincos_table_len) );
    }

    // reset modem object
    FREQMOD(_reset)(q);

    // return object
    return q;
}

// destroy modem object
int FREQMOD(_destroy)(FREQMOD() _q)
{
    // free table
    free(_q->sincos_table);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print modulation internals
int FREQMOD(_print)(FREQMOD() _q)
{
    printf("<liquid.freqmod");
    printf(", mod_factor=%g", _q->kf);
    printf(", tablen=%u",    _q->sincos_table_len);
    printf(">\n");
    return LIQUID_OK;
}

// reset modem object
int FREQMOD(_reset)(FREQMOD() _q)
{
    // reset phase accumulation
    _q->sincos_table_phase = 0;
    return LIQUID_OK;
}

// modulate sample
//  _q      :   frequency modulator object
//  _m      :   message signal m(t)
//  _s      :   complex baseband signal s(t)
int FREQMOD(_modulate)(FREQMOD()   _q,
                       T           _m,
                       TC *        _s)
{
    // accumulate phase; this wraps around a 16-bit boundary and ensures
    // that negative numbers are mapped to positive numbers
    _q->sincos_table_phase =
        (_q->sincos_table_phase + (1<<16) + (int)roundf(_q->ref*_m)) & 0xffff;

    // compute table index: mask out 10 most significant bits with rounding
    // (adding 0x0020 effectively rounds to nearest value with 10 bits of
    // precision)
    unsigned int index = ( (_q->sincos_table_phase+0x0020) >> 6) & 0x03ff;

    // return table value at index
    *_s = _q->sincos_table[index];
    return LIQUID_OK;
}

// modulate block of samples
//  _q      :   frequency modulator object
//  _m      :   message signal m(t), [size: _n x 1]
//  _n      :   number of input, output samples
//  _s      :   complex baseband signal s(t) [size: _n x 1]
int FREQMOD(_modulate_block)(FREQMOD()    _q,
                             T *          _m,
                             unsigned int _n,
                             TC *         _s)
{
    // TODO: implement more efficient method
    unsigned int i;
    for (i=0; i<_n; i++)
        FREQMOD(_modulate)(_q, _m[i], &_s[i]);
    return LIQUID_OK;
}

