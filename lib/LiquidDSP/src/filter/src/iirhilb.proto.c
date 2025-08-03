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

// infinite impulse response (IIR) Hilbert transform

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct IIRHILB(_s) {
    // filter objects
    IIRFILT()       filt_0; // upper filter branch
    IIRFILT()       filt_1; // lower filter branch
    unsigned int    state;  // bookkeeping state
};

// create iirhilb object
//  _ftype  : filter type (e.g. LIQUID_IIRDES_BUTTER)
//  _n      : filter order, _n > 0
//  _ap     : pass-band ripple [dB], _ap > 0
//  _as     : stop-band ripple [dB], _as > 0
IIRHILB() IIRHILB(_create)(liquid_iirdes_filtertype _ftype,
                           unsigned int             _n,
                           float                    _ap,
                           float                    _as)
{
    // validate iirhilb inputs
    if (_n == 0)
        return liquid_error_config("iirhilb_create(), filter order must be greater than zero");

    // allocate memory for main object
    IIRHILB() q = (IIRHILB()) malloc(sizeof(struct IIRHILB(_s)));

    // design filters
    int     btype  = LIQUID_IIRDES_LOWPASS; // filter band type
    int     format = LIQUID_IIRDES_SOS;     // filter coefficients format
    float   fc     =   0.25f;               // cutoff frequency [normalized]
    float   f0     =   0.0f;                // center frequency [normalized]
    q->filt_0 = IIRFILT(_create_prototype)(_ftype,btype,format,_n,fc,f0,_ap,_as);
    q->filt_1 = IIRFILT(_create_prototype)(_ftype,btype,format,_n,fc,f0,_ap,_as);

    // reset internal state and return object
    IIRHILB(_reset)(q);
    return q;
}

// Create a default iirhilb object with a particular filter order.
//  _n      : filter order, _n > 0
IIRHILB() IIRHILB(_create_default)(unsigned int _n)
{
    // validate iirhilb inputs
    if (_n == 0)
        return liquid_error_config("iirhilb_create_default(), filter order must be greater than zero");

    int     ftype  = LIQUID_IIRDES_BUTTER;  // filter design type
    float   Ap     =   0.1f;                // pass-band ripple [dB]
    float   as     =   60.0f;               // stop-band attenuation [dB]
    return IIRHILB(_create)(ftype,_n,Ap,as);
}

IIRHILB() IIRHILB(_copy)(IIRHILB() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("iirhilb%s_copy(), object cannot be NULL", EXTENSION_SHORT);

    // create filter object and copy base parameters
    IIRHILB() q_copy = (IIRHILB()) malloc(sizeof(struct IIRHILB(_s)));
    memmove(q_copy, q_orig, sizeof(struct IIRHILB(_s)));

    // copy objects and return
    q_copy->filt_0 = IIRFILT(_copy)(q_orig->filt_0);
    q_copy->filt_1 = IIRFILT(_copy)(q_orig->filt_1);

    return q_copy;
}

// destroy iirhilb object
int IIRHILB(_destroy)(IIRHILB() _q)
{
    // destroy window buffers
    int rc_0 = IIRFILT(_destroy)(_q->filt_0);
    int rc_1 = IIRFILT(_destroy)(_q->filt_1);

    // free main object memory
    free(_q);

    return (rc_0 == LIQUID_OK && rc_1 == LIQUID_OK) ?
        LIQUID_OK :
        liquid_error(LIQUID_EINT,"iirhilb%s_destroy(), could not destroy object", EXTENSION_SHORT);
}

// print iirhilb object internals
int IIRHILB(_print)(IIRHILB() _q)
{
    printf("<liquid.iirhilb>\n");
    return LIQUID_OK;
}

// reset iirhilb object internal state
int IIRHILB(_reset)(IIRHILB() _q)
{
    // clear window buffers
    int rc_0 = IIRFILT(_reset)(_q->filt_0);
    int rc_1 = IIRFILT(_reset)(_q->filt_1);

    // reset state flag
    _q->state = 0;
    return (rc_0 == LIQUID_OK && rc_1 == LIQUID_OK) ?
        LIQUID_OK :
        liquid_error(LIQUID_EINT,"iirhilb%s_reset(), could not reset object", EXTENSION_SHORT);
}

// execute Hilbert transform (real to complex)
//  _q      :   iirhilb object
//  _x      :   real-valued input sample
//  _y      :   complex-valued output sample
int IIRHILB(_r2c_execute)(IIRHILB()   _q,
                          T           _x,
                          T complex * _y)
{
    // compute relevant output depending on state
    T yi = 0;
    T yq = 0;
    switch ( _q->state ) {
    case 0:
        IIRFILT(_execute)(_q->filt_0,  _x, &yi);
        IIRFILT(_execute)(_q->filt_1,   0, &yq);
        *_y = 2*(yi + _Complex_I*yq);
        break;
    case 1:
        IIRFILT(_execute)(_q->filt_0,   0, &yi);
        IIRFILT(_execute)(_q->filt_1, -_x, &yq);
        *_y = 2*(-yq + _Complex_I*yi);
        break;
    case 2:
        IIRFILT(_execute)(_q->filt_0, -_x, &yi);
        IIRFILT(_execute)(_q->filt_1,   0, &yq);
        *_y = 2*(-yi - _Complex_I*yq);
        break;
    case 3:
        IIRFILT(_execute)(_q->filt_0,   0, &yi);
        IIRFILT(_execute)(_q->filt_1,  _x, &yq);
        *_y = 2*( yq - _Complex_I*yi);
        break;
    default:;
    }

    // cycle through state
    _q->state = (_q->state + 1) & 0x3;
    return LIQUID_OK;
}

