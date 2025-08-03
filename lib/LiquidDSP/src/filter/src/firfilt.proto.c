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
// firfilt : finite impulse response (FIR) filter
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// defined:
//  FIRFILT()       name-mangling macro
//  T               coefficients type
//  WINDOW()        window macro
//  DOTPROD()       dotprod macro
//  PRINTVAL()      print macro

// NOTE: using the window is about 27% slower, but fixes a valgrind issue
#define LIQUID_FIRFILT_USE_WINDOW   (1)

// firfilt object structure
struct FIRFILT(_s) {
    TC * h;             // filter coefficients array [size; h_len x 1]
    unsigned int h_len; // filter length

#if LIQUID_FIRFILT_USE_WINDOW
    // use window object for internal buffer
    WINDOW() w;
#else
    // use array as internal buffer (faster)
    TI * w;                 // internal buffer object
    unsigned int w_len;     // window length
    unsigned int w_mask;    // window index mask
    unsigned int w_index;   // window read index
#endif
    DOTPROD() dp;           // dot product object
    TC scale;               // output scaling factor
};

// create firfilt object
//  _h      :   coefficients (filter taps) [size: _n x 1]
//  _n      :   filter length
FIRFILT() FIRFILT(_create)(TC * _h,
                           unsigned int _n)
{
    // validate input
    if (_n == 0)
        return liquid_error_config("firfilt_%s_create(), filter length must be greater than zero", EXTENSION_FULL);

    // create filter object and initialize
    FIRFILT() q = (FIRFILT()) malloc(sizeof(struct FIRFILT(_s)));
    q->h_len = _n;
    q->h = (TC *) malloc((q->h_len)*sizeof(TC));

#if LIQUID_FIRFILT_USE_WINDOW
    // create window (internal buffer)
    q->w = WINDOW(_create)(q->h_len);
#else
    // initialize array for buffering
    q->w_len   = 1<<liquid_msb_index(q->h_len); // effectively 2^{floor(log2(len))+1}
    q->w_mask  = q->w_len - 1;
    q->w       = (TI *) malloc((q->w_len + q->h_len + 1)*sizeof(TI));
    q->w_index = 0;
#endif

    // move coefficients
    memmove(q->h, _h, (q->h_len)*sizeof(TC));

    // create dot product object with coefficients in reverse order
    q->dp = DOTPROD(_create_rev)(q->h, q->h_len);

    // set default scaling
    q->scale = 1;

    // reset filter state (clear buffer)
    FIRFILT(_reset)(q);

    return q;
}

// create filter using Kaiser-Bessel windowed sinc method
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _as     : stop-band attenuation [dB], _as > 0
//  _mu     : fractional sample offset, -0.5 < _mu < 0.5
FIRFILT() FIRFILT(_create_kaiser)(unsigned int _n,
                                  float        _fc,
                                  float        _as,
                                  float        _mu)
{

    // compute temporary array for holding coefficients
    float hf[_n];
    if (liquid_firdes_kaiser(_n, _fc, _as, _mu, hf) != LIQUID_OK)
        return liquid_error_config("firfilt_%s_create_kaiser(), invalid config", EXTENSION_FULL);

    // copy coefficients to type-specific array
    TC h[_n];
    unsigned int i;
    for (i=0; i<_n; i++)
        h[i] = (TC) hf[i];

    // 
    return FIRFILT(_create)(h, _n);
}

// create from square-root Nyquist prototype
//  _type   : filter type (e.g. LIQUID_FIRFILT_RRC)
//  _k      : nominal samples/symbol, _k > 1
//  _m      : filter delay [symbols], _m > 0
//  _beta   : rolloff factor, 0 < beta <= 1
//  _mu     : fractional sample offset,-0.5 < _mu < 0.5
FIRFILT() FIRFILT(_create_rnyquist)(int          _type,
                                    unsigned int _k,
                                    unsigned int _m,
                                    float        _beta,
                                    float        _mu)
{
    // generate square-root Nyquist filter
    unsigned int h_len = 2*_k*_m + 1;
    float hf[h_len];
    if (liquid_firdes_prototype(_type,_k,_m,_beta,_mu,hf) != LIQUID_OK)
        return liquid_error_config("firfilt_%s_create_rnyquist(), invalid configuration", EXTENSION_FULL);

    // copy coefficients to type-specific array (e.g. float complex)
    unsigned int i;
    TC hc[h_len];
    for (i=0; i<h_len; i++)
        hc[i] = hf[i];

    // return filter object and return
    return FIRFILT(_create)(hc, h_len);
}

