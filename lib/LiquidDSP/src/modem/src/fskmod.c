/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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

// M-ary frequency-shift keying modulator

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// fskmod
struct fskmod_s {
    // common
    unsigned int m;             // bits per symbol
    unsigned int k;             // samples per symbol
    float        bandwidth;     // filter bandwidth parameter

    unsigned int M;             // constellation size
    float        M2;            // (M-1)/2
    nco_crcf     oscillator;    // nco
};

// create fskmod object (frequency modulator)
//  _m          :   bits per symbol, _m > 0
//  _k          :   samples/symbol, _k >= 2^_m
//  _bandwidth  :   total signal bandwidth, (0,0.5)
fskmod fskmod_create(unsigned int _m,
                     unsigned int _k,
                     float        _bandwidth)
{
    // validate input
    if (_m == 0)
        return liquid_error_config("fskmod_create(), bits/symbol must be greater than 0");
    if (_k < 2 || _k > 2048)
        return liquid_error_config("fskmod_create(), samples/symbol must be in [2^_m, 2048]");
    if (_bandwidth <= 0.0f || _bandwidth >= 0.5f)
        return liquid_error_config("fskmod_create(), bandwidth must be in (0,0.5)");

    // create main object memory
    fskmod q = (fskmod) malloc(sizeof(struct fskmod_s));

    // set basic internal properties
    q->m         = _m;              // bits per symbol
    q->k         = _k;              // samples per symbol
    q->bandwidth = _bandwidth;      // signal bandwidth

    // derived values
    q->M  = 1 << q->m;              // constellation size
    q->M2 = 0.5f*(float)(q->M-1);   // (M-1)/2

    q->oscillator = nco_crcf_create(LIQUID_VCO);

    // reset modem object
    fskmod_reset(q);

    // return main object
    return q;
}

// copy object
fskmod fskmod_copy(fskmod q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("fskmod_copy(), object cannot be NULL");

    // create object and copy base parameters
    fskmod q_copy = (fskmod) malloc(sizeof(struct fskmod_s));
    memmove(q_copy, q_orig, sizeof(struct fskmod_s));

    // copy oscillator object
    q_copy->oscillator = nco_crcf_copy(q_orig->oscillator);

    // return new object
    return q_copy;
}

// destroy fskmod object
int fskmod_destroy(fskmod _q)
{
    // destroy oscillator object
    nco_crcf_destroy(_q->oscillator);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print fskmod object internals
int fskmod_print(fskmod _q)
{
    printf("<liquid.fskmod");
    printf(", bits/symbol=%u", _q->m);
    printf(", samples/symbol=%u", _q->k);
    printf(", bandwidth=%g", _q->bandwidth);
    printf(">\n");
    return LIQUID_OK;
}

// reset state
int fskmod_reset(fskmod _q)
{
    // reset internal objects
    nco_crcf_reset(_q->oscillator);
    return LIQUID_OK;
}

// modulate sample
//  _q      :   frequency modulator object
//  _s      :   input symbol
//  _y      :   output sample array [size: _k x 1]
int fskmod_modulate(fskmod          _q,
                    unsigned int    _s,
                    float complex * _y)
{
    // validate input
    if (_s >= _q->M)
        return liquid_error(LIQUID_EIRANGE,"fskmod_modulate(), input symbol (%u) exceeds maximum (%u)",_s, _q->M);

    // compute appropriate frequency
    float dphi = ((float)_s - _q->M2) * 2 * M_PI * _q->bandwidth / _q->M2;

    // set frequency appropriately
    nco_crcf_set_frequency(_q->oscillator, dphi);

    // generate output tone
    unsigned int i;
    for (i=0; i<_q->k; i++) {
        // compute complex output
        nco_crcf_cexpf(_q->oscillator, &_y[i]);
        
        // step oscillator
        nco_crcf_step(_q->oscillator);
    }
    return LIQUID_OK;
}

