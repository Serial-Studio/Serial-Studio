/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// firdecim.c
//
// finite impulse response decimator object definitions
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// decimator structure
struct IIRDECIM(_s) {
    unsigned int M;     // decimation factor

    // TODO: use IIR polyphase filterbank
    IIRFILT() iirfilt;  // filter object
};

// create interpolator from external coefficients
//  _M      : interpolation factor
//  _b      : feed-back coefficients [size: _nb x 1]
//  _nb     : feed-back coefficients length
//  _a      : feed-forward coefficients [size: _na x 1]
//  _na     : feed-forward coefficients length
IIRDECIM() IIRDECIM(_create)(unsigned int _M,
                             TC *         _b,
                             unsigned int _nb,
                             TC *         _a,
                             unsigned int _na)
{
    // validate input
    if (_M < 2)
        return liquid_error_config("iirinterp_%s_create(), interp factor must be greater than 1", EXTENSION_FULL);

    // allocate main object memory and set internal parameters
    IIRDECIM() q = (IIRDECIM()) malloc(sizeof(struct IIRDECIM(_s)));
    q->M = _M;

    // create filter
    q->iirfilt = IIRFILT(_create)(_b, _nb, _a, _na);

    // return interpolator object
    return q;
}

// create decimator with default Butterworth prototype
//  _M      : decimation factor
//  _order  : filter order
IIRDECIM() IIRDECIM(_create_default)(unsigned int _M,
                                     unsigned int _order)
{
    return IIRDECIM(_create_prototype)(_M,
                                       LIQUID_IIRDES_BUTTER,
                                       LIQUID_IIRDES_LOWPASS,
                                       LIQUID_IIRDES_SOS,
                                       _order,
                                       0.5f / (float)_M,    // fc
                                       0.0f,                // f0
                                       0.1f,                // pass-band ripple,
                                       60.0f);              // stop-band attenuation
}

// create interpolator from prototype
//  _M      :   interpolation factor
IIRDECIM() IIRDECIM(_create_prototype)(unsigned int             _M,
                                       liquid_iirdes_filtertype _ftype,
                                       liquid_iirdes_bandtype   _btype,
                                       liquid_iirdes_format     _format,
                                       unsigned int             _order,
                                       float                    _fc,
                                       float                    _f0,
                                       float                    _ap,
                                       float                    _as)
{
    // validate input
    if (_M < 2)
        return liquid_error_config("iirinterp_%s_create_prototype(), interp factor must be greater than 1", EXTENSION_FULL);

    // allocate main object memory and set internal parameters
    IIRDECIM() q = (IIRDECIM()) malloc(sizeof(struct IIRDECIM(_s)));
    q->M = _M;

    // create filter
    q->iirfilt = IIRFILT(_create_prototype)(_ftype, _btype, _format, _order, _fc, _f0, _ap, _as);

    // return interpolator object
    return q;
}

// copy object
IIRDECIM() IIRDECIM(_copy)(IIRDECIM() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("iirdecim%s_create(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy internal memory
    IIRDECIM() q_copy = (IIRDECIM()) malloc(sizeof(struct IIRDECIM(_s)));
    memmove(q_copy, q_orig, sizeof(struct IIRDECIM(_s)));

    // copy internal object and return
    q_copy->iirfilt = IIRFILT(_copy)(q_orig->iirfilt);
    return q_copy;
}

// destroy interpolator object
int IIRDECIM(_destroy)(IIRDECIM() _q)
{
    IIRFILT(_destroy)(_q->iirfilt);
    free(_q);
    return LIQUID_OK;
}

// print interpolator state
int IIRDECIM(_print)(IIRDECIM() _q)
{
    printf("<liquid.iirdecim_%s, decim=%u>\n", EXTENSION_FULL, _q->M);
    return LIQUID_OK;
}

// clear internal state
int IIRDECIM(_reset)(IIRDECIM() _q)
{
    return IIRFILT(_reset)(_q->iirfilt);
}

// execute decimator
//  _q      :   decimator object
//  _x      :   input sample array [size: _M x 1]
//  _y      :   output sample pointer
//  _index  :   decimator output index [0,_M-1]
int IIRDECIM(_execute)(IIRDECIM()   _q,
                       TI *         _x,
                       TO *         _y)
{
    TO v; // output value
    unsigned int i;
    for (i=0; i<_q->M; i++) {
        // run filter
        IIRFILT(_execute)(_q->iirfilt, _x[i], &v);

        // save output at appropriate index
        if (i==0)
            *_y = v;
    }
    return LIQUID_OK;
}

// execute decimator on block of _n*_M input samples
//  _q      : decimator object
//  _x      : input array [size: _n*_M x 1]
//  _n      : number of _output_ samples
//  _y      : output array [_sze: _n x 1]
int IIRDECIM(_execute_block)(IIRDECIM()   _q,
                             TI *         _x,
                             unsigned int _n,
                             TO *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        // execute _M input samples computing just one output each time
        IIRDECIM(_execute)(_q, &_x[i*_q->M], &_y[i]);
    }
    return LIQUID_OK;
}

// get system group delay at frequency _fc
//  _q      :   interpolator object
//  _f      :   frequency
float IIRDECIM(_groupdelay)(IIRDECIM() _q,
                            float      _fc)
{
    return IIRFILT(_groupdelay)(_q->iirfilt, _fc);
}

