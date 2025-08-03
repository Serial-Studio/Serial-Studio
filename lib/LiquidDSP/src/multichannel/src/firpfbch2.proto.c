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
// firpfbch2.c
//
// finite impulse response polyphase filterbank channelizer with output
// rate 2 Fs / M
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// firpfbch2 object structure definition
struct FIRPFBCH2(_s) {
    int          type;  // synthesis/analysis
    unsigned int M;     // number of channels
    unsigned int M2;    // number of channels/2
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
    WINDOW() * w0;      // window buffer object array
    WINDOW() * w1;      // window buffer object array (synthesizer only)
    int flag;           // flag indicating filter/buffer alignment
};

// create firpfbch2 object
//  _type   :   channelizer type (e.g. LIQUID_ANALYZER)
//  _M      :   number of channels (must be even)
//  _m      :   prototype filter semi-lenth, length=2*M*m
//  _h      :   prototype filter coefficient array
//  _h_len  :   number of coefficients
FIRPFBCH2() FIRPFBCH2(_create)(int          _type,
                               unsigned int _M,
                               unsigned int _m,
                               TC *         _h)
{
    // validate input
    if (_type != LIQUID_ANALYZER && _type != LIQUID_SYNTHESIZER)
        return liquid_error_config("firpfbch2_%s_create(), invalid type %d", EXTENSION_FULL, _type);
    if (_M < 2 || _M % 2)
        return liquid_error_config("firpfbch2_%s_create(), number of channels must be greater than 2 and even", EXTENSION_FULL);
    if (_m < 1)
        return liquid_error_config("firpfbch2_%s_create(), filter semi-length must be at least 1", EXTENSION_FULL);

    // create object
    FIRPFBCH2() q = (FIRPFBCH2()) malloc(sizeof(struct FIRPFBCH2(_s)));

    // set input parameters
    q->type     = _type;        // channelizer type (e.g. LIQUID_ANALYZER)
    q->M        = _M;           // number of channels
    q->m        = _m;           // prototype filter semi-length

    // compute derived values
    q->h_len    = 2*q->M*q->m;  // prototype filter length
    q->M2       = q->M / 2;     // number of channels / 2

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
    q->w0 = (WINDOW()*) malloc((q->M)*sizeof(WINDOW()));
    q->w1 = (WINDOW()*) malloc((q->M)*sizeof(WINDOW()));
    for (i=0; i<q->M; i++) {
        q->w0[i] = WINDOW(_create)(h_sub_len);
        q->w1[i] = WINDOW(_create)(h_sub_len);
    }

    // reset filterbank object and return
    FIRPFBCH2(_reset)(q);
    return q;
}

// create firpfbch2 object using Kaiser window prototype
//  _type   :   channelizer type (e.g. LIQUID_ANALYZER)
//  _M      :   number of channels (must be even)
//  _m      :   prototype filter semi-lenth, length=2*M*m+1
//  _as     :   filter stop-band attenuation [dB]
FIRPFBCH2() FIRPFBCH2(_create_kaiser)(int          _type,
                                      unsigned int _M,
                                      unsigned int _m,
                                      float        _as)
{
    // validate input
    if (_type != LIQUID_ANALYZER && _type != LIQUID_SYNTHESIZER)
        return liquid_error_config("firpfbch2_%s_create_kaiser(), invalid type %d", EXTENSION_FULL, _type);
    if (_M < 2 || _M % 2)
        return liquid_error_config("firpfbch2_%s_create_kaiser(), number of channels must be greater than 2 and even", EXTENSION_FULL);
    if (_m < 1)
        return liquid_error_config("firpfbch2_%s_create_kaiser(), filter semi-length must be at least 1", EXTENSION_FULL);

    // design prototype filter
    unsigned int h_len = 2*_M*_m+1;
    float * hf = (float*)malloc(h_len*sizeof(float));

    // filter cut-off frequency (analyzer has twice the
    // bandwidth of the synthesizer)
    float fc = (_type == LIQUID_ANALYZER) ? 1.0f/(float)_M : 0.5f/(float)_M;

    // compute filter coefficients (floating point precision)
    liquid_firdes_kaiser(h_len, fc, _as, 0.0f, hf);

    // normalize to unit average and scale by number of channels
    float hf_sum = 0.0f;
    unsigned int i;
    for (i=0; i<h_len; i++) hf_sum += hf[i];
    for (i=0; i<h_len; i++) hf[i] = hf[i] * (float)_M / hf_sum;

    // convert to type-specific array
    TC * h = (TC*) malloc(h_len * sizeof(TC));
    for (i=0; i<h_len; i++)
        h[i] = (TC) hf[i];

    // create filterbank channelizer object
    FIRPFBCH2() q = FIRPFBCH2(_create)(_type, _M, _m, h);

    // free prototype filter coefficients
    free(hf);
    free(h);

    // return object
    return q;
}

