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
// fdelay : arbitrary delay filter
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct FDELAY(_s) {
    unsigned int    nmax;       // maximum delay allowed
    unsigned int    m;          // filter semi-length
    unsigned int    npfb;       // number of filters in polyphase filter-bank
    float           delay;      // current delay
    WINDOW()        w;          // window for bulk delay
    FIRPFB()        pfb;        // polyphase filter-bank for fractional delay
    unsigned int    w_index;    // index: window (bulk delay)
    unsigned int    f_index;    // index: polyphase filter-bank (fractional delay)
};

// Create a delay object with a maximum offset and filter specification
//  _nmax   : maximum integer sample offset
//  _m      : polyphase filter-bank semi-length, _m > 0
//  _npfb   : number of filters in polyphase filter-bank, _npfb > 0
FDELAY() FDELAY(_create)(unsigned int _nmax,
                         unsigned int _m,
                         unsigned int _npfb)
{
    // validate input
    if (_nmax == 0)
        return liquid_error_config("fdelay_%s_create(), max delay must be greater than 0", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("fdelay_%s_create(), filter semi-length must be greater than 0", EXTENSION_FULL);
    if (_npfb == 0)
        return liquid_error_config("fdelay_%s_create(), number of filters must be greater than 0", EXTENSION_FULL);

    // create filter object and initialize
    FDELAY() q = (FDELAY()) malloc(sizeof(struct FDELAY(_s)));
    q->nmax = _nmax;
    q->m    = _m;
    q->npfb = _npfb;

    // create window (internal buffer); provision 1 extra sample to account
    // for additional delay
    q->w = WINDOW(_create)(q->nmax + 1);

    // create polyphase filter-bank
    q->pfb = FIRPFB(_create_default)(q->npfb, q->m);

    // reset filter state (clear buffer)
    FDELAY(_reset)(q);
    return q;
}

// Create a delay object with a maximum offset and default filter
// parameters (_m = 8, _npfb = 64)
//  _nmax   : maximum integer sample offset
FDELAY() FDELAY(_create_default)(unsigned int _nmax)
{
    return FDELAY(_create)(_nmax, 8, 64);
}

// Destroy delay object and free all internal memory
int FDELAY(_destroy)(FDELAY() _q)
{
    WINDOW(_destroy)(_q->w);
    FIRPFB(_destroy)(_q->pfb);
    free(_q);
    return LIQUID_OK;
}

// Reset delay object internals
int FDELAY(_reset)(FDELAY() _q)
{
    _q->delay = 0.0f;
    _q->w_index = _q->nmax-1;
    _q->f_index = 0;
    WINDOW(_reset)(_q->w);
    FIRPFB(_reset)(_q->pfb);
    return LIQUID_OK;
}

// Print delay object internals
int FDELAY(_print)(FDELAY() _q)
{
    printf("<liquid.fdelay, nmax=%u, m=%u, npfb=%u, delay=%.6f>\n",
            _q->nmax, _q->m, _q->npfb, _q->delay);
    return LIQUID_OK;
}

// Get current delay (accounting for _m?)
float FDELAY(_get_delay)(FDELAY() _q)
{
    return _q->delay;
}

int FDELAY(_set_delay)(FDELAY() _q,
                       float    _delay)
{
    if (_delay < 0) {
        return liquid_error(LIQUID_EIVAL,"fdelay_%s_set_delay(), delay (%g) cannot be negative",
                EXTENSION_FULL, _delay);
    } else if (_delay > _q->nmax) {
        return liquid_error(LIQUID_EIVAL,"fdelay_%s_set_delay(), delay (%g) cannot exceed maximum (%u)",
                EXTENSION_FULL, _delay, _q->nmax);
    }

    // compute offset from delay and integer/fractional components
    float offset   = (float)(_q->nmax) - _delay;
    int   intpart  = (int) floorf(offset);
    float fracpart = offset - (float)intpart;

    // set indices appropriately
    _q->w_index = intpart;
    _q->f_index = (unsigned int)roundf(_q->npfb * fracpart);
    while (_q->f_index >= _q->npfb) {
        _q->w_index++;
        _q->f_index -= _q->npfb;
    }
    // ensure valid range; clip if needed. Note that w_index can be equal
    // to nmax because the window was provisioned for nmax+1
    if (_q->w_index > _q->nmax)
        return liquid_error(LIQUID_EINT,"fdelay_%s_set_delay(), window index exceeds maximum", EXTENSION_FULL);
#if 0
    // debug
    printf("delay:%f -> offset:%f -> %d + %f -> %d + (%u/%u)\n",
           _delay, offset, intpart, fracpart, _q->w_index, _q->f_index, _q->npfb);
#endif
    _q->delay = _delay;
    return LIQUID_OK;
}

int FDELAY(_adjust_delay)(FDELAY() _q, float _delta)
{
    return FDELAY(_set_delay)(_q, _q->delay + _delta);
}

unsigned int FDELAY(_get_nmax)(FDELAY() _q)
{
    return _q->nmax;
}

unsigned int FDELAY(_get_m)(FDELAY() _q)
{
    return _q->m;
}

unsigned int FDELAY(_get_npfb)(FDELAY() _q)
{
    return _q->npfb;
}

// Push sample into filter object's internal buffer
//  _q      : filter object
//  _x      : single input sample
int FDELAY(_push)(FDELAY() _q,
                  TI       _x)
{
    // apply bulk delay
    WINDOW(_push)(_q->w, _x);
    TO v;
    WINDOW(_index)(_q->w, _q->w_index, &v);

    // apply fractional delay
    FIRPFB(_push)(_q->pfb, v);

    return LIQUID_OK;
}

// Write block of samplex into filter object's internal buffer
//  _q      : filter object
//  _x      : buffer of input samples, [size: _n x 1]
//  _n      : number of input samples
int FDELAY(_write)(FDELAY()     _q,
                   TI *         _x,
                   unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        if ( FDELAY(_push)(_q, _x[i]) != LIQUID_OK) {
            return liquid_error(LIQUID_EINT,"fdelay_%s_write(), could not write sample", EXTENSION_FULL);
        }
    }
    return LIQUID_OK;
}

// Execute vector dot product on the filter's internal buffer and
// coefficients
//  _q      : filter object
//  _y      : pointer to single output sample
int FDELAY(_execute)(FDELAY() _q,
                     TO *     _y)
{
    FIRPFB(_execute)(_q->pfb, _q->f_index, _y);
    return LIQUID_OK;
}

// Execute the filter on a block of input samples; in-place operation
// is permitted (_x and _y may point to the same place in memory)
//  _q      : filter object
//  _x      : pointer to input array, [size: _n x 1]
//  _n      : number of input, output samples
//  _y      : pointer to output array, [size: _n x 1]
int FDELAY(_execute_block)(FDELAY()     _q,
                           TI *         _x,
                           unsigned int _n,
                           TO *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        FDELAY(_push)(_q, _x[i]);
        FDELAY(_execute)(_q, _y+i);
    }
    return LIQUID_OK;
}

