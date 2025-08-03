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
// Rational-rate resampler
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct RRESAMP(_s) {
    // filter design parameters
    unsigned int    P;          // interpolation factor (primitive)
    unsigned int    Q;          // decimation factor (primitive)
    unsigned int    m;          // filter semi-length, h_len = 2*m + 1
    unsigned int    block_len;  // number of blocks to run in execute()
    FIRPFB()        pfb;        // filterbank object (interpolator), Q filters in bank
};

// internal: execute rational-rate resampler on a primitive-length block of
// input samples and store the resulting samples in the output array.
int RRESAMP(_execute_primitive)(RRESAMP() _q,
                                TI *      _x,
                                TO *      _y);

// Create rational-rate resampler object from external coefficients
//  _interp : interpolation factor
//  _decim  : decimation factor
//  _m      : filter semi-length (delay)
//  _h      : filter coefficients, [size: 2*_interp*_m x 1]
RRESAMP() RRESAMP(_create)(unsigned int _interp,
                           unsigned int _decim,
                           unsigned int _m,
                           TC *         _h)
{
    // validate input
    if (_interp == 0)
        return liquid_error_config("rresamp_%s_create(), interpolation rate must be greater than zero", EXTENSION_FULL);
    if (_decim == 0)
        return liquid_error_config("rresamp_%s_create(), decimation rate must be greater than zero", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("rresamp_%s_create(), filter semi-length must be greater than zero", EXTENSION_FULL);

    // allocate memory for resampler
    RRESAMP() q = (RRESAMP()) malloc(sizeof(struct RRESAMP(_s)));

    // set properties
    q->P         = _interp;
    q->Q         = _decim;
    q->m         = _m;
    q->block_len =  1;

    // create poly-phase filter bank
    q->pfb = FIRPFB(_create)(q->P, _h, 2*q->P*q->m);

    // reset object and return
    RRESAMP(_reset)(q);
    return q;
}

// Create rational-rate resampler object from filter prototype
//  _interp : interpolation factor
//  _decim  : decimation factor
//  _m      : filter semi-length (delay)
//  _bw     : filter bandwidth relative to sample rate
//  _as     : filter stop-band attenuation [dB]
RRESAMP() RRESAMP(_create_kaiser)(unsigned int _interp,
                                  unsigned int _decim,
                                  unsigned int _m,
                                  float        _bw,
                                  float        _as)
{
    // scale interpolation and decimation factors by their greatest common divisor
    unsigned int gcd = liquid_gcd(_interp, _decim);
    _interp /= gcd;
    _decim  /= gcd;

    // check for critical bandwidth
    if (_bw < 0)
        _bw = _interp > _decim ? 0.5f : 0.5f * (float)_interp / (float)_decim;
    else if (_bw > 0.5f)
        return liquid_error_config("rresamp_%s_create_kaiser(), invalid bandwidth (%g), must be less than 0.5", EXTENSION_FULL, _bw);

    // design filter
    unsigned int h_len = 2*_interp*_m + 1;
    float * hf = (float*) malloc(h_len*sizeof(float));
    TC    * h  = (TC*)    malloc(h_len*sizeof(TC)   );
    liquid_firdes_kaiser(h_len, _bw/(float)_interp, _as, 0.0f, hf);

    // convert to type-specific coefficients
    unsigned int i;
    for (i=0; i<h_len; i++)
        h[i] = (TC) hf[i];

    // create object and set parameters
    RRESAMP() q = RRESAMP(_create)(_interp, _decim, _m, h);
    RRESAMP(_set_scale)(q, 2.0f*_bw*sqrtf((float)(q->Q)/(float)(q->P)));
    q->block_len = gcd;

    // free allocated memory and return object
    free(hf);
    free(h);
    return q;
}

// create rational-rate resampler object from prototype
RRESAMP() RRESAMP(_create_prototype)(int          _type,
                                     unsigned int _interp,
                                     unsigned int _decim,
                                     unsigned int _m,
                                     float        _beta)
{
    // scale interpolation and decimation factors by their greatest common divisor
    unsigned int gcd = liquid_gcd(_interp, _decim);
    _interp /= gcd;
    _decim  /= gcd;

    // design filter
    int          decim = _interp < _decim;          // interpolator? or decimator
    unsigned int k     = decim ? _decim : _interp;  // prototype samples per symbol
    unsigned int h_len = 2*k*_m + 1;                // filter length
    float * hf = (float*) malloc(h_len*sizeof(float));
    TC    * h  = (TC*)    malloc(h_len*sizeof(TC)   );
    liquid_firdes_prototype(_type, k, _m, _beta, 0, hf);

    // convert to type-specific coefficients
    unsigned int i;
    for (i=0; i<h_len; i++)
        h[i] = (TC) hf[i];

    // create object
    RRESAMP() q = RRESAMP(_create)(_interp, _decim, _m, h);
    q->block_len = gcd;

    // adjust gain according to resampling rate
    float rate = RRESAMP(_get_rate)(q);
    RRESAMP(_set_scale)(q, decim ? sqrtf(rate) : 1.0f / sqrtf(rate));

    // free allocated memory and return object
    free(hf);
    free(h);
    return q;
}

// create rational-rate resampler object with a specified input
// resampling rate and default parameters
//  m (filter semi-length) = 12
//  as (filter stop-band attenuation) = 60 dB
RRESAMP() RRESAMP(_create_default)(unsigned int _interp,
                                   unsigned int _decim)
{
    // det default parameters
    unsigned int m  = 12;
    float        bw = 0.5f;
    float        as = 60.0f;

    // create and return resamp object
    return RRESAMP(_create_kaiser)(_interp, _decim, m, bw, as);
}

// copy object
RRESAMP() RRESAMP(_copy)(RRESAMP() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("rresamp_%s_create(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy internal memory
    RRESAMP() q_copy = (RRESAMP()) malloc(sizeof(struct RRESAMP(_s)));
    memmove(q_copy, q_orig, sizeof(struct RRESAMP(_s)));

    // copy internal object and return
    q_copy->pfb = FIRPFB(_copy)(q_orig->pfb);
    return q_copy;
}

// free resampler object
int RRESAMP(_destroy)(RRESAMP() _q)
{
    // free polyphase filterbank
    FIRPFB(_destroy)(_q->pfb);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print resampler object
int RRESAMP(_print)(RRESAMP() _q)
{
    printf("<liquid.rresamp_%s, rate=%u/%u=%.6f, block=%u, u=%u>\n", EXTENSION_FULL,
            _q->P, _q->Q, (float)(_q->P) / (float)(_q->Q), _q->block_len, _q->m);
    return LIQUID_OK;
}

// reset resampler object
int RRESAMP(_reset)(RRESAMP() _q)
{
    // clear filterbank
    return FIRPFB(_reset)(_q->pfb);
}

// Set output scaling for filter, default: \( 2 w \sqrt{P/Q} \)
//  _q      : resampler object
//  _scale  : scaling factor to apply to each output sample
int RRESAMP(_set_scale)(RRESAMP() _q,
                        TC        _scale)
{
    return FIRPFB(_set_scale)(_q->pfb, _scale);
}

// Get output scaling for filter
//  _q      : resampler object
//  _scale  : scaling factor to apply to each output sample
int RRESAMP(_get_scale)(RRESAMP() _q,
                        TC *      _scale)
{
    return FIRPFB(_get_scale)(_q->pfb, _scale);
}

// get resampler filter delay (semi-length m)
unsigned int RRESAMP(_get_delay)(RRESAMP() _q)
{
    return _q->m;
}

// get block length
unsigned int RRESAMP(_get_block_len)(RRESAMP() _q)
{
    return _q->block_len;
}

// Get rate of resampler, r = P/Q
float RRESAMP(_get_rate)(RRESAMP() _q)
{
    return (float)(_q->P) / (float)(_q->Q);
}

// Get original interpolation factor of resampler, P
unsigned int RRESAMP(_get_P)(RRESAMP() _q)
{
    return _q->P * _q->block_len;
}

// Get internal interpolation factor of resampler, \(P\), after removing gcd
unsigned int RRESAMP(_get_interp)(RRESAMP() _q)
{
    return _q->P;
}

// Get original decimation factor of resampler, Q
unsigned int RRESAMP(_get_Q)(RRESAMP() _q)
{
    return _q->Q * _q->block_len;
}

// Get internal decimator factor of resampler, \(Q\), after removing gcd
unsigned int RRESAMP(_get_decim)(RRESAMP() _q)
{
    return _q->Q;
}

// Write \(Q\) input samples (after removing greatest common divisor)
// into buffer, but do not compute output. This effectively updates the
// internal state of the resampler.
//  _q      : resamp object
//  _buf    : input sample array, [size: Q x 1]
int RRESAMP(_write)(RRESAMP() _q,
                    TI *      _buf)
{
    return FIRPFB(_write)(_q->pfb, _buf, _q->Q);
}

// Execute rational-rate resampler on a block of input samples and
// store the resulting samples in the output array.
//  _q  : resamp object
//  _x  : input sample array, [size: Q x 1]
//  _y  : output sample array [size: P x 1]
int RRESAMP(_execute)(RRESAMP() _q,
                      TI *      _x,
                      TO *      _y)
{
    // run in blocks
    unsigned int i;
    for (i=0; i<_q->block_len; i++) {
        // compute P outputs for Q inputs
        RRESAMP(_execute_primitive)(_q, _x, _y);

        // update input pointers accordingly
        _x += _q->Q;
        _y += _q->P;
    }
    return LIQUID_OK;
}

// Execute on a block of samples
//  _q  : resamp object
//  _x  : input sample array, [size: Q*n x 1]
//  _n  : block size
//  _y  : output sample array [size: P*n x 1]
int RRESAMP(_execute_block)(RRESAMP()      _q,
                            TI *           _x,
                            unsigned int   _n,
                            TO *           _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        RRESAMP(_execute)(_q, _x, _y);
        _x += _q->Q;
        _y += _q->P;
    }
    return LIQUID_OK;
}

// internal
int RRESAMP(_execute_primitive)(RRESAMP() _q,
                                TI *      _x,
                                TO *      _y)
{
    unsigned int index = 0; // filterbank index
    unsigned int i, n=0;
    for (i=0; i<_q->Q; i++) {
        // push input
        FIRPFB(_push)(_q->pfb, _x[i]);

        // continue to produce output
        while (index < _q->P) {
            FIRPFB(_execute)(_q->pfb, index, &_y[n++]);
            index += _q->Q;
        }

        // decrement filter-bank index by output rate
        index -= _q->P;
    }

    if (index != 0) {
        return liquid_error(LIQUID_EINT,"rresamp_%s_execute_primitive(), index=%u (expected 0)",
                EXTENSION_FULL, index);
    } else if (n != _q->P) {
        return liquid_error(LIQUID_EINT,"rresamp_%s_execute_primitive(), n=%u (expected P=%u)",
                EXTENSION_FULL, n, _q->P);
    }
    return LIQUID_OK;
}

