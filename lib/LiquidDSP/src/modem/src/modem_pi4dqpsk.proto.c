/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
// modem_pi4dqpsk.c
//

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

// create a pi/4-DQPSK (pi/4 differential quaternary phase-shift keying) modem object
MODEM() MODEM(_create_pi4dqpsk)()
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    q->scheme = LIQUID_MODEM_PI4DQPSK;

    MODEM(_init)(q, 2);

    q->modulate_func   = &MODEM(_modulate_pi4dqpsk);
    q->demodulate_func = &MODEM(_demodulate_pi4dqpsk);

    // reset and return
    MODEM(_reset)(q);
    return q;
}

// modulate PI4DQPSK
int MODEM(_modulate_pi4dqpsk)(MODEM()      _q,
                              unsigned int _sym_in,
                              TC *         _y)
{
    float d_theta = 0.0f;
    switch (_sym_in) {
    case 0: d_theta = +1*0.25*M_PI; break;
    case 1: d_theta = +3*0.25*M_PI; break;
    case 2: d_theta = -1*0.25*M_PI; break;
    case 3: d_theta = -3*0.25*M_PI; break;
    default:;
    }

    // accumulate phase
    _q->data.pi4dqpsk.theta += d_theta;

    // constrain phase
    if (_q->data.pi4dqpsk.theta >  M_PI) _q->data.pi4dqpsk.theta -= 2*M_PI;
    if (_q->data.pi4dqpsk.theta < -M_PI) _q->data.pi4dqpsk.theta += 2*M_PI;

    // compute output symbol and return
    *_y = liquid_cexpjf(_q->data.pi4dqpsk.theta);
    return LIQUID_OK;
}

// demodulate PI4DQPSK
int MODEM(_demodulate_pi4dqpsk)(MODEM()        _q,
                                TC             _x,
                                unsigned int * _sym_out)
{
    // compute input phase
    T theta = cargf(_x);

    // compute differential phase
    T d_theta = theta - _q->data.pi4dqpsk.theta;
    while (d_theta >  M_PI) d_theta -= 2*M_PI;
    while (d_theta < -M_PI) d_theta += 2*M_PI;

    // slice directly to output symbol
    if      (d_theta >  0.5f*M_PI) *_sym_out = 1;
    else if (d_theta >          0) *_sym_out = 0;
    else if (d_theta < -0.5f*M_PI) *_sym_out = 3;
    else                           *_sym_out = 2;

    // re-modulate symbol and store state
    T d_theta_ideal = 0.0f;
    switch (*_sym_out) {
    case 0: d_theta_ideal = +1*0.25*M_PI; break;
    case 1: d_theta_ideal = +3*0.25*M_PI; break;
    case 2: d_theta_ideal = -1*0.25*M_PI; break;
    case 3: d_theta_ideal = -3*0.25*M_PI; break;
    default:;
    }
    _q->x_hat = liquid_cexpjf(_q->data.pi4dqpsk.theta + d_theta_ideal);
    _q->r = _x;
    _q->data.pi4dqpsk.theta = theta;

    return LIQUID_OK;
}

// demodulate pi/4 differential QPSK (soft)
int MODEM(_demodulate_soft_pi4dqpsk)(MODEM()         _q,
                                     TC              _x,
                                     unsigned int  * _s,
                                     unsigned char * _soft_bits)
{
    // hard demod
    MODEM(_demodulate_pi4dqpsk)(_q, _x, _s);

    // convert to soft values
    _soft_bits[0] = *_s & 2 ? 255 : 0;
    _soft_bits[1] = *_s & 1 ? 255 : 0;

    return LIQUID_OK;
}

