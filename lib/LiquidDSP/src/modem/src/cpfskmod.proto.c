/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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

// continuous phase frequency-shift keying modulator

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "liquid.internal.h"

// design transmit filter
int CPFSKMOD(_firdes)(unsigned int _k,
                      unsigned int _m,
                      float        _beta,
                      int          _type,
                      float *      _h,
                      unsigned int _h_len);

struct CPFSKMOD(_s)
{
    // common
    unsigned int bps;           // bits per symbol
    unsigned int k;             // samples per symbol
    unsigned int m;             // filter delay (symbols)
    float        beta;          // filter bandwidth parameter
    float        h;             // modulation index
    int          type;          // filter type (e.g. LIQUID_CPFSK_SQUARE)
    unsigned int M;             // constellation size
    unsigned int symbol_delay;  // transmit filter delay [symbols]

    // pulse-shaping filter
    T *          ht;            // filter coefficients
    unsigned int ht_len;        // filter length
    firinterp_rrrf  interp;     // phase interpolator

    // phase integrator
    T * phase_interp;           // phase interpolation buffer
    T b0, b1, a1, v0, v1;       // integrator
};

// create CPFSKMOD() object (frequency modulator)
//  _bps    :   bits per symbol, _bps > 0
//  _h      :   modulation index, _h > 0
//  _k      :   samples/symbol, _k > 1, _k even
//  _m      :   filter delay (symbols), _m > 0
//  _beta   :   filter bandwidth parameter, _beta > 0
//  _type   :   filter type (e.g. LIQUID_CPFSK_SQUARE)
CPFSKMOD() CPFSKMOD(_create)(unsigned int _bps,
                             float        _h,
                             unsigned int _k,
                             unsigned int _m,
                             float        _beta,
                             int          _type)
{
    // validate input
    if (_bps == 0)
        return liquid_error_config("cpfskmod_create(), bits/symbol must be greater than 0");
    if (_h <= 0.0f)
        return liquid_error_config("cpfskmod_create(), modulation index must be greater than 0");
    if (_k < 2 || (_k%2))
        return liquid_error_config("cpfskmod_create(), samples/symbol must be greater than 2 and even");
    if (_m == 0)
        return liquid_error_config("cpfskmod_create(), filter delay must be greater than 0");
    if (_beta <= 0.0f || _beta > 1.0f)
        return liquid_error_config("cpfskmod_create(), filter roll-off must be in (0,1]");

    switch(_type) {
    case LIQUID_CPFSK_SQUARE:
    case LIQUID_CPFSK_RCOS_FULL:
    case LIQUID_CPFSK_RCOS_PARTIAL:
    case LIQUID_CPFSK_GMSK:
        break;
    default:
        return liquid_error_config("cpfskmod_create(), invalid filter type '%d'", _type);
    }

    // create main object memory
    CPFSKMOD() q = (CPFSKMOD()) malloc(sizeof(struct CPFSKMOD(_s)));

    // set basic internal properties
    q->bps  = _bps;     // bits per symbol
    q->h    = _h;       // modulation index
    q->k    = _k;       // samples per symbol
    q->m    = _m;       // filter delay (symbols)
    q->beta = _beta;    // filter roll-off factor (only for certain filters)
    q->type = _type;    // filter type

    // derived values
    q->M = 1 << q->bps; // constellation size

    // create object depending upon input type
    q->b0 =  0.5f;
    q->b1 =  0.5f;
    q->a1 = -1.0f;
    q->ht_len = 0;
    q->ht = NULL;
    unsigned int i;
    switch(q->type) {
    case LIQUID_CPFSK_SQUARE:
        q->ht_len = q->k;
        q->symbol_delay = 1;
        // modify integrator
        q->b0 = 0.0f;
        q->b1 = 1.0f;
        break;
    case LIQUID_CPFSK_RCOS_FULL:
        q->ht_len = q->k;
        q->symbol_delay = 1;
        break;
    case LIQUID_CPFSK_RCOS_PARTIAL:
        // TODO: adjust response based on 'm'
        q->ht_len = 3*q->k;
        q->symbol_delay = 2;
        break;
    case LIQUID_CPFSK_GMSK:
        q->symbol_delay = q->m + 1;
        q->ht_len = 2*(q->k)*(q->m) + (q->k) + 1;
        break;
    default:
        return liquid_error_config("cpfskmod_create(), invalid filter type '%d'", q->type);
    }

    // create pulse-shaping filter and scale by modulation index
    q->ht = (T*) malloc(q->ht_len *sizeof(T));
    CPFSKMOD(_firdes)(q->k, q->m, q->beta, q->type, q->ht, q->ht_len);
    for (i=0; i<q->ht_len; i++)
        q->ht[i] *= M_PI * q->h;
    q->interp = firinterp_rrrf_create(q->k, q->ht, q->ht_len);

    // allocate buffer for phase interpolation
    q->phase_interp = (T*) malloc(q->k*sizeof(T));

    // reset modem object
    cpfskmod_reset(q);

    return q;
}