// Create object from Parks-McClellan algorithm prototype
//  _h_len  : filter length, _h_len > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _as     : stop-band attenuation [dB], _as > 0
FIRFILT() FIRFILT(_create_firdespm)(unsigned int _h_len,
                                    float        _fc,
                                    float        _as)
{
    // generate square-root Nyquist filter
    float hf[_h_len];
    if (firdespm_lowpass(_h_len,_fc,_as,0,hf) != LIQUID_OK)
        return liquid_error_config("firfilt_%s_create_firdespm(), invalid config", EXTENSION_FULL);

    // copy coefficients to type-specific array (e.g. float complex)
    // and scale by filter bandwidth to be consistent with other lowpass prototypes
    unsigned int i;
    TC hc[_h_len];
    for (i=0; i<_h_len; i++)
        hc[i] = hf[i] * 0.5f / _fc;

    // return filter object and return
    return FIRFILT(_create)(hc, _h_len);
}

// create rectangular filter prototype
FIRFILT() FIRFILT(_create_rect)(unsigned int _n)
{
    // validate input
    if (_n == 0 || _n > 1024)
        return liquid_error_config("firfilt_%s_create_rect(), filter length must be in [1,1024]", EXTENSION_FULL);

    // create float array coefficients
    float hf[_n];
    unsigned int i;
    for (i=0; i<_n; i++)
        hf[i] = 1.0f;

    // copy coefficients to type-specific array
    TC h[_n];
    for (i=0; i<_n; i++)
        h[i] = (TC) hf[i];

    // return filter object and return
    return FIRFILT(_create)(h, _n);
}

// create DC blocking filter
FIRFILT() FIRFILT(_create_dc_blocker)(unsigned int _m,
                                      float        _as)
{
    // create float array coefficients and design filter
    unsigned int h_len = 2*_m+1;
    float        hf[h_len];
    if (liquid_firdes_notch(_m, 0, _as, hf) != LIQUID_OK)
        return liquid_error_config("firfilt_%s_create_dc_blocker(), invalid config",EXTENSION_FULL);

    // copy coefficients to type-specific array
    TC h[h_len];
    unsigned int i;
    for (i=0; i<h_len; i++)
        h[i] = (TC) hf[i];

    // return filter object and return
    return FIRFILT(_create)(h, h_len);
}

// create notch filter
FIRFILT() FIRFILT(_create_notch)(unsigned int _m,
                                 float        _as,
                                 float        _f0)
{
    // create float array coefficients and design filter
    unsigned int i;
    unsigned int h_len = 2*_m+1;    // filter length
    float        hf[h_len];         // prototype filter with float coefficients
    TC           h [h_len];         // output filter with type-specific coefficients
#if TC_COMPLEX
    // design notch filter as DC blocker, then mix to appropriate frequency
    if (liquid_firdes_notch(_m, 0, _as, hf) != LIQUID_OK)
        return liquid_error_config("firfilt_%s_create_notch(), invalid config",EXTENSION_FULL);
    for (i=0; i<h_len; i++) {
        float phi = 2.0f * M_PI * _f0 * ((float)i - (float)_m);
        h[i] = cexpf(_Complex_I*phi) * (TC) hf[i];
    }
#else
    // design notch filter for real-valued coefficients directly
    if (liquid_firdes_notch(_m, _f0, _as, hf) != LIQUID_OK)
        return liquid_error_config("firfilt_%s_create_notch(), invalid config",EXTENSION_FULL);
    for (i=0; i<h_len; i++)
        h[i] = hf[i];
#endif

    // return filter object and return
    return FIRFILT(_create)(h, h_len);
}

