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

// firpfbch : finite impulse response polyphase filterbank channelizer

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// firpfbch object structure definition
struct FIRPFBCH(_s) {
    int type;                   // synthesis/analysis
    unsigned int num_channels;  // number of channels
    unsigned int p;             // filter length (symbols)

    // filter
    unsigned int h_len;         // filter length
    TC * h;                     // filter coefficients
    
    // create separate bank of dotprod and window objects
    DOTPROD() * dp;             // dot product object array
    WINDOW() * w;               // window buffer object array
    unsigned int filter_index;  // running filter index (analysis)

    // fft plan
    FFT_PLAN fft;               // fft|ifft object
    TO * x;                     // fft|ifft transform input array
    TO * X;                     // fft|ifft transform output array
};

// 
// forward declaration of internal methods
//

int FIRPFBCH(_analyzer_push)(FIRPFBCH() _q, TI _x);

int FIRPFBCH(_analyzer_run)(FIRPFBCH()   _q,
                            unsigned int _k,
                            TO *         _X);


// create FIR polyphase filterbank channelizer object
//  _type   : channelizer type (LIQUID_ANALYZER | LIQUID_SYNTHESIZER)
//  _M      : number of channels
//  _p      : filter length (symbols)
//  _h      : filter coefficients, [size: _M*_p x 1]
FIRPFBCH() FIRPFBCH(_create)(int          _type,
                             unsigned int _M,
                             unsigned int _p,
                             TC *         _h)
{
    // validate input
    if (_type != LIQUID_ANALYZER && _type != LIQUID_SYNTHESIZER)
        return liquid_error_config("firpfbch_%s_create(), invalid type: %d", EXTENSION_FULL, _type);
    if (_M == 0)
        return liquid_error_config("firpfbch_%s_create(), number of channels must be greater than 0", EXTENSION_FULL);
    if (_p == 0)
        return liquid_error_config("firpfbch_%s_create(), invalid filter size (must be greater than 0)", EXTENSION_FULL);

    // create main object
    FIRPFBCH() q = (FIRPFBCH()) malloc(sizeof(struct FIRPFBCH(_s)));

    // set user-defined properties
    q->type         = _type;
    q->num_channels = _M;
    q->p            = _p;

    // derived values
    q->h_len = q->num_channels * q->p;

    // create bank of filters
    q->dp = (DOTPROD()*) malloc((q->num_channels)*sizeof(DOTPROD()));
    q->w  = (WINDOW()*)  malloc((q->num_channels)*sizeof(WINDOW()));

    // copy filter coefficients
    q->h = (TC*) malloc((q->h_len)*sizeof(TC));
    unsigned int i;
    for (i=0; i<q->h_len; i++)
        q->h[i] = _h[i];

    // generate bank of sub-samped filters
    unsigned int n;
    unsigned int h_sub_len = q->p;
    TC h_sub[h_sub_len];
    for (i=0; i<q->num_channels; i++) {
        // sub-sample prototype filter, loading coefficients in reverse order
        for (n=0; n<h_sub_len; n++) {
            h_sub[h_sub_len-n-1] = q->h[i + n*(q->num_channels)];
        }
        // create window buffer and dotprod object (coefficients
        // loaded in reverse order)
        q->dp[i] = DOTPROD(_create)(h_sub,h_sub_len);
        q->w[i]  = WINDOW(_create)(h_sub_len);
    }

    // allocate memory for buffers
    q->x = (T*) FFT_MALLOC((q->num_channels)*sizeof(T));
    q->X = (T*) FFT_MALLOC((q->num_channels)*sizeof(T));

    // create fft plan
    if (q->type == LIQUID_ANALYZER)
        q->fft = FFT_CREATE_PLAN(q->num_channels, q->X, q->x, FFT_DIR_FORWARD, FFT_METHOD);
    else
        q->fft = FFT_CREATE_PLAN(q->num_channels, q->X, q->x, FFT_DIR_BACKWARD, FFT_METHOD);

    // reset filterbank object
    FIRPFBCH(_reset)(q);

    // return filterbank object
    return q;
}

