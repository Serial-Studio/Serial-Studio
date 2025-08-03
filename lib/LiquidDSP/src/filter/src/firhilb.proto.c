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

// finite impulse response (FIR) Hilbert transform

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct FIRHILB(_s) {
    T * h;                  // filter coefficients
    T complex * hc;         // filter coefficients (complex)
    unsigned int h_len;     // length of filter
    float as;               // filter stop-band attenuation [dB]

    unsigned int m;         // filter semi-length, h_len = 4*m+1

    // quadrature filter component
    T * hq;                 // quadrature filter coefficients
    unsigned int hq_len;    // quadrature filter length (2*m)

    // input buffers
    WINDOW() w0;            // input buffer (even samples)
    WINDOW() w1;            // input buffer (odd samples)

    // additional buffers needed exclusively for real-to-complex filter operations
    WINDOW() w2;
    WINDOW() w3;

    // vector dot product
    DOTPROD() dpq;

    // regular real-to-complex/complex-to-real operation
    unsigned int toggle;
};

// create firhilb object
//  _m      :   filter semi-length (delay: 2*m+1)
//  _as     :   stop-band attenuation [dB]
FIRHILB() FIRHILB(_create)(unsigned int _m,
                           float        _as)
{
    // validate firhilb inputs
    if (_m < 2)
        return liquid_error_config("firhilb_create(), filter semi-length (m) must be at least 2");

    // allocate memory for main object
    FIRHILB() q = (FIRHILB()) malloc(sizeof(struct FIRHILB(_s)));
    q->m  = _m;         // filter semi-length
    q->as = fabsf(_as); // stop-band attenuation

    // set filter length and allocate memory for coefficients
    q->h_len = 4*(q->m) + 1;
    q->h     = (T *)         malloc((q->h_len)*sizeof(T));
    q->hc    = (T complex *) malloc((q->h_len)*sizeof(T complex));

    // allocate memory for quadrature filter component
    q->hq_len = 2*(q->m);
    q->hq     = (T *) malloc((q->hq_len)*sizeof(T));

    // compute filter coefficients for half-band filter
    liquid_firdes_kaiser(q->h_len, 0.25f, q->as, 0.0f, q->h);

    // alternate sign of non-zero elements
    unsigned int i;
    for (i=0; i<q->h_len; i++) {
        float t = (float)i - (float)(q->h_len-1)/2.0f;
        q->hc[i] = q->h[i] * cexpf(_Complex_I*0.5f*M_PI*t);
        q->h[i]  = cimagf(q->hc[i]);
    }

    // resample, reverse direction
    unsigned int j=0;
    for (i=1; i<q->h_len; i+=2)
        q->hq[j++] = q->h[q->h_len - i - 1];

    // create windows for upper and lower polyphase filter branches
    q->w0 = WINDOW(_create)(2*(q->m));
    q->w1 = WINDOW(_create)(2*(q->m));
    q->w2 = WINDOW(_create)(2*(q->m));
    q->w3 = WINDOW(_create)(2*(q->m));

    // create internal dot product object
    q->dpq = DOTPROD(_create)(q->hq, q->hq_len);

    // reset internal state and return object
    FIRHILB(_reset)(q);
    return q;
}

FIRHILB() FIRHILB(_copy)(FIRHILB() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firhilb%s_copy(), object cannot be NULL", EXTENSION_SHORT);

    // create filter object and copy base parameters
    FIRHILB() q_copy = (FIRHILB()) malloc(sizeof(struct FIRHILB(_s)));
    memmove(q_copy, q_orig, sizeof(struct FIRHILB(_s)));

    // copy filter coefficients
    q_copy->h  = (T *)        liquid_malloc_copy(q_orig->h,  q_orig->h_len,  sizeof(T));
    q_copy->hc = (T complex*) liquid_malloc_copy(q_orig->hc, q_orig->h_len,  sizeof(T complex));
    q_copy->hq = (T *)        liquid_malloc_copy(q_orig->hq, q_orig->hq_len, sizeof(T));

    // copy objects and return
    q_copy->w0  = WINDOW (_copy)(q_orig->w0 );
    q_copy->w1  = WINDOW (_copy)(q_orig->w1 );
    q_copy->w2  = WINDOW (_copy)(q_orig->w2 );
    q_copy->w3  = WINDOW (_copy)(q_orig->w3 );
    q_copy->dpq = DOTPROD(_copy)(q_orig->dpq);
    return q_copy;
}