// Execute Hilbert transform (real to complex) on a block of samples
int IIRHILB(_r2c_execute_block)(IIRHILB()    _q,
                                T *          _x,
                                unsigned int _n,
                                T complex *  _y)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        IIRHILB(_r2c_execute)(_q, _x[i], _y+i);
    return LIQUID_OK;
}

// execute Hilbert transform (complex to real)
//  _q      :   iirhilb object
//  _y      :   complex-valued input sample
//  _x      :   real-valued output sample
int IIRHILB(_c2r_execute)(IIRHILB() _q,
                          T complex _x,
                          T *       _y)
{
    // compute relevant output depending on state
    T yi = 0;
    T yq = 0;
    switch ( _q->state ) {
    case 0:
        IIRFILT(_execute)(_q->filt_0,  crealf(_x), &yi);
        IIRFILT(_execute)(_q->filt_1,  cimagf(_x), &yq);
        *_y = yi;
        break;
    case 1:
        IIRFILT(_execute)(_q->filt_0,  cimagf(_x), &yi);
        IIRFILT(_execute)(_q->filt_1, -crealf(_x), &yq);
        *_y = -yq;
        break;
    case 2:
        IIRFILT(_execute)(_q->filt_0, -crealf(_x), &yi);
        IIRFILT(_execute)(_q->filt_1, -cimagf(_x), &yq);
        *_y = -yi;
        break;
    case 3:
        IIRFILT(_execute)(_q->filt_0, -cimagf(_x), &yi);
        IIRFILT(_execute)(_q->filt_1,  crealf(_x), &yq);
        *_y = yq;
        break;
    default:;
    }

    // cycle through state
    _q->state = (_q->state + 1) & 0x3;
    return LIQUID_OK;
}
// Execute Hilbert transform (complex to real) on a block of samples
int IIRHILB(_c2r_execute_block)(IIRHILB()    _q,
                                T complex *  _x,
                                unsigned int _n,
                                T *          _y)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        IIRHILB(_c2r_execute)(_q, _x[i], _y+i);
    return LIQUID_OK;
}

// execute Hilbert transform decimator (real to complex)
//  _q      :   iirhilb object
//  _x      :   real-valued input array [size: 2 x 1]
//  _y      :   complex-valued output sample
int IIRHILB(_decim_execute)(IIRHILB()   _q,
                            T *         _x,
                            T complex * _y)
{
    // mix down by Fs/4
    T xi = _q->state ? -_x[0] :  _x[0];
    T xq = _q->state ?  _x[1] : -_x[1];

    // upper branch
    T yi0, yi1;
    IIRFILT(_execute)(_q->filt_0, xi, &yi0);
    IIRFILT(_execute)(_q->filt_0,  0, &yi1);

    // lower branch
    T yq0, yq1;
    IIRFILT(_execute)(_q->filt_1,  0, &yq0);
    IIRFILT(_execute)(_q->filt_1, xq, &yq1);

    // set return value
    *_y = 2*(yi0 + _Complex_I*yq0);

    // toggle state flag
    _q->state = 1 - _q->state;
    return LIQUID_OK;
}

// execute Hilbert transform decimator (real to complex) on
// a block of samples
//  _q      :   Hilbert transform object
//  _x      :   real-valued input array [size: 2*_n x 1]
//  _n      :   number of *output* samples
//  _y      :   complex-valued output array [size: _n x 1]
int IIRHILB(_decim_execute_block)(IIRHILB()    _q,
                                  T *          _x,
                                  unsigned int _n,
                                  T complex *  _y)
{
    unsigned int i;

    for (i=0; i<_n; i++)
        IIRHILB(_decim_execute)(_q, &_x[2*i], &_y[i]);
    return LIQUID_OK;
}

// execute Hilbert transform interpolator (complex to real)
//  _q      :   iirhilb object
//  _y      :   complex-valued input sample
//  _x      :   real-valued output array [size: 2 x 1]
int IIRHILB(_interp_execute)(IIRHILB() _q,
                             T complex _x,
                             T *       _y)
{
    // upper branch
    T yi0, yi1;
    IIRFILT(_execute)(_q->filt_0, crealf(_x), &yi0);
    IIRFILT(_execute)(_q->filt_0,          0, &yi1);

    // lower branch
    T yq0, yq1;
    IIRFILT(_execute)(_q->filt_1, cimagf(_x), &yq0);
    IIRFILT(_execute)(_q->filt_1,          0, &yq1);

    // mix up by Fs/4 and retain real component
    //     {yi0 + j yq0, yi1 + j yq1}
    // 0: Re{+1 (yi0 + j yq0), +j (yi1 + j yq1)} = Re{ yi0 + j yq0, -yq1 + j yi1} = { yi0, -yq1}
    // 1: Re{-1 (yi0 + j yq0), -j (yi1 + j yq1)} = Re{-yi0 - j yq0,  yq1 - j yi1} = {-yi0,  yq1}
    _y[0] = 2*(_q->state ? -yi0 :  yi0);
    _y[1] = 2*(_q->state ?  yq1 : -yq1);

    // toggle state flag
    _q->state = 1 - _q->state;
    return LIQUID_OK;
}

// execute Hilbert transform interpolator (complex to real)
// on a block of samples
//  _q      :   Hilbert transform object
//  _x      :   complex-valued input array [size: _n x 1]
//  _n      :   number of *input* samples
//  _y      :   real-valued output array [size: 2*_n x 1]
int IIRHILB(_interp_execute_block)(IIRHILB()    _q,
                                   T complex *  _x,
                                   unsigned int _n,
                                   T *          _y)
{
    unsigned int i;

    for (i=0; i<_n; i++)
        IIRHILB(_interp_execute)(_q, _x[i], &_y[2*i]);
    return LIQUID_OK;
}
