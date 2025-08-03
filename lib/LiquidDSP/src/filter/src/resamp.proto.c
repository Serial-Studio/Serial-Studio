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

//
// Arbitrary resampler
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// defined:
//  TO          output data type
//  TC          coefficient data type
//  TI          input data type
//  RESAMP()    name-mangling macro
//  FIRPFB()    firpfb macro

// enable run-time debug printing of resampler
#define DEBUG_RESAMP_PRINT              0

// internal: update timing
int RESAMP(_update_timing_state)(RESAMP() _q);

struct RESAMP(_s) {
    // filter design parameters
    unsigned int m;     // filter semi-length, h_len = 2*m + 1
    float as;           // filter stop-band attenuation
    float fc;           // filter cutoff frequency

    // resampling properties/states
    float rate;         // resampling rate (output/input)
    float del;          // fractional delay step

    // floating-point phase
    float tau;          // accumulated timing phase, 0 <= tau < 1
    float bf;           // soft filterbank index, bf = tau*npfb = b + mu
    int b;              // base filterbank index, 0 <= b < npfb
    float mu;           // fractional filterbank interpolation value, 0 <= mu < 1
    TO y0;              // filterbank output at index b
    TO y1;              // filterbank output at index b+1

    // polyphase filterbank properties/object
    unsigned int npfb;  // number of filters in the bank
    FIRPFB() f;         // filterbank object (interpolator)

    enum {
        RESAMP_STATE_BOUNDARY, // boundary between input samples
        RESAMP_STATE_INTERP,   // regular interpolation
    } state;
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
    if (_npfb == 0)
        return liquid_error_config("resamp_%s_create(), number of filter banks must be greater than zero", EXTENSION_FULL);

    // allocate memory for resampler
    RESAMP() q = (RESAMP()) malloc(sizeof(struct RESAMP(_s)));

    // set rate using formal method (specifies output stride
    // value 'del')
    RESAMP(_set_rate)(q, _rate);

    // set properties
    q->m    = _m;       // prototype filter semi-length
    q->fc   = _fc;      // prototype filter cutoff frequency
    q->as   = _as;      // prototype filter stop-band attenuation
    q->npfb = _npfb;    // number of filters in bank

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
    q->f = FIRPFB(_create)(q->npfb,h,n-1);

    // reset object and return
    RESAMP(_reset)(q);
    return q;
}

// create arbitrary resampler object with a specified input
// resampling rate and default parameters
//  m (filter semi-length) = 7
//  fc (filter cutoff frequency) = 0.25
//  as (filter stop-band attenuation) = 60 dB
//  npfb (number of filters in the bank) = 64
RESAMP() RESAMP(_create_default)(float _rate)
{
    // validate input
    if (_rate <= 0)
        return liquid_error_config("resamp_%s_create_default(), resampling rate must be greater than zero", EXTENSION_FULL);

    // det default parameters
    unsigned int m    = 7;
    float        fc   = 0.5f*_rate > 0.49f ? 0.49f : 0.5f*_rate;
    float        as   = 60.0f;
    unsigned int npfb = 64;

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
    q_copy->f = FIRPFB(_copy)(q_orig->f);

    // return object
    return q_copy;
}

// free arbitrary resampler object
int RESAMP(_destroy)(RESAMP() _q)
{
    // free polyphase filterbank
    FIRPFB(_destroy)(_q->f);

    // free main object memory
    free(_q);

    return LIQUID_OK;
}

// print resampler object
int RESAMP(_print)(RESAMP() _q)
{
    printf("<liquid.resamp_%s, rate=%g, m=%u, as=%.3f, fc=%g, npfb=%u>\n",
        EXTENSION_FULL, _q->r, _q->m, _q->as, _q->fc, _q->npfb);
    return FIRPFB(_print)(_q->f);
}

// reset resampler object
int RESAMP(_reset)(RESAMP() _q)
{
    // clear filterbank
    FIRPFB(_reset)(_q->f);

    // reset states
    _q->state = RESAMP_STATE_INTERP;// input/output sample state
    _q->tau   = 0.0f;           // accumulated timing phase
    _q->bf    = 0.0f;           // soft-valued filterbank index
    _q->b     = 0;              // base filterbank index
    _q->mu    = 0.0f;           // fractional filterbank interpolation value
    _q->y0    = 0;              // filterbank output at index b
    _q->y1    = 0;              // filterbank output at index b+1
    return LIQUID_OK;
}

// get resampler filter delay (semi-length m)
unsigned int RESAMP(_get_delay)(RESAMP() _q)
{
    return _q->m;
}

// Set output scaling for resampler
int RESAMP(_set_scale)(RESAMP() _q,
                       TC       _scale)
{
    return FIRPFB(_set_scale)(_q->f, _scale);
}

// Get output scaling for resampler
int RESAMP(_get_scale)(RESAMP() _q,
                       TC *    _scale)
{
    return FIRPFB(_get_scale)(_q->f, _scale);
}

