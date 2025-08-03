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
// finite impulse response (FIR) polyphase filter bank (PFB)
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct FIRPFB(_s) {
    unsigned int h_len;         // total number of filter coefficients
    unsigned int h_sub_len;     // sub-sampled filter length
    unsigned int num_filters;   // number of filters

    WINDOW() w;                 // window buffer
    DOTPROD() * dp;             // array of vector dot product objects
    TC scale;                   // output scaling factor
};

// create firpfb from external coefficients
//  _num_filters : number of filters in the bank
//  _h           : coefficients [size: _num_filters*_h_len x 1]
//  _h_len       : filter delay (symbols)
FIRPFB() FIRPFB(_create)(unsigned int _num_filters,
                         TC *         _h,
                         unsigned int _h_len)
{
    // validate input
    if (_num_filters == 0)
        return liquid_error_config("firpfb_%s_create(), number of filters must be greater than zero",EXTENSION_FULL);
    if (_h_len == 0)
        return liquid_error_config("firpfb_%s_create(), filter length must be greater than zero",EXTENSION_FULL);

    // create main filter object
    FIRPFB() q = (FIRPFB()) malloc(sizeof(struct FIRPFB(_s)));

    // set user-defined parameters
    q->num_filters = _num_filters;
    q->h_len       = _h_len;

    // each filter is realized as a dotprod object
    q->dp = (DOTPROD()*) malloc((q->num_filters)*sizeof(DOTPROD()));

    // generate bank of sub-samped filters
    // length of each sub-sampled filter
    unsigned int h_sub_len = _h_len / q->num_filters;
    TC h_sub[h_sub_len];
    unsigned int i, n;
    for (i=0; i<q->num_filters; i++) {
        for (n=0; n<h_sub_len; n++) {
            // load filter in reverse order
            h_sub[h_sub_len-n-1] = _h[i + n*(q->num_filters)];
        }

        // create dot product object
        q->dp[i] = DOTPROD(_create)(h_sub,h_sub_len);
    }

    // save sub-sampled filter length
    q->h_sub_len = h_sub_len;

    // create window buffer
    q->w = WINDOW(_create)(q->h_sub_len);

    // set default scaling
    q->scale = 1;

    // reset object and return
    FIRPFB(_reset)(q);
    return q;
}

// create default firpfb
FIRPFB() FIRPFB(_create_default)(unsigned int _num_filters,
                                 unsigned int _m)
{
    return FIRPFB(_create_kaiser)(_num_filters, _m, 0.5f, 60.0f);
}

