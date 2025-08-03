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
// modem_ook.c
//

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

// create an ook (on/off keying) modem object
MODEM() MODEM(_create_ook)()
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    q->scheme = LIQUID_MODEM_OOK;

    MODEM(_init)(q, 1);

    q->modulate_func   = &MODEM(_modulate_ook);
    q->demodulate_func = &MODEM(_demodulate_ook);

    // reset and return
    MODEM(_reset)(q);
    return q;
}

// modulate symbol using on/off keying
int MODEM(_modulate_ook)(MODEM()         _q,
                         unsigned int    _sym_in,
                         float complex * _y)
{
    // compute output sample directly from input
    *_y = _sym_in ? 0.0f : M_SQRT2;
    return LIQUID_OK;
}

// demodulate OOK
int MODEM(_demodulate_ook)(MODEM()        _q,
                           float complex  _x,
                           unsigned int * _sym_out)
{
    // slice directly to output symbol
    *_sym_out = (crealf(_x) > M_SQRT1_2 ) ? 0 : 1;

    // re-modulate symbol and store state
    MODEM(_modulate_ook)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
    return LIQUID_OK;
}

