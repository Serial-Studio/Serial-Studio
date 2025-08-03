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
// firfarrow.c
//
// Finite impulse response Farrow filter
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define FIRFARROW_USE_DOTPROD 1

#define FIRFARROW_DEBUG 0

// defined:
//  FIRFARROW()     name-mangling macro
//  T               coefficients type
//  WINDOW()        window macro
//  DOTPROD()       dotprod macro
//  PRINTVAL()      print macro

int FIRFARROW(_genpoly)(FIRFARROW() _q);

struct FIRFARROW(_s) {
    TC * h;             // filter coefficients
    unsigned int h_len; // filter length
    float fc;           // filter cutoff
    float as;           // stop-band attenuation [dB]
    unsigned int Q;     // polynomial order

    float mu;           // fractional sample delay
    float * P;          // polynomail coefficients matrix [ h_len x Q+1 ]
    float gamma;        // inverse of DC response (normalization factor)

#if FIRFARROW_USE_DOTPROD
    WINDOW() w;
#else
    TI * v;
    unsigned int v_index;
#endif
};

// create firfarrow object
//  _h_len      : filter length
//  _p          : polynomial order
//  _fc         : filter cutoff frequency
//  _as         : stopband attenuation [dB]
FIRFARROW() FIRFARROW(_create)(unsigned int _h_len,
                               unsigned int _p,
                               float        _fc,
                               float        _as)
{
    // validate input
    if (_h_len < 2)
        return liquid_error_config("firfarrow_%s_create(), filter length must be > 2", EXTENSION_FULL);
    if (_p < 1)
        return liquid_error_config("firfarrow_%s_create(), polynomial order must be at least 1", EXTENSION_FULL);
    if (_fc < 0.0f || _fc > 0.5f)
        return liquid_error_config("firfarrow_%s_create(), filter cutoff must be in [0,0.5]", EXTENSION_FULL);
    if (_as < 0.0f)
        return liquid_error_config("firfarrow_%s_create(), filter stop-band attenuation must be greater than zero", EXTENSION_FULL);

    // create main object
    FIRFARROW() q = (FIRFARROW()) malloc(sizeof(struct FIRFARROW(_s)));

    // set internal properties
    q->h_len = _h_len;  // filter length
    q->Q     = _p;      // polynomial order
    q->as    = _as;     // filter stop-band attenuation
    q->fc    = _fc;     // filter cutoff frequency

    // allocate memory for filter coefficients
    q->h = (TC *) malloc((q->h_len)*sizeof(TC));

#if FIRFARROW_USE_DOTPROD
    q->w = WINDOW(_create)(q->h_len);
#else
    q->v = malloc((q->h_len)*sizeof(TI));
#endif

    // allocate memory for polynomial matrix [ h_len x Q+1 ]
    q->P = (float*) malloc((q->h_len)*(q->Q+1)*sizeof(float));

    // reset the filter object
    FIRFARROW(_reset)(q);

    // generate polynomials
    FIRFARROW(_genpoly)(q);

    // set nominal delay of 0
    FIRFARROW(_set_delay)(q,0.0f);

    // return main object
    return q;
}

// destroy firfarrow object, freeing all internal memory
int FIRFARROW(_destroy)(FIRFARROW() _q)
{
#if FIRFARROW_USE_DOTPROD
    WINDOW(_destroy)(_q->w);
#else
    free(_q->v);
#endif
    free(_q->h);    // free the filter coefficients array
    free(_q->P);    // free the polynomial matrix

    // free main object
    free(_q);
    return LIQUID_OK;
}

// print firfarrow object's internal properties
int FIRFARROW(_print)(FIRFARROW() _q)
{
    printf("<firfarrow, len=%u, order=%u>\n", _q->h_len, _q->Q);
#if 0
    printf("polynomial coefficients:\n");

    // print coefficients
    unsigned int i, j, n=0;
    for (i=0; i<_q->h_len; i++) {
        printf("  %3u : ", i);
        for (j=0; j<_q->Q+1; j++)
            printf("%12.4e ", _q->P[n++]);
        printf("\n");
    }

    printf("filter coefficients (mu=%8.4f):\n", _q->mu);
    n = _q->h_len;
    for (i=0; i<n; i++) {
        printf("  h(%3u) = ", i+1);
        PRINTVAL_TC(_q->h[n-i-1],%12.8f);
        printf(";\n");
    }
#endif
    return LIQUID_OK;
}

// reset firfarrow object's internal state
int FIRFARROW(_reset)(FIRFARROW() _q)
{
#if FIRFARROW_USE_DOTPROD
    return WINDOW(_reset)(_q->w);
#else
    unsigned int i;
    for (i=0; i<_q->h_len; i++)
        _q->v[i] = 0;
    _q->v_index = 0;
    return LIQUID_OK;
#endif
}

// push sample into firfarrow object
//  _q      :   firfarrow object
//  _x      :   input sample
int FIRFARROW(_push)(FIRFARROW() _q,
                     TI _x)
{
#if FIRFARROW_USE_DOTPROD
    return WINDOW(_push)(_q->w, _x);
#else
    _q->v[ _q->v_index ] = _x;
    (_q->v_index)++;
    _q->v_index = (_q->v_index) % (_q->h_len);
    return LIQUID_OK;
#endif
}