// copy object
FIRPFBCH2() FIRPFBCH2(_copy)(FIRPFBCH2() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firfilt_%s_copy(), object cannot be NULL", EXTENSION_FULL);

    // create object and copy base parameters
    FIRPFBCH2() q_copy = (FIRPFBCH2()) malloc(sizeof(struct FIRPFBCH2(_s)));
    memmove(q_copy, q_orig, sizeof(struct FIRPFBCH2(_s)));

    // generate and copy bank of sub-samped filters
    unsigned int i;
    q_copy->dp = (DOTPROD()*) malloc((q_copy->M)*sizeof(DOTPROD()));
    for (i=0; i<q_copy->M; i++)
        q_copy->dp[i] = DOTPROD(_copy)(q_orig->dp[i]);

    // create FFT plan (inverse transform)
    q_copy->X = (T*) FFT_MALLOC((q_copy->M)*sizeof(T));   // IFFT input
    q_copy->x = (T*) FFT_MALLOC((q_copy->M)*sizeof(T));   // IFFT output
    q_copy->ifft = FFT_CREATE_PLAN(q_copy->M, q_copy->X, q_copy->x, FFT_DIR_BACKWARD, FFT_METHOD);

    // create and copy buffer objects
    q_copy->w0 = (WINDOW()*) malloc((q_copy->M)*sizeof(WINDOW()));
    q_copy->w1 = (WINDOW()*) malloc((q_copy->M)*sizeof(WINDOW()));
    for (i=0; i<q_copy->M; i++) {
        q_copy->w0[i] = WINDOW(_copy)(q_orig->w0[i]);
        q_copy->w1[i] = WINDOW(_copy)(q_orig->w1[i]);
    }

    return q_copy;
}

// destroy firpfbch2 object, freeing internal memory
int FIRPFBCH2(_destroy)(FIRPFBCH2() _q)
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
    for (i=0; i<_q->M; i++) {
        WINDOW(_destroy)(_q->w0[i]);
        WINDOW(_destroy)(_q->w1[i]);
    }
    free(_q->w0);
    free(_q->w1);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// reset firpfbch2 object internals
int FIRPFBCH2(_reset)(FIRPFBCH2() _q)
{
    unsigned int i;

    // clear window buffers
    for (i=0; i<_q->M; i++) {
        WINDOW(_reset)(_q->w0[i]);
        WINDOW(_reset)(_q->w1[i]);
    }

    // reset filter/buffer alignment flag
    _q->flag = 0;
    return LIQUID_OK;
}

// print firpfbch2 object internals
int FIRPFBCH2(_print)(FIRPFBCH2() _q)
{
    printf("<liquid.firpfbch2, type=\"%s\", channels=%u, semilen=%u>\n",
        _q->type == LIQUID_ANALYZER ? "analyzer" : "synthesizer",
        _q->M, _q->m);
    return LIQUID_OK;
}

// get type, either LIQUID_ANALYZER or LIQUID_SYNTHESIZER
int FIRPFBCH2(_get_type)(FIRPFBCH2() _q)
{
    return _q->type;
}

// get number of channels, M
unsigned int FIRPFBCH2(_get_M)(FIRPFBCH2() _q)
{
    return _q->M;
}

// get prototype filter sem-length, m
unsigned int FIRPFBCH2(_get_m)(FIRPFBCH2() _q)
{
    return _q->m;
}