// create firpfb using kaiser window
//  _num_filters : number of filters in the bank
//  _m           : filter semi-length [samples]
//  _fc          : filter cut-off frequency 0 < _fc < 0.5
//  _as          : filter stop-band suppression [dB]
FIRPFB() FIRPFB(_create_kaiser)(unsigned int _num_filters,
                                unsigned int _m,
                                float        _fc,
                                float        _as)
{
    // validate input
    if (_num_filters == 0)
        return liquid_error_config("firpfb_%s_create_kaiser(), number of filters must be greater than zero", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("firpfb_%s_create_kaiser(), filter delay must be greater than 0", EXTENSION_FULL);
    if (_fc < 0.0f || _fc > 0.5f)
        return liquid_error_config("firpfb_%s_create_kaiser(), filter cut-off frequence must be in (0,0.5)", EXTENSION_FULL);
    if (_as < 0.0f)
        return liquid_error_config("firpfb_%s_create_kaiser(), filter excess bandwidth factor must be in [0,1]", EXTENSION_FULL);

    // design filter using kaiser window
    unsigned int H_len = 2*_num_filters*_m + 1;
    float Hf[H_len];
    liquid_firdes_kaiser(H_len, _fc/(float)_num_filters, _as, 0.0f, Hf);

    // copy coefficients to type-specific array (e.g. float complex)
    unsigned int i;
    TC Hc[H_len];
    for (i=0; i<H_len; i++)
        Hc[i] = Hf[i];

    // return filterbank object
    return FIRPFB(_create)(_num_filters, Hc, H_len);
}

// create square-root Nyquist filterbank
//  _type        :   filter type (e.g. LIQUID_FIRFILT_RRC)
//  _num_filters :   number of filters in the bank
//  _k           :   samples/symbol _k > 1
//  _m           :   filter delay (symbols), _m > 0
//  _beta        :   excess bandwidth factor, 0 < _beta < 1
FIRPFB() FIRPFB(_create_rnyquist)(int          _type,
                                  unsigned int _num_filters,
                                  unsigned int _k,
                                  unsigned int _m,
                                  float        _beta)
{
    // validate input
    if (_num_filters == 0)
        return liquid_error_config("firpfb_%s_create_rnyquist(), number of filters must be greater than zero", EXTENSION_FULL);
    if (_k < 2)
        return liquid_error_config("firpfb_%s_create_rnyquist(), filter samples/symbol must be greater than 1", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("firpfb_%s_create_rnyquist(), filter delay must be greater than 0", EXTENSION_FULL);
    if (_beta < 0.0f || _beta > 1.0f)
        return liquid_error_config("firpfb_%s_create_rnyquist(), filter excess bandwidth factor must be in [0,1]", EXTENSION_FULL);

    // generate square-root Nyquist filter
    unsigned int H_len = 2*_num_filters*_k*_m + 1;
    float Hf[H_len];
    liquid_firdes_prototype(_type,_num_filters*_k,_m,_beta,0,Hf);

    // copy coefficients to type-specific array (e.g. float complex)
    unsigned int i;
    TC Hc[H_len];
    for (i=0; i<H_len; i++)
        Hc[i] = Hf[i];

    // return filterbank object
    return FIRPFB(_create)(_num_filters, Hc, H_len);
}

// create firpfb derivative square-root Nyquist filterbank
//  _type        :   filter type (e.g. LIQUID_FIRFILT_RRC)
//  _num_filters :   number of filters in the bank
//  _k           :   samples/symbol _k > 1
//  _m           :   filter delay (symbols), _m > 0
//  _beta        :   excess bandwidth factor, 0 < _beta < 1
FIRPFB() FIRPFB(_create_drnyquist)(int          _type,
                                   unsigned int _num_filters,
                                   unsigned int _k,
                                   unsigned int _m,
                                   float        _beta)
{
    // validate input
    if (_num_filters == 0)
        return liquid_error_config("firpfb_%s_create_drnyquist(), number of filters must be greater than zero", EXTENSION_FULL);
    if (_k < 2)
        return liquid_error_config("firpfb_%s_create_drnyquist(), filter samples/symbol must be greater than 1", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("firpfb_%s_create_drnyquist(), filter delay must be greater than 0", EXTENSION_FULL);
    if (_beta < 0.0f || _beta > 1.0f)
        return liquid_error_config("firpfb_%s_create_drnyquist(), filter excess bandwidth factor must be in [0,1]", EXTENSION_FULL);

    // generate square-root Nyquist filter
    unsigned int H_len = 2*_num_filters*_k*_m + 1;
    float Hf[H_len];
    liquid_firdes_prototype(_type,_num_filters*_k,_m,_beta,0,Hf);
    
    // compute derivative filter
    float dHf[H_len];
    float HdH_max = 0.0f;
    unsigned int i;
    for (i=0; i<H_len; i++) {
        if (i==0) {
            dHf[i] = Hf[i+1] - Hf[H_len-1];
        } else if (i==H_len-1) {
            dHf[i] = Hf[0]   - Hf[i-1];
        } else {
            dHf[i] = Hf[i+1] - Hf[i-1];
        }

        // find maximum of h*dh
        if ( fabsf(Hf[i]*dHf[i]) > HdH_max )
            HdH_max = fabsf(Hf[i]*dHf[i]);
    }

    // copy coefficients to type-specific array (e.g. float complex)
    // and apply scaling factor for normalized response
    TC Hc[H_len];
    for (i=0; i<H_len; i++)
        Hc[i] = dHf[i] * 0.06f / HdH_max;

    // return filterbank object
    return FIRPFB(_create)(_num_filters, Hc, H_len);
}


// re-create filterbank object
//  _q           : original firpfb object
//  _num_filters :   number of filters in the bank
//  _h           : coefficients [size: _num_filters x _h_len]
//  _h_len       : length of each filter
FIRPFB() FIRPFB(_recreate)(FIRPFB()     _q,
                           unsigned int _num_filters,
                           TC *         _h,
                           unsigned int _h_len)
{
    // check to see if filter length has changed
    if (_h_len != _q->h_len || _num_filters != _q->num_filters) {
        // filter length has changed: recreate entire filter
        FIRPFB(_destroy)(_q);
        _q = FIRPFB(_create)(_num_filters,_h,_h_len);
        return _q;
    }

    // re-create each dotprod object
    TC h_sub[_q->h_sub_len];
    unsigned int i, n;
    for (i=0; i<_q->num_filters; i++) {
        for (n=0; n<_q->h_sub_len; n++) {
            // load filter in reverse order
            h_sub[_q->h_sub_len-n-1] = _h[i + n*(_q->num_filters)];
        }

        _q->dp[i] = DOTPROD(_recreate)(_q->dp[i],h_sub,_q->h_sub_len);
    }
    return _q;
}

// copy object
FIRPFB() FIRPFB(_copy)(FIRPFB() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firpfb_%s_create(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy base parameters
    FIRPFB() q_copy     = (FIRPFB()) malloc(sizeof(struct FIRPFB(_s)));
    q_copy->h_len       = q_orig->h_len;
    q_copy->h_sub_len   = q_orig->h_sub_len;
    q_copy->num_filters = q_orig->num_filters;
    q_copy->w           = WINDOW(_copy)(q_orig->w);

    // copy array of dotproduct objects
    q_copy->dp = (DOTPROD()*) malloc((q_copy->num_filters)*sizeof(DOTPROD()));
    unsigned int i;
    for (i=0; i<q_copy->num_filters; i++)
        q_copy->dp[i] = DOTPROD(_copy)(q_orig->dp[i]);

    q_copy->scale = q_orig->scale;
    return q_copy;
}

// destroy firpfb object, freeing all internal memory
int FIRPFB(_destroy)(FIRPFB() _q)
{
    unsigned int i;
    for (i=0; i<_q->num_filters; i++)
        DOTPROD(_destroy)(_q->dp[i]);
    free(_q->dp);
    WINDOW(_destroy)(_q->w);
    free(_q);
    return LIQUID_OK;
}

// print firpfb object's parameters
int FIRPFB(_print)(FIRPFB() _q)
{
    printf("<liquid.firpfb_%s, num_filters=%u, len=%u",
        EXTENSION_FULL, _q->num_filters, _q->h_sub_len);
    printf(", scale=");
    PRINTVAL_TC(_q->scale,%g);
    printf(">\n");
    return LIQUID_OK;
}

// clear/reset firpfb object internal state
int FIRPFB(_reset)(FIRPFB() _q)
{
    return WINDOW(_reset)(_q->w);
}

// set output scaling for filter
int FIRPFB(_set_scale)(FIRPFB() _q,
                         TC      _scale)
{
    _q->scale = _scale;
    return LIQUID_OK;
}

// get output scaling for filter
int FIRPFB(_get_scale)(FIRPFB() _q,
                         TC *    _scale)
{
    *_scale = _q->scale;
    return LIQUID_OK;
}

// push sample into firpfb internal buffer
int FIRPFB(_push)(FIRPFB() _q, TI _x)
{
    // push value into window buffer
    return WINDOW(_push)(_q->w, _x);
}

// Write a block of samples into object's internal buffer
int FIRPFB(_write)(FIRPFB()     _q,
                    TI *         _x,
                    unsigned int _n)
{
    return WINDOW(_write)(_q->w, _x, _n);
}

// execute the filter on internal buffer and coefficients
//  _q      : firpfb object
//  _i      : index of filter to use
//  _y      : pointer to output sample
int FIRPFB(_execute)(FIRPFB()     _q,
                     unsigned int _i,
                     TO *         _y)
{
    // validate input
    if (_i >= _q->num_filters) {
        return liquid_error(LIQUID_EICONFIG,"firpfb_execute(), filterbank index (%u) exceeds maximum (%u)",_i,_q->num_filters);
    }

    // read buffer
    TI *r;
    WINDOW(_read)(_q->w, &r);

    // execute dot product
    DOTPROD(_execute)(_q->dp[_i], r, _y);

    // apply scaling factor
    *_y *= _q->scale;
    return LIQUID_OK;
}

// execute the filter on a block of input samples; the
// input and output buffers may be the same
//  _q      : firpfb object
//  _i      : index of filter to use
//  _x      : pointer to input array [size: _n x 1]
//  _n      : number of input, output samples
//  _y      : pointer to output array [size: _n x 1]
int FIRPFB(_execute_block)(FIRPFB()     _q,
                           unsigned int _i,
                           TI *         _x,
                           unsigned int _n,
                           TO *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        // push sample into filter
        FIRPFB(_push)(_q, _x[i]);

        // compute output at appropriate index
        FIRPFB(_execute)(_q, _i, &_y[i]);
    }
    return LIQUID_OK;
}