// set rate of arbitrary resampler
//  _q      : resampling object
//  _rate   : new sampling rate, _rate > 0
int RESAMP(_set_rate)(RESAMP() _q,
                      float    _rate)
{
    if (_rate <= 0)
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_set_rate(), resampling rate must be greater than zero", EXTENSION_FULL);

    // set internal rate
    _q->rate = _rate;

    // set output stride
    _q->del = 1.0f / _q->rate;
    return LIQUID_OK;
}

// get rate of arbitrary resampler
float RESAMP(_get_rate)(RESAMP() _q)
{
    return _q->rate;
}

// adjust resampling rate
int RESAMP(_adjust_rate)(RESAMP() _q,
                         float    _gamma)
{
    if (_gamma <= 0)
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_adjust_rate(), resampling adjustment (%12.4e) must be greater than zero", EXTENSION_FULL, _gamma);

    // adjust internal rate
    _q->rate *= _gamma;

    // set output stride
    _q->del = 1.0f / _q->rate;
    return LIQUID_OK;
}


// set resampling timing phase
//  _q      : resampling object
//  _tau    : sample timing
int RESAMP(_set_timing_phase)(RESAMP() _q,
                              float    _tau)
{
    if (_tau < -1.0f || _tau > 1.0f)
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_set_timing_phase(), timing phase must be in [-1,1], is %f.",EXTENSION_FULL,_tau);

    // set internal timing phase
    _q->tau = _tau;
    return LIQUID_OK;
}

// adjust resampling timing phase
//  _q      : resampling object
//  _delta  : sample timing adjustment
int RESAMP(_adjust_timing_phase)(RESAMP() _q,
                                 float    _delta)
{
    if (_delta < -1.0f || _delta > 1.0f)
        return liquid_error(LIQUID_EICONFIG,"resamp_%s_adjust_timing_phase(), timing phase adjustment must be in [-1,1], is %f.",EXTENSION_FULL,_delta);

    // adjust internal timing phase
    _q->tau += _delta;
    return LIQUID_OK;
}

// Get the number of output samples given current state and input buffer size.
unsigned int RESAMP(_get_num_output)(RESAMP()     _q,
                                     unsigned int _num_input)
{
    liquid_error(LIQUID_EINT,"resamp_%s_get_num_output(), function not implemented",EXTENSION_FULL);
    return 0;
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
    // push input sample into filterbank
    FIRPFB(_push)(_q->f, _x);
    unsigned int n=0;
    
    while (_q->b < _q->npfb) {

#if DEBUG_RESAMP_PRINT
        printf("  [%2u] : s=%1u, tau=%12.8f, b : %12.8f (%4d + %8.6f)\n", n+1, _q->state, _q->tau, _q->bf, _q->b, _q->mu);
#endif
        switch (_q->state) {
        case RESAMP_STATE_BOUNDARY:
            // compute filterbank output
            FIRPFB(_execute)(_q->f, 0, &_q->y1);

            // interpolate
            _y[n++] = (1.0f - _q->mu)*_q->y0 + _q->mu*_q->y1;
        
            // update timing state
            RESAMP(_update_timing_state)(_q);

            _q->state = RESAMP_STATE_INTERP;
            break;

        case RESAMP_STATE_INTERP:
            // compute output at base index
            FIRPFB(_execute)(_q->f, _q->b, &_q->y0);

            // check to see if base index is last filter in the bank, in
            // which case the resampler needs an additional input sample
            // to finish the linear interpolation process
            if (_q->b == _q->npfb-1) {
                // last filter: need additional input sample
                _q->state = RESAMP_STATE_BOUNDARY;
            
                // set index to indicate new sample is needed
                _q->b = _q->npfb;
            } else {
                // do not need additional input sample; compute
                // output at incremented base index
                FIRPFB(_execute)(_q->f, _q->b+1, &_q->y1);

                // perform linear interpolation between filterbank outputs
                _y[n++] = (1.0f - _q->mu)*_q->y0 + _q->mu*_q->y1;

                // update timing state
                RESAMP(_update_timing_state)(_q);
            }
            break;
        default:
            return liquid_error(LIQUID_EICONFIG,"resamp_%s_execute(), invalid/unknown state", EXTENSION_FULL);
        }
    }

    // decrement timing phase by one sample
    _q->tau -= 1.0f;
    _q->bf  -= (float)(_q->npfb);
    _q->b   -= _q->npfb;

    // specify number of samples written
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


//
// internal methods
// 

// update timing state; increment output timing stride and
// quantize filterbank indices
int RESAMP(_update_timing_state)(RESAMP() _q)
{
    // update high-resolution timing phase
    _q->tau += _q->del;

    // convert to high-resolution filterbank index 
    _q->bf  = _q->tau * (float)(_q->npfb);

    // split into integer filterbank index and fractional interpolation
    _q->b   = (int)floorf(_q->bf);      // base index
    _q->mu  = _q->bf - (float)(_q->b);  // fractional index
    return LIQUID_OK;
}