// re-create firfilt object
//  _q      :   original firfilt object
//  _h      :   new coefficients [size: _n x 1]
//  _n      :   new filter length
FIRFILT() FIRFILT(_recreate)(FIRFILT() _q,
                             TC * _h,
                             unsigned int _n)
{
    unsigned int i;

    // reallocate memory array if filter length has changed
    if (_n != _q->h_len) {
        // reallocate memory
        _q->h_len = _n;
        _q->h = (TC*) realloc(_q->h, (_q->h_len)*sizeof(TC));

#if LIQUID_FIRFILT_USE_WINDOW
        // recreate window object, preserving internal state
        _q->w = WINDOW(_recreate)(_q->w, _q->h_len);
#else
        // free old array
        free(_q->w);

        // initialize array for buffering
        _q->w_len   = 1<<liquid_msb_index(_q->h_len);   // effectively 2^{floor(log2(len))+1}
        _q->w_mask  = _q->w_len - 1;
        // FIXME: valgrind is complaining about an uninitialized variable in the following malloc line
        _q->w       = (TI *) malloc((_q->w_len + _q->h_len + 1)*sizeof(TI));
        _q->w_index = 0;
#endif
    }

    // load filter in reverse order
    for (i=_n; i>0; i--)
        _q->h[i-1] = _h[_n-i];

    // re-create internal dot product object
    _q->dp = DOTPROD(_recreate)(_q->dp, _q->h, _q->h_len);

    return _q;
}