// create modulator object for minimum-shift keying
//  _k      : samples/symbol, _k > 1, _k even
CPFSKMOD() CPFSKMOD(_create_msk)(unsigned int _k)
{
    return CPFSKMOD(_create)(1, 0.5f, _k, 1, 1.0f, LIQUID_CPFSK_SQUARE);
}

// create modulator object for minimum-shift keying
//  _k      : samples/symbol, _k > 1, _k even
//  _m      : filter delay (symbols), _m > 0
//  _BT     : bandwidth-time factor, 0 < _BT < 1
CPFSKMOD() CPFSKMOD(_create_gmsk)(unsigned int _k,
                                  unsigned int _m,
                                  float        _BT)
{
    return CPFSKMOD(_create)(1, 0.5f, _k, _m, _BT, LIQUID_CPFSK_GMSK);
}

// Copy object including all internal objects and state
CPFSKMOD() CPFSKMOD(_copy)(CPFSKMOD() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("cpfskmod_copy(), object cannot be NULL");

    // create filter object and copy base parameters
    CPFSKMOD() q_copy = (CPFSKMOD()) malloc(sizeof(struct CPFSKMOD(_s)));
    memmove(q_copy, q_orig, sizeof(struct CPFSKMOD(_s)));

    // copy objects, arrays
    q_copy->interp = firinterp_rrrf_copy(q_orig->interp);
    q_copy->ht           = (T*) liquid_malloc_copy(q_orig->ht,           q_orig->ht_len, sizeof(T));
    q_copy->phase_interp = (T*) liquid_malloc_copy(q_orig->phase_interp, q_orig->k,      sizeof(T));

    // return new object
    return q_copy;
}

