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

// finite impulse response decimator object definitions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// decimator structure
struct FIRDECIM(_s) {
    TC *            h;      // coefficients array
    unsigned int    h_len;  // number of coefficients
    unsigned int    M;      // decimation factor

    WINDOW()        w;      // buffer
    DOTPROD()       dp;     // vector dot product
    TC              scale;  // output scaling factor
};

// create decimator object
//  _M      :   decimation factor
//  _h      :   filter coefficients [size: _h_len x 1]
//  _h_len  :   filter coefficients length
FIRDECIM() FIRDECIM(_create)(unsigned int _M,
                             TC *         _h,
                             unsigned int _h_len)
{
    // validate input
    if (_h_len == 0)
        return liquid_error_config("decim_%s_create(), filter length must be greater than zero", EXTENSION_FULL);
    if (_M == 0)
        return liquid_error_config("decim_%s_create(), decimation factor must be greater than zero", EXTENSION_FULL);

    FIRDECIM() q = (FIRDECIM()) malloc(sizeof(struct FIRDECIM(_s)));
    q->h_len = _h_len;
    q->M     = _M;

    // allocate memory for coefficients
    q->h = (TC*) malloc((q->h_len)*sizeof(TC));

    // load filter in reverse order
    unsigned int i;
    for (i=0; i<q->h_len; i++)
        q->h[i] = _h[_h_len-i-1];

    // create window (internal buffer)
    q->w = WINDOW(_create)(q->h_len);

    // create dot product object
    q->dp = DOTPROD(_create)(q->h, q->h_len);

    // set default scaling
    q->scale = 1;

    // reset filter state (clear buffer)
    FIRDECIM(_reset)(q);

    return q;
}

