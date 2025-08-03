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
// modem_qpsk.c
//

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

// create a qpsk (quaternary phase-shift keying) modem object
MODEM() MODEM(_create_qpsk)()
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    q->scheme = LIQUID_MODEM_QPSK;

    MODEM(_init)(q, 2);

    q->modulate_func   = &MODEM(_modulate_qpsk);
    q->demodulate_func = &MODEM(_demodulate_qpsk);

    // reset and return
    MODEM(_reset)(q);
    return q;
}


// modulate QPSK
int MODEM(_modulate_qpsk)(MODEM()      _q,
                           unsigned int _sym_in,
                           TC *         _y)
{
    // compute output sample directly from input
    *_y  = (_sym_in & 0x01 ? -M_SQRT1_2 : M_SQRT1_2) +
           (_sym_in & 0x02 ? -M_SQRT1_2 : M_SQRT1_2)*_Complex_I;
    return LIQUID_OK;
}

// demodulate QPSK
int MODEM(_demodulate_qpsk)(MODEM() _q,
                            TC _x,
                            unsigned int * _sym_out)
{
    // slice directly to output symbol
    *_sym_out  = (crealf(_x) > 0 ? 0 : 1) +
                 (cimagf(_x) > 0 ? 0 : 2);

    // re-modulate symbol and store state
    MODEM(_modulate_qpsk)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
    return LIQUID_OK;
}

// demodulate QPSK (soft)
int MODEM(_demodulate_soft_qpsk)(MODEM()         _q,
                                  TC              _x,
                                  unsigned int  * _s,
                                  unsigned char * _soft_bits)
{
    // gamma = 1/(2*sigma^2), approximate for constellation size
    T gamma = 5.8f;

    // approximate log-likelihood ratios
    T LLR;
    int soft_bit;
    
    // compute soft value for first bit
    LLR = -2.0f * cimagf(_x) * gamma;
    soft_bit = LLR*16 + 127;
    if (soft_bit > 255) soft_bit = 255;
    if (soft_bit <   0) soft_bit = 0;
    _soft_bits[0] = (unsigned char) ( soft_bit );

    // compute soft value for first bit
    LLR = -2.0f * crealf(_x) * gamma;
    soft_bit = LLR*16 + 127;
    if (soft_bit > 255) soft_bit = 255;
    if (soft_bit <   0) soft_bit = 0;
    _soft_bits[1] = (unsigned char) ( soft_bit );

    // re-modulate symbol and store state
    *_s  = (crealf(_x) > 0 ? 0 : 1) +
           (cimagf(_x) > 0 ? 0 : 2);
    MODEM(_modulate_qpsk)(_q, *_s, &_q->x_hat);
    _q->r = _x;
    return LIQUID_OK;
}