// copy object
FIRFILT() FIRFILT(_copy)(FIRFILT() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firfilt_%s_copy(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy base parameters
    FIRFILT() q_copy = (FIRFILT()) malloc(sizeof(struct FIRFILT(_s)));
    memmove(q_copy, q_orig, sizeof(struct FIRFILT(_s)));

    // copy filter coefficients
    q_copy->h = (TC *) liquid_malloc_copy(q_orig->h, q_orig->h_len, sizeof(TC));

#if LIQUID_FIRFILT_USE_WINDOW
    // copy window
    q_copy->w = WINDOW(_copy)(q_orig->w);
#else
    // initialize array for buffering
    q_copy->w = (TI *) liquid_malloc_copy(q_orig->w, q_orig->w_len, sizeof(TI));
#endif

    // copy dot product object and return
    q_copy->dp    = DOTPROD(_copy)(q_orig->dp);
    return q_copy;
}

// destroy firfilt object
int FIRFILT(_destroy)(FIRFILT() _q)
{
#if LIQUID_FIRFILT_USE_WINDOW
    WINDOW(_destroy)(_q->w);
#else
    free(_q->w);
#endif
    DOTPROD(_destroy)(_q->dp);
    free(_q->h);
    free(_q);
    return LIQUID_OK;
}

// reset internal state of filter object
int FIRFILT(_reset)(FIRFILT() _q)
{
#if LIQUID_FIRFILT_USE_WINDOW
    return WINDOW(_reset)(_q->w);
#else
    unsigned int i;
    for (i=0; i<_q->w_len; i++)
        _q->w[i] = 0.0;
    _q->w_index = 0;
    return LIQUID_OK;
#endif
}

// print filter object internals (taps, buffer)
int FIRFILT(_print)(FIRFILT() _q)
{
    printf("<liquid.firfilt_%s, n=%u", EXTENSION_FULL, _q->h_len);
    printf(", scale=");
    PRINTVAL_TC(_q->scale,%12.8f);
    printf(">\n");
    return LIQUID_OK;
}

// set output scaling for filter
int FIRFILT(_set_scale)(FIRFILT() _q,
                        TC        _scale)
{
    _q->scale = _scale;
    return LIQUID_OK;
}

// get output scaling for filter
int FIRFILT(_get_scale)(FIRFILT() _q,
                        TC *      _scale)
{
    *_scale = _q->scale;
    return LIQUID_OK;
}

// push sample into filter object's internal buffer
//  _q      :   filter object
//  _x      :   input sample
int FIRFILT(_push)(FIRFILT() _q,
                   TI        _x)
{
#if LIQUID_FIRFILT_USE_WINDOW
    return WINDOW(_push)(_q->w, _x);
#else
    // increment index
    _q->w_index++;

    // wrap around pointer
    _q->w_index &= _q->w_mask;

    // if pointer wraps around, copy excess memory
    if (_q->w_index == 0)
        memmove(_q->w, _q->w + _q->w_len, (_q->h_len)*sizeof(TI));

    // append value to end of buffer
    _q->w[_q->w_index + _q->h_len - 1] = _x;
    return LIQUID_OK;
#endif
}

// Write block of samples into filter object's internal buffer
//  _q      : filter object
//  _x      : buffer of input samples, [size: _n x 1]
//  _n      : number of input samples
int FIRFILT(_write)(FIRFILT()    _q,
                    TI *         _x,
                    unsigned int _n)
{
#if LIQUID_FIRFILT_USE_WINDOW
    return WINDOW(_write)(_q->w, _x, _n);
#else
    // TODO: be smarter about this
    unsigned int i;
    for (i=0; i<_n; i++) {
        int rc = FIRFILT(_push)(_q, _x[i]);
        if (rc != LIQUID_OK)
            return rc;
    }
    return LIQUID_OK;
#endif
}

// compute output sample (dot product between internal
// filter coefficients and internal buffer)
//  _q      :   filter object
//  _y      :   output sample pointer
int FIRFILT(_execute)(FIRFILT() _q,
                      TO *      _y)
{
    // read buffer (retrieve pointer to aligned memory array)
#if LIQUID_FIRFILT_USE_WINDOW
    TI *r;
    WINDOW(_read)(_q->w, &r);
#else
    TI *r = _q->w + _q->w_index;
#endif

    // execute dot product
    DOTPROD(_execute)(_q->dp, r, _y);

    // apply scaling factor
    *_y *= _q->scale;
    return LIQUID_OK;
}

// run on single sample
int FIRFILT(_execute_one)(FIRFILT() _q,
                          TI        _x,
                          TO *      _y)
{
    FIRFILT(_push)(_q, _x);
    return FIRFILT(_execute)(_q, _y);
}

// execute the filter on a block of input samples; the
// input and output buffers may be the same
//  _q      : filter object
//  _x      : pointer to input array [size: _n x 1]
//  _n      : number of input, output samples
//  _y      : pointer to output array [size: _n x 1]
int FIRFILT(_execute_block)(FIRFILT()    _q,
                            TI *         _x,
                            unsigned int _n,
                            TO *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        // push sample into filter
        FIRFILT(_push)(_q, _x[i]);

        // compute output sample
        FIRFILT(_execute)(_q, &_y[i]);
    }
    return LIQUID_OK;
}

// get filter length
unsigned int FIRFILT(_get_length)(FIRFILT() _q)
{
    return _q->h_len;
}

// Get pointer to coefficients array
const TC * FIRFILT(_get_coefficients)(FIRFILT() _q)
{
    return (const TC *) _q->h;
}

// Copy internal coefficients to external buffer
int FIRFILT(_copy_coefficients)(FIRFILT() _q,
                                TC *      _h)
{
    // internal coefficients are stored in normal order
    memmove(_h, _q->h, (_q->h_len)*sizeof(TC));
    return LIQUID_OK;
}

// compute complex frequency response
//  _q      :   filter object
//  _fc     :   frequency
//  _H      :   output frequency response
int FIRFILT(_freqresponse)(FIRFILT()       _q,
                           float           _fc,
                           float complex * _H)
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

// compute group delay in samples
//  _q      :   filter object
//  _fc     :   frequency
float FIRFILT(_groupdelay)(FIRFILT() _q,
                           float     _fc)
{
    // copy coefficients
    float h[_q->h_len];
    unsigned int i;
    for (i=0; i<_q->h_len; i++)
        h[i] = crealf(_q->h[i]);

    return fir_group_delay(h, _q->h_len, _fc);
}

