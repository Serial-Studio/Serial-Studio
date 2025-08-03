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

// 
// CVSD: continuously variable slope delta
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"

// enable use of the pre-emphasis and post-emphasis filters
#define CVSD_ENABLE_SIGNAL_CONDITIONING (1)

struct cvsd_s {
    unsigned int num_bits;
    unsigned char bitref;   // historical bit reference
    unsigned char bitmask;  // historical bit reference mask
    float ref;              // internal reference

    float zeta;             // delta step factor
    float delta;            // current step size
    float delta_min;        // minimum delta
    float delta_max;        // maximum delta

#if CVSD_ENABLE_SIGNAL_CONDITIONING
    float alpha;            // pre-/de-emphasis filter coefficient
    float beta;             // DC-blocking coefficient (decoder)
    iirfilt_rrrf prefilt;   // pre-emphasis filter (encoder)
    iirfilt_rrrf postfilt;  // e-emphasis filter (decoder)
#endif
};

// create cvsd object
//  _num_bits   :   number of adjacent bits to observe
//  _zeta       :   slope adjustment multiplier
//  _alpha      :   pre-/post-emphasis filter coefficient (0.9 recommended)
// NOTE: _alpha must be in [0,1]
cvsd cvsd_create(unsigned int _num_bits,
                 float        _zeta,
                 float        _alpha)
{
    if (_num_bits == 0)
        return liquid_error_config("cvsd_create(), _num_bits must be positive");
    if (_zeta <= 1.0f)
        return liquid_error_config("cvsd_create(), zeta must be greater than 1");
    if (_alpha < 0.0f || _alpha > 1.0f)
        return liquid_error_config("cvsd_create(), alpha must be in [0,1]");

    cvsd q = (cvsd) malloc(sizeof(struct cvsd_s));
    q->num_bits = _num_bits;
    q->bitref = 0;
    q->bitmask = (1<<(q->num_bits)) - 1;

    q->ref = 0.0f;
    q->zeta = _zeta;
    q->delta = 0.01f;
    q->delta_min = 0.01f;
    q->delta_max = 1.0f;

#if CVSD_ENABLE_SIGNAL_CONDITIONING
    // design pre-emphasis filter
    q->alpha = _alpha;
    float b_pre[2] = {1.0f, -q->alpha};
    float a_pre[2] = {1.0f, 0.0f};
    q->prefilt  = iirfilt_rrrf_create(b_pre,2,a_pre,2);

    // design post-emphasis filter
    q->beta = 0.99f;    // DC-blocking parameter
    float b_post[3] = {1.0f, -1.0f, 0.0f};
    float a_post[3] = {1.0f, -(q->alpha + q->beta), q->alpha*q->beta};
    q->postfilt = iirfilt_rrrf_create(b_post,3,a_post,3);
#endif

    return q;
}

// destroy cvsd object
int cvsd_destroy(cvsd _q)
{
#if CVSD_ENABLE_SIGNAL_CONDITIONING
    // destroy filters
    iirfilt_rrrf_destroy(_q->prefilt);
    iirfilt_rrrf_destroy(_q->postfilt);
#endif

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print cvsd object parameters
int cvsd_print(cvsd _q)
{
    printf("<liquid.cvsd, bits=%u, zeta=%g", _q->num_bits, _q->zeta);
#if CVSD_ENABLE_SIGNAL_CONDITIONING
    printf(", alpha=%g", _q->alpha);
#endif
    printf(">\n");
    return LIQUID_OK;
}

// encode single sample
unsigned char cvsd_encode(cvsd _q,
                          float _audio_sample)
{
    // push audio sample through pre-filter
    float y;
#if CVSD_ENABLE_SIGNAL_CONDITIONING
    iirfilt_rrrf_execute(_q->prefilt, _audio_sample, &y);
#else
    y = _audio_sample;
#endif

    // determine output value
    unsigned char bit = (_q->ref > y) ? 0 : 1;

    // shift last value into buffer
    _q->bitref <<= 1;
    _q->bitref |= bit;
    _q->bitref &= _q->bitmask;

    // update delta
    if (_q->bitref == 0 || _q->bitref == _q->bitmask)
        _q->delta *= _q->zeta;  // increase delta
    else
        _q->delta /= _q->zeta;  // decrease delta

    // limit delta
    _q->delta = (_q->delta > _q->delta_max) ? _q->delta_max : _q->delta;
    _q->delta = (_q->delta < _q->delta_min) ? _q->delta_min : _q->delta;

    // update reference
    _q->ref += (bit) ? _q->delta : -_q->delta;

    // limit reference
    _q->ref = (_q->ref >  1.0f) ?  1.0f : _q->ref;
    _q->ref = (_q->ref < -1.0f) ? -1.0f : _q->ref;

    return bit;
}

// decode single sample
float cvsd_decode(cvsd _q,
                  unsigned char _bit)
{
    // append bit into register
    _q->bitref <<= 1;
    _q->bitref |= (_bit & 0x01);
    _q->bitref &= _q->bitmask;

    // update delta
    if (_q->bitref == 0 || _q->bitref == _q->bitmask)
        _q->delta *= _q->zeta;  // increase delta
    else
        _q->delta /= _q->zeta;  // decrease delta

    // limit delta
    _q->delta = (_q->delta > _q->delta_max) ? _q->delta_max : _q->delta;
    _q->delta = (_q->delta < _q->delta_min) ? _q->delta_min : _q->delta;

    // update reference
    _q->ref += (_bit&0x01) ? _q->delta : -_q->delta;

    // limit reference
    _q->ref = (_q->ref >  1.0f) ?  1.0f : _q->ref;
    _q->ref = (_q->ref < -1.0f) ? -1.0f : _q->ref;

    // push reference value through post-filter
    float y;
#if CVSD_ENABLE_SIGNAL_CONDITIONING
    iirfilt_rrrf_execute(_q->postfilt, _q->ref, &y);
#else
    y = _q->ref;
#endif

    return y;
}

// encode 8 samples
int cvsd_encode8(cvsd _q,
                 float * _audio,
                 unsigned char * _data)
{
    unsigned char data=0x00;
    unsigned int i;
    for (i=0; i<8; i++) {
        data <<= 1;
        data |= cvsd_encode(_q, _audio[i]);
    }

    // set return value
    *_data = data;
    return LIQUID_OK;
}

// decode 8 samples
int cvsd_decode8(cvsd _q,
                 unsigned char _data,
                 float * _audio)
{
    unsigned char bit;
    unsigned int i;
    for (i=0; i<8; i++) {
        bit = (_data >> (8-i-1)) & 0x01;
        _audio[i] = cvsd_decode(_q, bit);
    }
    return LIQUID_OK;
}