// create decimator from Kaiser prototype
//  _M      :   decimolation factor
//  _m      :   symbol delay
//  _as     :   stop-band attenuation [dB]
FIRDECIM() FIRDECIM(_create_kaiser)(unsigned int _M,
                                    unsigned int _m,
                                    float        _as)
{
    // validate input
    if (_M < 2)
        return liquid_error_config("decim_%s_create_kaiser(), decim factor must be greater than 1", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("decim_%s_create_kaiser(), filter delay must be greater than 0", EXTENSION_FULL);
    if (_as < 0.0f)
        return liquid_error_config("decim_%s_create_kaiser(), stop-band attenuation must be positive", EXTENSION_FULL);

    // compute filter coefficients (floating point precision)
    unsigned int h_len = 2*_M*_m + 1;
    float hf[h_len];
    float fc = 0.5f / (float) (_M);
    liquid_firdes_kaiser(h_len, fc, _as, 0.0f, hf);

    // copy coefficients to type-specific array (e.g. float complex)
    TC hc[h_len];
    unsigned int i;
    for (i=0; i<h_len; i++)
        hc[i] = hf[i];
    
    // return decimator object
    return FIRDECIM(_create)(_M, hc, h_len);
}

// create square-root Nyquist decimator
//  _type   :   filter type (e.g. LIQUID_FIRFILT_RRC)
//  _M      :   samples/symbol _M > 1
//  _m      :   filter delay (symbols), _m > 0
//  _beta   :   excess bandwidth factor, 0 < _beta < 1
//  _dt     :   fractional sample delay, 0 <= _dt < 1
FIRDECIM() FIRDECIM(_create_prototype)(int          _type,
                                       unsigned int _M,
                                       unsigned int _m,
                                       float        _beta,
                                       float        _dt)
{
    // validate input
    if (_M < 2)
        return liquid_error_config("decim_%s_create_prototype(), decimation factor must be greater than 1", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("decim_%s_create_prototype(), filter delay must be greater than 0", EXTENSION_FULL);
    if (_beta < 0.0f || _beta > 1.0f)
        return liquid_error_config("decim_%s_create_prototype(), filter excess bandwidth factor must be in [0,1]", EXTENSION_FULL);
    if (_dt < -1.0f || _dt > 1.0f)
        return liquid_error_config("decim_%s_create_prototype(), filter fractional sample delay must be in [-1,1]", EXTENSION_FULL);

    // generate square-root Nyquist filter
    unsigned int h_len = 2*_M*_m + 1;
    float h[h_len];
    if (liquid_firdes_prototype(_type,_M,_m,_beta,_dt,h) != LIQUID_OK)
        return liquid_error_config("decim_%s_create_prototype(), could not design internal filter", EXTENSION_FULL);

    // copy coefficients to type-specific array (e.g. float complex)
    unsigned int i;
    TC hc[h_len];
    for (i=0; i<h_len; i++)
        hc[i] = h[i];

    // return decimator object
    return FIRDECIM(_create)(_M, hc, h_len);
}

// copy object
FIRDECIM() FIRDECIM(_copy)(FIRDECIM() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firfilt_%s_create(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy base parameters
    FIRDECIM() q_copy = (FIRDECIM()) malloc(sizeof(struct FIRDECIM(_s)));
    q_copy->h_len = q_orig->h_len;
    q_copy->M     = q_orig->M;
    q_copy->h     = (TC *) malloc((q_orig->h_len)*sizeof(TC));
    memmove(q_copy->h, q_orig->h, (q_orig->h_len)*sizeof(TC));

    // copy window, dotprod,m and scale
    q_copy->w     = WINDOW(_copy)(q_orig->w);
    q_copy->dp    = DOTPROD(_copy)(q_orig->dp);
    q_copy->scale = q_orig->scale;
    return q_copy;
}

// destroy decimator object
int FIRDECIM(_destroy)(FIRDECIM() _q)
{
    WINDOW(_destroy)(_q->w);
    DOTPROD(_destroy)(_q->dp);
    free(_q->h);
    free(_q);
    return LIQUID_OK;
}

// print decimator object internals
int FIRDECIM(_print)(FIRDECIM() _q)
{
    printf("<liquid.firdecim, decim=%u", _q->M);
    // print scaling
    printf(", scale=");
    PRINTVAL_TC(_q->scale,%g);
    printf(">\n");
    return LIQUID_OK;
}

// reset decimator object
int FIRDECIM(_reset)(FIRDECIM() _q)
{
    return WINDOW(_reset)(_q->w);
}

// Get decimation rate
unsigned int FIRDECIM(_get_decim_rate)(FIRDECIM() _q)
{
    return _q->M;
}

// Set output scaling for decimator
//  _q      : decimator object
//  _scale  : scaling factor to apply to each output sample
int FIRDECIM(_set_scale)(FIRDECIM() _q,
                          TC         _scale)
{
    _q->scale = _scale;
    return LIQUID_OK;
}

// Get output scaling for decimator
//  _q      : decimator object
//  _scale  : scaling factor to apply to each output sample
int FIRDECIM(_get_scale)(FIRDECIM() _q,
                         TC *       _scale)
{
    *_scale = _q->scale;
    return LIQUID_OK;
}

// Compute complex frequency response \(H\) of decimator on prototype
// filter coefficients at a specific frequency \(f_c\)
//  _q      : decimator object
//  _fc     : normalized frequency for evaluation
//  _H      : pointer to output complex frequency response
int FIRDECIM(_freqresp)(FIRDECIM()             _q,
                        float                  _fc,
                        liquid_float_complex * _H)
{
#if TC_COMPLEX==0
    int rc = liquid_freqrespf(_q->h, _q->h_len, _fc, _H);
#elif TC_COMPLEX==1
    int rc = liquid_freqrespcf(_q->h, _q->h_len, _fc, _H);
#else
#   error("invalid complex type for coefficients")
#endif

    // apply scaling
    *_H *= _q->scale;
    return rc;
}

// execute decimator
//  _q      :   decimator object
//  _x      :   input sample array [size: _M x 1]
//  _y      :   output sample pointer
int FIRDECIM(_execute)(FIRDECIM() _q,
                       TI *       _x,
                       TO *       _y)
{
    TI * r; // read pointer
    unsigned int i;
    for (i=0; i<_q->M; i++) {
        WINDOW(_push)(_q->w, _x[i]);

        if (i==0) {
            // read buffer (retrieve pointer to aligned memory array)
            WINDOW(_read)(_q->w, &r);

            // execute dot product
            DOTPROD(_execute)(_q->dp, r, _y);

            // apply scaling factor
            *_y *= _q->scale;
        }
    }
    return LIQUID_OK;
}

// execute decimator on block of _n*_M input samples
//  _q      : decimator object
//  _x      : input array [size: _n*_M x 1]
//  _n      : number of _output_ samples
//  _y      : output array [_size: _n x 1]
int FIRDECIM(_execute_block)(FIRDECIM()   _q,
                             TI *         _x,
                             unsigned int _n,
                             TO *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        // execute _M input samples computing just one output each time
        FIRDECIM(_execute)(_q, &_x[i*_q->M], &_y[i]);
    }
    return LIQUID_OK;
}

