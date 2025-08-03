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

// temporary shim to support backwards compatibility between "modemcf" and "modem"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "liquid.internal.h"

modem modem_create(modulation_scheme _scheme)
    { return modemcf_create(_scheme); }

modem modem_create_arbitrary(float complex * _table, unsigned int _M)
    { return modemcf_create_arbitrary(_table, _M); }

modem modem_recreate(modem _q, modulation_scheme _scheme)
    { return modemcf_recreate(_q, _scheme); }

int modem_destroy(modem _q)
    { return modemcf_destroy(_q); }

int modem_print(modem _q)
    { return modemcf_print(_q); }

int modem_reset(modem _q)
    { return modemcf_reset(_q); }

unsigned int modem_gen_rand_sym(modem _q)
    { return modemcf_gen_rand_sym(_q); }

unsigned int modem_get_bps(modem _q)
    { return modemcf_get_bps(_q); }

modulation_scheme modem_get_scheme(modem _q)
    { return modemcf_get_scheme(_q); }

int modem_modulate(modem _q, unsigned int _s, float complex * _y)
    { return modemcf_modulate(_q, _s, _y); }

int modem_demodulate(modem _q, float complex _x, unsigned int * _s)
    { return modemcf_demodulate(_q, _x, _s); }

int modem_demodulate_soft(modem _q, float complex _x,
        unsigned int * _s, unsigned char * _soft_bits)
    { return modemcf_demodulate_soft(_q, _x, _s, _soft_bits); }

int modem_get_demodulator_sample(modem _q, float complex * _x_hat)
    { return modemcf_get_demodulator_sample(_q, _x_hat); }

float modem_get_demodulator_phase_error(modem _q)
    { return modemcf_get_demodulator_phase_error(_q); }

float modem_get_demodulator_evm(modem _q)
    { return modemcf_get_demodulator_evm(_q); }

