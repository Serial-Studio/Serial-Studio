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
// modem_bpsk.c : specific BPSK modem
//

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

// create a bpsk (binary phase-shift keying) modem object
MODEM() MODEM(_create_bpsk)()
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    q->scheme = LIQUID_MODEM_BPSK;

    MODEM(_init)(q, 1);

    q->modulate_func   = &MODEM(_modulate_bpsk);
    q->demodulate_func = &MODEM(_demodulate_bpsk);

    // reset and return
    MODEM(_reset)(q);
    return q;
}

// modulate BPSK
int MODEM(_modulate_bpsk)(MODEM()      _q,
                          unsigned int _sym_in,
                          TC *         _y)
{
    // compute output sample directly from input
    *_y = _sym_in ? -1.0f : 1.0f;
    return LIQUID_OK;
}

// demodulate BPSK
int MODEM(_demodulate_bpsk)(MODEM()        _q,
                             TC             _x,
                             unsigned int * _sym_out)
{
    // slice directly to output symbol
    *_sym_out = (crealf(_x) > 0 ) ? 0 : 1;

    // re-modulate symbol and store state
    MODEM(_modulate_bpsk)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
    return LIQUID_OK;
}

// demodulate BPSK (soft)
int MODEM(_demodulate_soft_bpsk)(MODEM()         _q,
                                 TC              _x,
                                 unsigned int  * _s,
                                 unsigned char * _soft_bits)
{
    // gamma = 1/(2*sigma^2), approximate for constellation size
    T gamma = 4.0f;

    // approximate log-likelihood ratio
    T LLR = -2.0f * crealf(_x) * gamma;
    int soft_bit = LLR*16 + 127;
    if (soft_bit > 255) soft_bit = 255;
    if (soft_bit <   0) soft_bit = 0;
    _soft_bits[0] = (unsigned char) ( soft_bit );

    // re-modulate symbol and store state
    unsigned int symbol_out = (crealf(_x) > 0 ) ? 0 : 1;
    MODEM(_modulate_bpsk)(_q, symbol_out, &_q->x_hat);
    _q->r = _x;
    *_s = symbol_out;
    return LIQUID_OK;
}

