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

// finite impulse response polyphase filterbank channelizer with output
// rate Fs / P

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// firpfbchr object structure definition
struct FIRPFBCHR(_s) {
    unsigned int M;     // number of channels
    unsigned int P;     // decimation factor
    unsigned int m;     // filter semi-length

    // filter
    unsigned int h_len; // prototype filter length: 2*M*m
    
    // create separate bank of dotprod and window objects
    DOTPROD() * dp;     // dot product object array

    // inverse FFT plan
    FFT_PLAN ifft;      // inverse FFT object
    TO * X;             // IFFT input array  [size: M x 1]
    TO * x;             // IFFT output array [size: M x 1]

    // common data structures shared between analysis and
    // synthesis algorithms
    WINDOW() * w;       // window buffer object array
    unsigned int base_index;
};

// create rational rate resampling channelizer (firpfbchr) object by
// specifying filter coefficients directly
//  _chans  : number of output channels in chanelizer
//  _decim  : output decimation factor (output rate is 1/P the input)
//  _m      : prototype filter semi-length, length=2*M*m
//  _h      : prototype filter coefficient array, [size: 2*M*m x 1]
FIRPFBCHR() FIRPFBCHR(_create)(unsigned int _chans,
                               unsigned int _decim,
                               unsigned int _m,
                               TC *         _h)
{
    // validate input
    if (_chans < 2)
        return liquid_error_config("firpfbchr_%s_create(), number of channels must be at least 2", EXTENSION_FULL);
    if (_decim < 1)
        return liquid_error_config("firpfbchr_%s_create(), decimation rate must be at least 1", EXTENSION_FULL);
    if (_m < 1)
        return liquid_error_config("firpfbchr_%s_create(), filter semi-length must be at least 1", EXTENSION_FULL);
    if (_h == NULL)
        return liquid_error_config("firpfbchr_%s_create(), filter coefficients cannot be null", EXTENSION_FULL);

    // create object
    FIRPFBCHR() q = (FIRPFBCHR()) malloc(sizeof(struct FIRPFBCHR(_s)));

    // set input parameters
    q->M  = _chans; // number of channels
    q->P  = _decim; //
    q->m  = _m;     // prototype filter semi-length

    // compute derived values
    q->h_len    = 2*q->M*q->m;  // prototype filter length

    // generate bank of sub-samped filters
    q->dp = (DOTPROD()*) malloc((q->M)*sizeof(DOTPROD()));
    unsigned int i;
    unsigned int n;
    unsigned int h_sub_len = 2 * q->m;
    TC h_sub[h_sub_len];
    for (i=0; i<q->M; i++) {
        // sub-sample prototype filter, loading coefficients
        // in reverse order
        for (n=0; n<h_sub_len; n++)
            h_sub[h_sub_len-n-1] = _h[i + n*(q->M)];

        // create dotprod object
        q->dp[i] = DOTPROD(_create)(h_sub,h_sub_len);
    }

    // create FFT plan (inverse transform)
    q->X = (T*) FFT_MALLOC((q->M)*sizeof(T));   // IFFT input
    q->x = (T*) FFT_MALLOC((q->M)*sizeof(T));   // IFFT output
    q->ifft = FFT_CREATE_PLAN(q->M, q->X, q->x, FFT_DIR_BACKWARD, FFT_METHOD);

    // create buffer objects
    q->w = (WINDOW()*) malloc((q->M)*sizeof(WINDOW()));
    for (i=0; i<q->M; i++)
        q->w[i] = WINDOW(_create)(h_sub_len);

    // reset filterbank object and return
    FIRPFBCHR(_reset)(q);
    return q;
}

// create rational rate resampling channelizer (firpfbchr) object by
// specifying filter design parameters for Kaiser prototype
//  _chans  : number of output channels in chanelizer
//  _decim  : output decimation factor (output rate is 1/P the input)
//  _m      : prototype filter semi-length, length=2*M*m
//  _as     : filter stop-band attenuation [dB]
FIRPFBCHR() FIRPFBCHR(_create_kaiser)(unsigned int _chans,
                                      unsigned int _decim,
                                      unsigned int _m,
                                      float        _as)
{
    // validate input
    if (_chans < 2)
        return liquid_error_config("firpfbchr_%s_create_kaiser(), number of channels must be at least 2", EXTENSION_FULL);
    if (_decim < 1)
        return liquid_error_config("firpfbchr_%s_create_kaiser(), decimation rate must be at least 1", EXTENSION_FULL);
    if (_m < 1)
        return liquid_error_config("firpfbchr_%s_create_kaiser(), filter semi-length must be at least 1", EXTENSION_FULL);
    if (_as <= 0.0f)
        return liquid_error_config("firpfbchr_%s_create_kaiser(), stop-band suppression out of range", EXTENSION_FULL);

    // design prototype filter
    unsigned int h_len = 2*_chans*_m+1;
    float * hf = (float*)malloc(h_len*sizeof(float));

    // filter cut-off frequency
    float fc = 0.5f/(float)_decim;

    // compute filter coefficients (floating point precision)
    liquid_firdes_kaiser(h_len, fc, _as, 0.0f, hf);

    // normalize to unit average and scale by number of channels
    float hf_sum = 0.0f;
    unsigned int i;
    for (i=0; i<h_len; i++) hf_sum += hf[i];
    for (i=0; i<h_len; i++) hf[i] = hf[i] * sqrtf(_decim) * (float)_chans / hf_sum;

    // convert to type-specific array
    TC * h = (TC*) malloc(h_len * sizeof(TC));
    for (i=0; i<h_len; i++)
        h[i] = (TC) hf[i];

    // create filterbank channelizer object
    FIRPFBCHR() q = FIRPFBCHR(_create)(_chans, _decim, _m, h);

    // free prototype filter coefficients
    free(hf);
    free(h);

    // return object
    return q;
}