// destroy modulator object
int CPFSKMOD(_destroy)(CPFSKMOD() _q)
{
    // destroy pulse-shaping filter/interpolator
    free(_q->ht);
    free(_q->phase_interp);
    firinterp_rrrf_destroy(_q->interp);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print modulator object internals
int CPFSKMOD(_print)(CPFSKMOD() _q)
{
    printf("<liquid.cpfskmod, bps=%u, h=%g, sps=%u, m=%u, beta=%g",
        _q->bps, _q->h, _q->k, _q->m, _q->beta);
    switch(_q->type) {
    case LIQUID_CPFSK_SQUARE:       printf(", type=\"square\"");       break;
    case LIQUID_CPFSK_RCOS_FULL:    printf(", type=\"rcos-full\"");    break;
    case LIQUID_CPFSK_RCOS_PARTIAL: printf(", type=\"rcos-partial\""); break;
    case LIQUID_CPFSK_GMSK:         printf(", type=\"gmsk\"");         break;
    default:;
    }
    printf(">\n");
    return LIQUID_OK;
}

// reset state
int CPFSKMOD(_reset)(CPFSKMOD() _q)
{
    // reset interpolator
    firinterp_rrrf_reset(_q->interp);

    // reset phase integrator
    _q->v0 = 0.0f;
    _q->v1 = 0.0f;
    return LIQUID_OK;
}

// Get modulator's number of bits per symbol
unsigned int CPFSKMOD(_get_bits_per_symbol)(CPFSKMOD() _q)
{
    return _q->bps;
}

// Get modulator's modulation index
float CPFSKMOD(_get_modulation_index)(CPFSKMOD() _q)
{
    return _q->h;
}

// Get modulator's number of samples per symbol
unsigned int CPFSKMOD(_get_samples_per_symbol)(CPFSKMOD() _q)
{
    return _q->k;
}

// Get modulator's filter delay [symbols]
unsigned int CPFSKMOD(_get_delay)(CPFSKMOD() _q)
{
    return _q->symbol_delay;
}

// Get modulator's bandwidth parameter
float CPFSKMOD(_get_beta)(CPFSKMOD() _q)
{
    return _q->beta;
}

// Get modulator's filter type
int CPFSKMOD(_get_type)(CPFSKMOD() _q)
{
    return _q->type;
}


// modulate sample
//  _q      :   frequency modulator object
//  _s      :   input symbol
//  _y      :   output sample array [size: _k x 1]
int CPFSKMOD(_modulate)(CPFSKMOD()     _q,
                        unsigned int   _s,
                        TC           * _y)
{
    // run interpolator
    float v = 2.0f*_s - (float)(_q->M) + 1.0f;
    firinterp_rrrf_execute(_q->interp, v, _q->phase_interp);

    // integrate phase state
    unsigned int i;
    float theta;
    for (i=0; i<_q->k; i++) {
        // push phase through integrator
        _q->v0 = _q->phase_interp[i] - _q->v1*_q->a1;
        theta  = _q->v0*_q->b0 + _q->v1*_q->b1;
        _q->v1 = _q->v0;

        // constrain state
        if (_q->v1 > 2*M_PI)
            _q->v1 -= 2*M_PI;
        if (_q->v1 < -2*M_PI)
            _q->v1 += 2*M_PI;

        // compute output
        _y[i] = liquid_cexpjf(theta);
    }
    return LIQUID_OK;
}

// 
// internal methods
//

// design transmit filter
int CPFSKMOD(_firdes)(unsigned int _k,
                      unsigned int _m,
                      float        _beta,
                      int          _type,
                      float *      _ht,
                      unsigned int _ht_len)
{
    unsigned int i;
    // create filter based on specified type
    switch(_type) {
    case LIQUID_CPFSK_SQUARE:
        // square pulse
        if (_ht_len != _k)
            return liquid_error(LIQUID_EICONFIG,"cpfskmodem_firdes(), invalid filter length (square)");
        for (i=0; i<_ht_len; i++)
            _ht[i] = 1.0f;
        break;
    case LIQUID_CPFSK_RCOS_FULL:
        // full-response raised-cosine pulse
        if (_ht_len != _k)
            return liquid_error(LIQUID_EICONFIG,"cpfskmodem_firdes(), invalid filter length (rcos)");
        for (i=0; i<_ht_len; i++)
            _ht[i] = 1.0f - cosf(2.0f*M_PI*i/(float)_ht_len);
        break;
    case LIQUID_CPFSK_RCOS_PARTIAL:
        // full-response raised-cosine pulse
        if (_ht_len != 3*_k)
            return liquid_error(LIQUID_EICONFIG,"cpfskmodem_firdes(), invalid filter length (rcos)");
        // initialize with zeros
        for (i=0; i<_ht_len; i++)
            _ht[i] = 0.0f;
        // adding raised-cosine pulse with half-symbol delay
        for (i=0; i<2*_k; i++)
            _ht[i+_k/2] = 1.0f - cosf(2.0f*M_PI*i/(float)(2*_k));
        break;
    case LIQUID_CPFSK_GMSK:
        // Gauss minimum-shift keying pulse
        if (_ht_len != 2*_k*_m + _k + 1)
            return liquid_error(LIQUID_EICONFIG,"cpfskmodem_firdes(), invalid filter length (gmsk)");
        // initialize with zeros
        for (i=0; i<_ht_len; i++)
            _ht[i] = 0.0f;
        // adding Gauss pulse with half-symbol delay
        liquid_firdes_gmsktx(_k,_m,_beta,0.0f,&_ht[_k/2]);
        break;
    default:
        return liquid_error(LIQUID_EINT,"cpfskmodem_firdes(), invalid filter type '%d'", _type);
    }

    // normalize pulse area to unity
    float ht_sum = 0.0f;
    for (i=0; i<_ht_len; i++)
        ht_sum += _ht[i];
    for (i=0; i<_ht_len; i++)
        _ht[i] *= 1.0f / ht_sum;

    return LIQUID_OK;
}