// create FIR polyphase filterbank channelizer object with
// prototype filter based on windowed Kaiser design
//  _type   : channelizer type (LIQUID_ANALYZER | LIQUID_SYNTHESIZER)
//  _M      : number of channels
//  _m      : filter delay (symbols)
//  _as     : stop-band attenuation [dB]
FIRPFBCH() FIRPFBCH(_create_kaiser)(int          _type,
                                    unsigned int _M,
                                    unsigned int _m,
                                    float        _as)
{
    // validate input
    if (_type != LIQUID_ANALYZER && _type != LIQUID_SYNTHESIZER)
        return liquid_error_config("firpfbch_%s_create_kaiser(), invalid type: %d", EXTENSION_FULL, _type);
    if (_M == 0)
        return liquid_error_config("firpfbch_%s_create_kaiser(), number of channels must be greater than 0", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("firpfbch_%s_create_kaiser(), invalid filter size (must be greater than 0)", EXTENSION_FULL);
    
    _as = fabsf(_as);

    // design filter
    unsigned int h_len = 2*_M*_m + 1;
    float h[h_len];
    float fc = 0.5f / (float)_M; // TODO : check this value
    liquid_firdes_kaiser(h_len, fc, _as, 0.0f, h);

    // copy coefficients to type-specfic array
    TC hc[h_len];
    unsigned int i;
    for (i=0; i<h_len; i++)
        hc[i] = h[i];

    // create filterbank object
    unsigned int p = 2*_m;
    FIRPFBCH() q = FIRPFBCH(_create)(_type, _M, p, hc);

    // return filterbank object
    return q;
}

// create FIR polyphase filterbank channelizer object with
// prototype root-Nyquist filter
//  _type   : channelizer type (LIQUID_ANALYZER | LIQUID_SYNTHESIZER)
//  _M      : number of channels
//  _m      : filter delay (symbols)
//  _beta   : filter excess bandwidth factor, in [0,1]
//  _ftype  : filter prototype (rrcos, rkaiser, etc.)
FIRPFBCH() FIRPFBCH(_create_rnyquist)(int          _type,
                                      unsigned int _M,
                                      unsigned int _m,
                                      float        _beta,
                                      int          _ftype)
{
    // validate input
    if (_type != LIQUID_ANALYZER && _type != LIQUID_SYNTHESIZER)
        return liquid_error_config("firpfbch_%s_create_rnyquist(), invalid type: %d", EXTENSION_FULL, _type);
    if (_M == 0)
        return liquid_error_config("firpfbch_%s_create_rnyquist(), number of channels must be greater than 0", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("firpfbch_%s_create_rnyquist(), invalid filter size (must be greater than 0)", EXTENSION_FULL);
    
    // design filter based on requested prototype
    unsigned int h_len = 2*_M*_m + 1;
    float h[h_len];
    if (liquid_firdes_prototype(_ftype, _M, _m, _beta, 0.0f, h) != LIQUID_OK)
        return liquid_error_config("firpfbch_%s_create_rnyquist(), invalid filter type/configuration", EXTENSION_FULL);

    // copy coefficients to type-specfic array, reversing order if
    // channelizer is an analyzer, matched filter: g(-t)
    unsigned int g_len = 2*_M*_m;
    TC gc[g_len];
    unsigned int i;
    if (_type == LIQUID_SYNTHESIZER) {
        for (i=0; i<g_len; i++)
            gc[i] = h[i];
    } else {
        for (i=0; i<g_len; i++)
            gc[i] = h[g_len-i-1];
    }

    // create filterbank object
    unsigned int p = 2*_m;
    FIRPFBCH() q = FIRPFBCH(_create)(_type, _M, p, gc);

    // return filterbank object
    return q;
}

// destroy firpfbch object
int FIRPFBCH(_destroy)(FIRPFBCH() _q)
{
    unsigned int i;

    // free dot product, window objects and arrays
    for (i=0; i<_q->num_channels; i++) {
        DOTPROD(_destroy)(_q->dp[i]);
        WINDOW(_destroy)(_q->w[i]);
    }
    free(_q->dp);
    free(_q->w);

    // free transform object
    FFT_DESTROY_PLAN(_q->fft);

    // free additional arrays
    free(_q->h);
    FFT_FREE(_q->x);
    FFT_FREE(_q->X);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// clear/reset firpfbch object internals
int FIRPFBCH(_reset)(FIRPFBCH() _q)
{
    unsigned int i;
    for (i=0; i<_q->num_channels; i++) {
        WINDOW(_reset)(_q->w[i]);
        _q->x[i] = 0;
        _q->X[i] = 0;
    }
    _q->filter_index = _q->num_channels-1;
    return LIQUID_OK;
}

// print firpfbch object
int FIRPFBCH(_print)(FIRPFBCH() _q)
{
    printf("<liquid.firpfbch, type=\"%s\", channels=%u, semilen=%u>\n",
        _q->type == LIQUID_ANALYZER ? "analyzer" : "synthesizer",
        _q->num_channels, _q->p);
    return LIQUID_OK;
}

// 
// SYNTHESIZER
//

// execute filterbank as synthesizer on block of samples
//  _q      :   filterbank channelizer object
//  _x      :   channelized input, [size: num_channels x 1]
//  _y      :   output time series, [size: num_channels x 1]
int FIRPFBCH(_synthesizer_execute)(FIRPFBCH() _q,
                                   TI *       _x,
                                   TO *       _y)
{
    unsigned int i;

    // copy channelized symbols to transform input
    memmove(_q->X, _x, _q->num_channels*sizeof(TI));

    // execute inverse DFT, store result in buffer 'x'
    FFT_EXECUTE(_q->fft);

    // push samples into filter bank and execute
    T * r;      // read pointer
    for (i=0; i<_q->num_channels; i++) {
        WINDOW(_push)(_q->w[i], _q->x[i]);
        WINDOW(_read)(_q->w[i], &r);
        DOTPROD(_execute)(_q->dp[i], r, &_y[i]);

        // normalize by DFT scaling factor
        //_y[i] /= (float) (_q->num_channels);
    }
    return LIQUID_OK;
}

// 
// ANALYZER
//

// execute filterbank as analyzer on block of samples
//  _q      :   filterbank channelizer object
//  _x      :   input time series, [size: num_channels x 1]
//  _y      :   channelized output, [size: num_channels x 1]
int FIRPFBCH(_analyzer_execute)(FIRPFBCH() _q,
                                TI *       _x,
                                TO *       _y)
{
    unsigned int i;

    // push samples into buffers
    for (i=0; i<_q->num_channels; i++)
        FIRPFBCH(_analyzer_push)(_q, _x[i]);

    // execute analysis filters on the given input starting
    // with filterbank at index zero
    return FIRPFBCH(_analyzer_run)(_q, 0, _y);
}

// 
// internal methods
//

// push single sample into analysis filterbank, updating index
// counter appropriately
//  _q      :   filterbank channelizer object
//  _x      :   input sample
int FIRPFBCH(_analyzer_push)(FIRPFBCH() _q,
                             TI         _x)
{
    // push sample into filter
    WINDOW(_push)(_q->w[_q->filter_index], _x);

    // decrement filter index
    _q->filter_index = (_q->filter_index + _q->num_channels - 1) % _q->num_channels;
    return LIQUID_OK;
}

// run filterbank analyzer dot products, DFT
//  _q      :   filterbank channelizer object
//  _k      :   filterbank alignment index
//  _y      :   output array, [size: num_channels x 1]
int FIRPFBCH(_analyzer_run)(FIRPFBCH()   _q,
                            unsigned int _k,
                            TO *         _y)
{
    unsigned int i;

    // execute filter outputs, reversing order of output (not
    // sure why this is necessary)
    T * r;  // read pointer
    unsigned int index;
    for (i=0; i<_q->num_channels; i++) {
        // compute appropriate index
        index = (i+_k) % _q->num_channels;

        // read buffer at specified index
        WINDOW(_read)(_q->w[index], &r);

        // compute dot product
        DOTPROD(_execute)(_q->dp[i], r, &_q->X[_q->num_channels-i-1]);
    }

    // execute DFT, store result in buffer 'x'
    FFT_EXECUTE(_q->fft);

    // move to output array
    memmove(_y, _q->x, _q->num_channels*sizeof(TO));
    return LIQUID_OK;
}

