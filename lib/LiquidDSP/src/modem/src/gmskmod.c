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

// Gauss minimum-shift keying modem

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

struct gmskmod_s {
    unsigned int k;         // samples/symbol
    unsigned int m;         // symbol delay
    float        BT;        // bandwidth/time product
    unsigned int h_len;     // filter length
    float *      h;         // pulse shaping filter

    // interpolator
    firinterp_rrrf interp_tx;

    float theta;            // phase state
    float k_inv;            // 1/k
};

// create gmskmod object
//  _k      :   samples/symbol
//  _m      :   filter delay (symbols)
//  _BT     :   excess bandwidth factor
gmskmod gmskmod_create(unsigned int _k,
                       unsigned int _m,
                       float        _BT)
{
    if (_k < 2)
        return liquid_error_config("gmskmod_create(), samples/symbol must be at least 2");
    if (_m < 1)
        return liquid_error_config("gmskmod_create(), symbol delay must be at least 1");
    if (_BT <= 0.0f || _BT >= 1.0f)
        return liquid_error_config("gmskmod_create(), bandwidth/time product must be in (0,1)");

    gmskmod q = (gmskmod)malloc(sizeof(struct gmskmod_s));

    // set properties
    q->k  = _k;
    q->m  = _m;
    q->BT = _BT;

    // derived values
    q->k_inv = 1.0f / (float)(q->k);

    // allocate memory for filter taps
    q->h_len = 2*(q->k)*(q->m)+1;
    q->h = (float*) malloc(q->h_len * sizeof(float));

    // compute filter coefficients
    liquid_firdes_gmsktx(q->k, q->m, q->BT, 0.0f, q->h);

    // create interpolator object
    q->interp_tx = firinterp_rrrf_create_prototype(LIQUID_FIRFILT_GMSKTX, q->k, q->m, q->BT, 0);

    // reset modem state
    gmskmod_reset(q);

    // return modem object
    return q;
}

// copy object
gmskmod gmskmod_copy(gmskmod q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("gmskmod_copy(), object cannot be NULL");

    // create object and copy base parameters
    gmskmod q_copy = (gmskmod) malloc(sizeof(struct gmskmod_s));
    memmove(q_copy, q_orig, sizeof(struct gmskmod_s));

    // copy filter coefficients
    q_copy->h = (float *) liquid_malloc_copy(q_orig->h, q_orig->h_len, sizeof(float));

    // copy interpolator object
    q_copy->interp_tx = firinterp_rrrf_copy(q_orig->interp_tx);

    // return new object
    return q_copy;
}

int gmskmod_destroy(gmskmod _q)
{
    // destroy interpolator
    firinterp_rrrf_destroy(_q->interp_tx);

    // free transmit filter array
    free(_q->h);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

int gmskmod_print(gmskmod _q)
{
    printf("<liquid.gmskmod, k=%u, m=%u, BT=%g>\n", _q->k, _q->m, _q->BT);
#if 0
    unsigned int i;
    for (i=0; i<_q->h_len; i++)
        printf("  ht(%4u) = %12.8f;\n", i+1, _q->h[i]);
#endif
    return LIQUID_OK;
}

int gmskmod_reset(gmskmod _q)
{
    // reset phase state
    _q->theta = 0.0f;

    // clear interpolator buffer
    firinterp_rrrf_reset(_q->interp_tx);
    return LIQUID_OK;
}

int gmskmod_modulate(gmskmod         _q,
                     unsigned int    _s,
                     float complex * _y)
{
    // generate sample from symbol
    float x = _s==0 ? -_q->k_inv : _q->k_inv;

    // run interpolator
    float phi[_q->k];
    firinterp_rrrf_execute(_q->interp_tx, x, phi);

    // integrate phase state
    unsigned int i;
    for (i=0; i<_q->k; i++) {
        // integrate phase state
        _q->theta += phi[i];

        // ensure phase in [-pi, pi]
        if (_q->theta >  M_PI) _q->theta -= 2*M_PI;
        if (_q->theta < -M_PI) _q->theta += 2*M_PI;

        // compute output
        _y[i] = liquid_cexpjf(_q->theta);
    }
    return LIQUID_OK;
}