// execute filterbank channelizer (analyzer)
//  _x      :   channelizer input,  [size: M/2 x 1]
//  _y      :   channelizer output, [size: M   x 1]
int FIRPFBCH2(_execute_analyzer)(FIRPFBCH2() _q,
                                 TI *        _x,
                                 TO *        _y)
{
    unsigned int i;

    // load buffers in blocks of num_channels/2 starting
    // in the middle of the filter bank and moving in the
    // negative direction
    unsigned int base_index = _q->flag ? _q->M : _q->M2;
    for (i=0; i<_q->M2; i++) {
        // push sample into buffer at filter index
        WINDOW(_push)(_q->w0[base_index-i-1], _x[i]);
    }

    // execute filter outputs
    unsigned int offset = _q->flag ? _q->M2 : 0;
    TI * r;      // buffer read pointer
    for (i=0; i<_q->M; i++) {
        // read buffer at index
        WINDOW(_read)(_q->w0[i], &r);

        // run dot product storing result in IFFT input buffer
        DOTPROD(_execute)(_q->dp[(offset+i)%_q->M], r, &_q->X[i]);
    }

    // execute IFFT, store result in buffer 'x'
    FFT_EXECUTE(_q->ifft);

    // scale result by 1/num_channels (C transform)
    for (i=0; i<_q->M; i++)
        _y[i] = _q->x[i] / (float)(_q->M);

    // update flag
    _q->flag = 1 - _q->flag;
    return LIQUID_OK;
}

// execute filterbank channelizer (synthesizer)
//  _x      :   channelizer input,  [size: M   x 1]
//  _y      :   channelizer output, [size: M/2 x 1]
int FIRPFBCH2(_execute_synthesizer)(FIRPFBCH2() _q,
                                    TI *        _x,
                                    TO *        _y)
{
    unsigned int i;

    // copy input array to internal IFFT input buffer
    memmove(_q->X, _x, _q->M * sizeof(TI));

    // execute IFFT, store result in buffer 'x'
    FFT_EXECUTE(_q->ifft);

    // TODO: ignore this scaling
    // scale result by 1/num_channels (C transform)
    for (i=0; i<_q->M; i++)
        _q->x[i] *= 1.0f / (float)(_q->M);
    // scale result by num_channels/2
    for (i=0; i<_q->M; i++)
        _q->x[i] *= (float)(_q->M2);

    // push samples into appropriate buffer
    WINDOW() * buffer = (_q->flag == 0 ? _q->w1 : _q->w0);
    for (i=0; i<_q->M; i++)
        WINDOW(_push)(buffer[i], _q->x[i]);

    // compute filter outputs
    TO * r0, * r1;  // buffer read pointers
    TO   y0,   y1;  // dotprod outputs
    for (i=0; i<_q->M2; i++) {
        // buffer index
        unsigned int b = (_q->flag == 0) ? i : i+_q->M2;

        // read buffer with index offset
        WINDOW(_read)(_q->w0[b], &r0);
        WINDOW(_read)(_q->w1[b], &r1);

        // swap buffer outputs on alternating runs
        TO * p0 = _q->flag ? r0 : r1;
        TO * p1 = _q->flag ? r1 : r0;

        // run dot products
        DOTPROD(_execute)(_q->dp[i],        p0, &y0);
        DOTPROD(_execute)(_q->dp[i+_q->M2], p1, &y1);

        // save output
        _y[i] = y0 + y1;
    }
    _q->flag = 1 - _q->flag;
    return LIQUID_OK;
}

// execute filterbank channelizer
// LIQUID_ANALYZER:     input: M/2, output: M
// LIQUID_SYNTHESIZER:  input: M,   output: M/2
//  _x      :   channelizer input
//  _y      :   channelizer output
int FIRPFBCH2(_execute)(FIRPFBCH2() _q,
                        TI *        _x,
                        TO *        _y)
{
    switch (_q->type) {
    case LIQUID_ANALYZER:
        return FIRPFBCH2(_execute_analyzer)(_q, _x, _y);
    case LIQUID_SYNTHESIZER:
        return FIRPFBCH2(_execute_synthesizer)(_q, _x, _y);
    default:;
    }
    return liquid_error(LIQUID_EINT,"firpfbch2_%s_execute(), invalid internal type", EXTENSION_FULL);
}

