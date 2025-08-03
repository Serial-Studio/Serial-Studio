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

// Arbitrary resampler (fixed-point phase versions)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_RESAMP_PRINT  0

// main object
struct RESAMP(_s) {
    // filter design parameters
    unsigned int    m;      // filter semi-length, h_len = 2*m + 1
    float           as;     // filter stop-band attenuation
    float           fc;     // filter cutoff frequency

    // internal state variables
    float           r;      // resampling rate
    uint32_t        step;   // step size (quantized resampling rate)
    uint32_t        phase;  // sampling phase
    unsigned int    bits_index;
    unsigned int    npfb;   // 256
    FIRPFB()        pfb;    // filter bank
};

// create arbitrary resampler
//  _rate   :   resampling rate
//  _m      :   prototype filter semi-length
//  _fc     :   prototype filter cutoff frequency, fc in (0, 0.5)
//  _as     :   prototype filter stop-band attenuation [dB] (e.g. 60)
//  _npfb   :   number of filters in polyphase filterbank
RESAMP() RESAMP(_create)(float        _rate,
                         unsigned int _m,
                         float        _fc,
                         float        _as,
                         unsigned int _npfb)
{
    // validate input
    if (_rate <= 0)
        return liquid_error_config("resamp_%s_create(), resampling rate must be greater than zero", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("resamp_%s_create(), filter semi-length must be greater than zero", EXTENSION_FULL);
    if (_fc <= 0.0f || _fc >= 0.5f)
        return liquid_error_config("resamp_%s_create(), filter cutoff must be in (0,0.5)", EXTENSION_FULL);
    if (_as <= 0.0f)
        return liquid_error_config("resamp_%s_create(), filter stop-band suppression must be greater than zero", EXTENSION_FULL);

    // check number of bits representing filter bank resolution
    unsigned int bits = liquid_nextpow2(_npfb);
    if (bits < 1 || bits > 16)
        return liquid_error_config("resamp_%s_create(), number of filter banks must be in (2^0,2^16)", EXTENSION_FULL);

    // allocate memory for resampler
    RESAMP() q = (RESAMP()) malloc(sizeof(struct RESAMP(_s)));

    // set rate using formal method (specifies output stride
    // value 'del')
    RESAMP(_set_rate)(q, _rate);

    // set properties
    q->m    = _m;               // prototype filter semi-length
    q->fc   = _fc;              // prototype filter cutoff frequency
    q->as   = _as;              // prototype filter stop-band attenuation
    q->bits_index = bits;       // number of bits representing filters in bank
    q->npfb = 1<<q->bits_index; // number of filters in bank

    // design filter
    unsigned int n = 2*q->m*q->npfb+1;
    float hf[n];
    TC h[n];
    liquid_firdes_kaiser(n,q->fc/((float)(q->npfb)),q->as,0.0f,hf);

    // normalize filter coefficients by DC gain
    unsigned int i;
    float gain=0.0f;
    for (i=0; i<n; i++)
        gain += hf[i];
    gain = (q->npfb)/(gain);

    // copy to type-specific array, applying gain
    for (i=0; i<n; i++)
        h[i] = hf[i]*gain;
    q->pfb = FIRPFB(_create)(q->npfb,h,n-1);

    // reset object and return
    RESAMP(_reset)(q);
    return q;
}

// create arbitrary resampler object with a specified input
// resampling rate and default parameters
//  m (filter semi-length) = 7
//  fc (filter cutoff frequency) = 0.25
//  as (filter stop-band attenuation) = 60 dB
//  npfb (number of filters in the bank) = 256
RESAMP() RESAMP(_create_default)(float _rate)
{
    // validate input
    if (_rate <= 0)
        return liquid_error_config("resamp_%s_create_default(), resampling rate must be greater than zero", EXTENSION_FULL);

    // det default parameters
    unsigned int m    = 7;
    float        fc   = 0.25f;
    float        as   = 60.0f;
    unsigned int npfb = 256;

    // create and return resamp object
    return RESAMP(_create)(_rate, m, fc, as, npfb);
}

// copy object
RESAMP() RESAMP(_copy)(RESAMP() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("resamp_%s_copy(), object cannot be NULL", EXTENSION_FULL);

    // create object, copy internal memory, overwrite with specific values
    RESAMP() q_copy = (RESAMP()) malloc(sizeof(struct RESAMP(_s)));
    memmove(q_copy, q_orig, sizeof(struct RESAMP(_s)));

    // copy filter bank
    q_copy->pfb = FIRPFB(_copy)(q_orig->pfb);

    // return object
    return q_copy;
}

// free arbitrary resampler object
int RESAMP(_destroy)(RESAMP() _q)
{
    // free polyphase filterbank
    FIRPFB(_destroy)(_q->pfb);

    // free main object memory
    free(_q);

    return LIQUID_OK;
}

// print resampler object
int RESAMP(_print)(RESAMP() _q)
{
    printf("<liquid.resamp_%s, rate=%g, m=%u, as=%.3f, fc=%g, npfb=%u>\n",
        EXTENSION_FULL, _q->r, _q->m, _q->as, _q->fc, _q->npfb);
    return LIQUID_OK;
}

// reset resampler object
int RESAMP(_reset)(RESAMP() _q)
{
    // reset state
    _q->phase = 0;

    // clear filterbank
    return FIRPFB(_reset)(_q->pfb);
}

// get resampler filter delay (semi-length m)
unsigned int RESAMP(_get_delay)(RESAMP() _q)
{
    return _q->m;
}

// set rate of arbitrary resampler
//  _q      : resampling object
//  _rate   : new sampling rate, _rate > 0
int RESAMP(_set_rate)(RESAMP() _q,
                      float    _rate)
{
    if (_rate <= 0) {
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_set_rate(), resampling rate must be greater than zero", EXTENSION_FULL);
    } else if (_rate < 0.004f || _rate > 250.0f) {
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_set_rate(), resampling rate must be in [0.004,250]", EXTENSION_FULL);
    }

    // set internal rate
    _q->r = _rate;

    // set output stride
    // TODO: ensure step is reasonable size
    _q->step = (uint32_t)round((1<<24)/_q->r);

    return LIQUID_OK;
}

// get rate of arbitrary resampler
float RESAMP(_get_rate)(RESAMP() _q)
{
    return _q->r;
}

// adjust resampling rate
int RESAMP(_adjust_rate)(RESAMP() _q,
                         float    _gamma)
{
    if (_gamma <= 0)
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_adjust_rate(), resampling adjustment (%12.4e) must be greater than zero", EXTENSION_FULL, _gamma);

    // adjust internal rate
    return RESAMP(_set_rate)(_q, _q->r * _gamma);
}


// set resampling timing phase
//  _q      : resampling object
//  _tau    : sample timing
int RESAMP(_set_timing_phase)(RESAMP() _q,
                              float    _tau)
{
    return liquid_error(LIQUID_EICONFIG,"resamp_%s_set_timing_phase(), method not implemented",EXTENSION_FULL);
    if (_tau < -1.0f || _tau > 1.0f) {
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_set_timing_phase(), timing phase must be in [-1,1], is %f.",EXTENSION_FULL,_tau);
    }

    // TODO: set internal timing phase (quantized)
    //_q->tau = _tau;

    return LIQUID_OK;
}

// adjust resampling timing phase
//  _q      : resampling object
//  _delta  : sample timing adjustment
int RESAMP(_adjust_timing_phase)(RESAMP() _q,
                                 float    _delta)
{
    return liquid_error(LIQUID_EICONFIG,"resamp_%s_adjust_timing_phase(), method not implemented",EXTENSION_FULL);
    if (_delta < -1.0f || _delta > 1.0f) {
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_adjust_timing_phase(), timing phase adjustment must be in [-1,1], is %f.",EXTENSION_FULL,_delta);
    }

    // TODO: adjust internal timing phase (quantized)
    //_q->tau += _delta;
    return LIQUID_OK;
}

// Get the number of output samples given current state and input buffer size.
unsigned int RESAMP(_get_num_output)(RESAMP()     _q,
                                     unsigned int _num_input)
{
    // count the number of rollovers
    uint32_t phase = _q->phase;
    unsigned int i, num_output=0;
    for (i=0; i<_num_input; i++) {
        while (phase <= 0x00ffffff) {
            num_output++;
            phase += _q->step;
        }
        phase -= (1<<24);
    }
    return num_output;
}

// run arbitrary resampler
//  _q          :   resampling object
//  _x          :   single input sample
//  _y          :   output array
//  _num_written:   number of samples written to output
int RESAMP(_execute)(RESAMP()       _q,
                     TI             _x,
                     TO *           _y,
                     unsigned int * _num_written)
{
    // push input
    FIRPFB(_push)(_q->pfb, _x);

    // continue to produce output
    unsigned int n=0;
    while (_q->phase <= 0x00ffffff) {
        //unsigned int index = (_q->phase + 0x00008000) >> 16;
        unsigned int index = _q->phase >> (24 - _q->bits_index); // round down
        FIRPFB(_execute)(_q->pfb, index, &_y[n++]);
        _q->phase += _q->step;
    }

    // decrement filter-bank index by output rate
    _q->phase -= (1<<24);

    // error checking for now
    *_num_written = n;
    return LIQUID_OK;
}

// execute arbitrary resampler on a block of samples
//  _q              :   resamp object
//  _x              :   input buffer [size: _nx x 1]
//  _nx             :   input buffer
//  _y              :   output sample array (pointer)
//  _ny             :   number of samples written to _y
int RESAMP(_execute_block)(RESAMP()       _q,
                           TI *           _x,
                           unsigned int   _nx,
                           TO *           _y,
                           unsigned int * _ny)
{
    // initialize number of output samples to zero
    unsigned int ny = 0;

    // number of samples written for each individual iteration
    unsigned int num_written;

    // iterate over each input sample
    unsigned int i;
    for (i=0; i<_nx; i++) {
        // run resampler on single input
        RESAMP(_execute)(_q, _x[i], &_y[ny], &num_written);

        // update output counter
        ny += num_written;
    }

    // set return value for number of output samples written
    *_ny = ny;
    return LIQUID_OK;
}