// set fractional delay of firfarrow object
//  _q      : firfarrow object
//  _mu     : fractional sample delay
int FIRFARROW(_set_delay)(FIRFARROW() _q,
                          float       _mu)
{
    // validate input
    if (_mu < -1.0f || _mu > 1.0f) {
        return liquid_error(LIQUID_EIVAL,"firfarrow_%s_create(), delay must be in [-1,1]\n", EXTENSION_FULL);
    }

    unsigned int i, n=0;
    for (i=0; i<_q->h_len; i++) {
        // compute filter tap from polynomial using negative
        // value for _mu
        _q->h[i] = POLY(_val)(_q->P+n, _q->Q, -_mu);

        // normalize filter by inverse of DC response
        _q->h[i] *= _q->gamma;

        n += _q->Q+1;

        //printf("  h[%3u] = %12.8f\n", i, _q->h[i]);
    }
    return LIQUID_OK;
}

// execute firfarrow internal dot product
//  _q      : firfarrow object
//  _y      : output sample pointer
int FIRFARROW(_execute)(FIRFARROW() _q,
                        TO *        _y)
{
#if FIRFARROW_USE_DOTPROD
    TI *r;
    WINDOW(_read)(_q->w, &r);
    DOTPROD(_run4)(_q->h, r, _q->h_len, _y);
#else
    TO y = 0;
    unsigned int i;
    for (i=0; i<_q->h_len; i++)
        y += _q->v[ (i+_q->v_index)%(_q->h_len) ] * _q->h[i];
    *_y = y;
#endif
    return LIQUID_OK;
}

// compute firfarrow filter on block of samples; the input
// and output arrays may have the same pointer
//  _q      : firfarrow object
//  _x      : input array [size: _n x 1]
//  _n      : input, output array size
//  _y      : output array [size: _n x 1]
int FIRFARROW(_execute_block)(FIRFARROW()  _q,
                              TI *         _x,
                              unsigned int _n,
                              TO *         _y)
{
    unsigned int i;

    for (i=0; i<_n; i++) {
        // push input sample
        FIRFARROW(_push)(_q, _x[i]);

        // compute output
        FIRFARROW(_execute)(_q, &_y[i]);
    }
    return LIQUID_OK;
}

// get length of firfarrow object (number of filter taps)
unsigned int FIRFARROW(_get_length)(FIRFARROW() _q)
{
    return _q->h_len;
}

// get coefficients of firfarrow object
//  _q      : firfarrow object
//  _h      : output coefficients pointer
int FIRFARROW(_get_coefficients)(FIRFARROW() _q,
                                 TC *        _h)
{
    memmove(_h, _q->h, (_q->h_len)*sizeof(TC));
    return LIQUID_OK;
}

// compute complex frequency response
//  _q      : filter object
//  _fc     : frequency
//  _H      : output frequency response
int FIRFARROW(_freqresponse)(FIRFARROW() _q,
                             float _fc,
                             float complex * _H)
{
    unsigned int i;
    float complex H = 0.0f;

    for (i=0; i<_q->h_len; i++)
        H += _q->h[i] * cexpf(_Complex_I*2*M_PI*_fc*i);

    // set return value
    *_H = H;
    return LIQUID_OK;
}

// compute group delay [samples]
//  _q      :   filter object
//  _fc     :   frequency
float FIRFARROW(_groupdelay)(FIRFARROW() _q,
                             float _fc)
{
    // copy coefficients to be in correct order
    float h[_q->h_len];
    unsigned int i;
    unsigned int n = _q->h_len;
    for (i=0; i<n; i++)
        h[i] = crealf(_q->h[i]);

    return fir_group_delay(h, n, _fc);
}



// 
// internal
//

// generate polynomials to represent filter coefficients
int FIRFARROW(_genpoly)(FIRFARROW() _q)
{
    // TODO : shy away from 'float' and use 'TC' types
    unsigned int i, j, n=0;
    float x, mu, h0, h1;
    float mu_vect[_q->Q+1];
    float hp_vect[_q->Q+1];
    float p[_q->Q+1];
    float beta = kaiser_beta_As(_q->as);
    for (i=0; i<_q->h_len; i++) {
#if FIRFARROW_DEBUG
        printf("i : %3u / %3u\n", i, _q->h_len);
#endif
        x = (float)(i) - (float)(_q->h_len-1)/2.0f;
        for (j=0; j<=_q->Q; j++) {
            mu = ((float)j - (float)_q->Q)/((float)_q->Q) + 0.5f;

            h0 = sincf(2.0f*(_q->fc)*(x + mu));
            h1 = liquid_kaiser(i,_q->h_len,beta);
#if FIRFARROW_DEBUG
            printf("  %3u : x=%12.8f, mu=%12.8f, h0=%12.8f, h1=%12.8f, hp=%12.8f\n",
                    j, x, mu, h0, h1, h0*h1);
#endif

            mu_vect[j] = mu;
            hp_vect[j] = h0*h1;
        }
        POLY(_fit)(mu_vect,hp_vect,_q->Q+1,p,_q->Q+1);
#if FIRFARROW_DEBUG
        printf("  polynomial : ");
        for (j=0; j<=_q->Q; j++)
            printf("%8.4f,", p[j]);
        printf("\n");
#endif

        // copy coefficients to internal matrix
        memmove(_q->P+n, p, (_q->Q+1)*sizeof(float));
        n += _q->Q+1;
    }

#if FIRFARROW_DEBUG
    // print coefficients
    n=0;
    for (i=0; i<_q->h_len; i++) {
        printf("%3u : ", i);
        for (j=0; j<_q->Q+1; j++)
            printf("%12.4e ", _q->P[n++]);
        printf("\n");
    }
#endif

    // normalize DC gain
    _q->gamma = 1.0f;                // initialize gamma to 1
    FIRFARROW(_set_delay)(_q,0.0f); // compute filter taps with zero delay
    _q->gamma = 0.0f;                // clear gamma
    for (i=0; i<_q->h_len; i++)      // compute DC response
        _q->gamma += _q->h[i];
    _q->gamma = 1.0f / (_q->gamma);   // invert result

    return LIQUID_OK;
}