// destroy firhilb object
int FIRHILB(_destroy)(FIRHILB() _q)
{
    // destroy window buffers
    WINDOW(_destroy)(_q->w0);
    WINDOW(_destroy)(_q->w1);
    WINDOW(_destroy)(_q->w2);
    WINDOW(_destroy)(_q->w3);
    
    // destroy internal dot product object
    DOTPROD(_destroy)(_q->dpq);

    // free coefficients arrays
    free(_q->h);
    free(_q->hc);
    free(_q->hq);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print firhilb object internals
int FIRHILB(_print)(FIRHILB() _q)
{
    printf("<liquid.firhilb%s, len=%u>\n", EXTENSION_SHORT, _q->h_len);
    return LIQUID_OK;
}

// reset firhilb object internal state
int FIRHILB(_reset)(FIRHILB() _q)
{
    // clear window buffers
    WINDOW(_reset)(_q->w0);
    WINDOW(_reset)(_q->w1);
    WINDOW(_reset)(_q->w2);
    WINDOW(_reset)(_q->w3);

    // reset toggle flag
    _q->toggle = 0;
    return LIQUID_OK;
}

// execute Hilbert transform (real to complex)
//  _q      :   firhilb object
//  _x      :   real-valued input sample
//  _y      :   complex-valued output sample
int FIRHILB(_r2c_execute)(FIRHILB()   _q,
                          T           _x,
                          T complex * _y)
{
    T * r;  // buffer read pointer
    T yi;   // in-phase component
    T yq;   // quadrature component

    if ( _q->toggle == 0 ) {
        // push sample into upper branch
        WINDOW(_push)(_q->w0, _x);

        // upper branch (delay)
        WINDOW(_index)(_q->w0, _q->m-1, &yi);

        // lower branch (filter)
        WINDOW(_read)(_q->w1, &r);
        
        // execute dotprod
        DOTPROD(_execute)(_q->dpq, r, &yq);
    } else {
        // push sample into lower branch
        WINDOW(_push)(_q->w1, _x);

        // upper branch (delay)
        WINDOW(_index)(_q->w1, _q->m-1, &yi);

        // lower branch (filter)
        WINDOW(_read)(_q->w0, &r);

        // execute dotprod
        DOTPROD(_execute)(_q->dpq, r, &yq);
    }

    // toggle flag
    _q->toggle = 1 - _q->toggle;

    // set return value
    *_y = yi + _Complex_I * yq;
    return LIQUID_OK;
}

// execute Hilbert transform (complex to real)
//  _q      :   firhilb object
//  _x      :   complex-valued input sample
//  _y0     :   real-valued output sample, lower side-band retained
//  _y1     :   real-valued output sample, upper side-band retained
int FIRHILB(_c2r_execute)(FIRHILB() _q,
                          T complex _x,
                          T *       _y0,
                          T *       _y1)
{
    T * r;  // buffer read pointer
    T yi;   //
    T yq;   //

    if (_q->toggle == 0) {
        // push samples into appropriate buffers
        WINDOW(_push)(_q->w0, crealf(_x));
        WINDOW(_push)(_q->w1, cimagf(_x));

        // compute delay branch
        WINDOW(_index)(_q->w0, _q->m-1, &yi);

        // compute filter branch
        WINDOW(_read)(_q->w3, &r);
        DOTPROD(_execute)(_q->dpq, r, &yq);
    } else {
        // push samples into appropriate buffers
        WINDOW(_push)(_q->w2, crealf(_x));
        WINDOW(_push)(_q->w3, cimagf(_x));

        // compute delay branch
        WINDOW(_index)(_q->w2, _q->m-1, &yi);

        // compute filter branch
        WINDOW(_read)(_q->w1, &r);
        DOTPROD(_execute)(_q->dpq, r, &yq);
    }

    // adjust state
    _q->toggle = 1 - _q->toggle;

    // set return value
    *_y0 = yi + yq; // lower side-band
    *_y1 = yi - yq; // upper side-band
    return LIQUID_OK;
}

// execute Hilbert transform decimator (real to complex)
//  _q      :   firhilb object
//  _x      :   real-valued input array [size: 2 x 1]
//  _y      :   complex-valued output sample
int FIRHILB(_decim_execute)(FIRHILB()   _q,
                            T *         _x,
                            T complex * _y)
{
    T * r;  // buffer read pointer
    T yi;   // in-phase component
    T yq;   // quadrature component

    // compute quadrature component (filter branch)
    WINDOW(_push)(_q->w1, _x[0]);
    WINDOW(_read)(_q->w1, &r);
    DOTPROD(_execute)(_q->dpq, r, &yq);

    // delay branch
    WINDOW(_push)(_q->w0, _x[1]);
    WINDOW(_index)(_q->w0, _q->m-1, &yi);

    // set return value
    T complex v = yi + _Complex_I * yq;
    *_y = _q->toggle ? -v : v;

    // toggle flag
    _q->toggle = 1 - _q->toggle;
    return LIQUID_OK;
}

// execute Hilbert transform decimator (real to complex) on
// a block of samples
//  _q      :   Hilbert transform object
//  _x      :   real-valued input array [size: 2*_n x 1]
//  _n      :   number of *output* samples
//  _y      :   complex-valued output array [size: _n x 1]
int FIRHILB(_decim_execute_block)(FIRHILB()    _q,
                                  T *          _x,
                                  unsigned int _n,
                                  T complex *  _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        FIRHILB(_decim_execute)(_q, &_x[2*i], &_y[i]);
    }

    return LIQUID_OK;
}