// destroy firpfbchr object, freeing internal memory
int FIRPFBCHR(_destroy)(FIRPFBCHR() _q)
{
    unsigned int i;

    // free dotprod objects
    for (i=0; i<_q->M; i++)
        DOTPROD(_destroy)(_q->dp[i]);
    free(_q->dp);

    // free transform object and arrays
    FFT_DESTROY_PLAN(_q->ifft);
    FFT_FREE(_q->X);
    FFT_FREE(_q->x);
    
    // free window objects (buffers)
    for (i=0; i<_q->M; i++)
        WINDOW(_destroy)(_q->w[i]);
    free(_q->w);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// reset firpfbchr object internals
int FIRPFBCHR(_reset)(FIRPFBCHR() _q)
{
    unsigned int i;

    // clear window buffers
    for (i=0; i<_q->M; i++)
        WINDOW(_reset)(_q->w[i]);

    // reset filter/buffer alignment flag
    _q->base_index = _q->M - 1;
    return LIQUID_OK;
}

// print firpfbchr object internals
int FIRPFBCHR(_print)(FIRPFBCHR() _q)
{
    printf("<liquid.firpfbchr, channels=%u, decim=%u, semilen=%u>\n",
        _q->M, _q->P, _q->m);
    return LIQUID_OK;
}

// DEPRECATED: get number of output channels to channelizer
unsigned int FIRPFBCHR(_get_M)(FIRPFBCHR() _q)
{
    return FIRPFBCHR(_get_num_channels)(_q);
}

// get number of output channels to channelizer
unsigned int FIRPFBCHR(_get_num_channels)(FIRPFBCHR() _q)
{
    return _q->M;
}

// DEPRECATED: get decimation rate
unsigned int FIRPFBCHR(_get_P)(FIRPFBCHR() _q)
{
    return FIRPFBCHR(_get_decim_rate)(_q);
}

// get decimation rate
unsigned int FIRPFBCHR(_get_decim_rate)(FIRPFBCHR() _q)
{
    return _q->P;
}

// get semi-length to channelizer filter prototype
unsigned int FIRPFBCHR(_get_m)(FIRPFBCHR() _q)
{
    return _q->m;
}

// push samples into filter bank
//  _q      : channelizer object
//  _x      : channelizer input, [size: decim x 1]
int FIRPFBCHR(_push)(FIRPFBCHR() _q,
                     TI *        _x)
{
    // load buffers in blocks of P in the reverse direction
    unsigned int i;
    for (i=0; i<_q->P; i++) {
        // push sample into buffer at filter index
        WINDOW(_push)(_q->w[_q->base_index], _x[i]);

        // decrement base index, wrapping around
        _q->base_index = _q->base_index == 0 ? _q->M-1 : _q->base_index-1;
    }
    return LIQUID_OK;
}

// execute filterbank channelizer (synthesizer)
//  _q      : channelizer object
//  _y      : channelizer output, [size: M x 1]
int FIRPFBCHR(_execute)(FIRPFBCHR() _q,
                        TO *        _y)
{
    unsigned int i;

    // execute filter outputs
    TO * r;  // buffer read pointer
    for (i=0; i<_q->M; i++) {
        // buffer index
        unsigned int buffer_index = (_q->base_index+i+1) % _q->M;

        // read buffer at index
        WINDOW(_read)(_q->w[buffer_index], &r);

        // run dot products
        DOTPROD(_execute)(_q->dp[i], r, &_q->X[buffer_index]);
    }

    // execute IFFT, store result in buffer 'x'
    FFT_EXECUTE(_q->ifft);

    // copy result to output, scale result by 1/num_channels (C transform)
    float g = 1.0f / (float)(_q->M);
    for (i=0; i<_q->M; i++)
        _y[i] = _q->x[i] * g;
    return LIQUID_OK;
}