// execute Hilbert transform interpolator (complex to real)
//  _q      :   firhilb object
//  _y      :   complex-valued input sample
//  _x      :   real-valued output array [size: 2 x 1]
int FIRHILB(_interp_execute)(FIRHILB() _q,
                             T complex _x,
                             T *       _y)
{
    T * r;  // buffer read pointer

    // TODO macro for crealf, cimagf?
    T vi = _q->toggle ? -crealf(_x) : crealf(_x);
    T vq = _q->toggle ? -cimagf(_x) : cimagf(_x);

    // compute delay branch
    WINDOW(_push)(_q->w0, vq);
    WINDOW(_index)(_q->w0, _q->m-1, &_y[0]);

    // compute second branch (filter)
    WINDOW(_push)(_q->w1, vi);
    WINDOW(_read)(_q->w1, &r);
    DOTPROD(_execute)(_q->dpq, r, &_y[1]);

    // toggle flag
    _q->toggle = 1 - _q->toggle;
    return LIQUID_OK;
}

// execute Hilbert transform interpolator (complex to real)
// on a block of samples
//  _q      :   Hilbert transform object
//  _x      :   complex-valued input array [size: _n x 1]
//  _n      :   number of *input* samples
//  _y      :   real-valued output array [size: 2*_n x 1]
int FIRHILB(_interp_execute_block)(FIRHILB()    _q,
                                   T complex *  _x,
                                   unsigned int _n,
                                   T *          _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        FIRHILB(_interp_execute)(_q, _x[i], &_y[2*i]);
    }

    return LIQUID_OK;
}
