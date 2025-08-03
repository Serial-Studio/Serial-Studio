/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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

// Recursive least-squares (RLS) equalizer

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#define DEBUG

struct EQRLS(_s) {
    unsigned int p;     // filter order
    float lambda;       // RLS forgetting factor
    float delta;        // RLS initialization factor

    // internal matrices
    T * h0;             // initial coefficients
    T * w0, * w1;       // weights [px1]
    T * P0, * P1;       // recursion matrix [pxp]
    T * g;              // gain vector [px1]

    // temporary matrices
    T * xP0;            // [1xp]
    T zeta;             // constant

    T * gxl;            // [pxp]
    T * gxlP0;          // [pxp]

    unsigned int n;     // input counter
    WINDOW() buffer;    // input buffer
};


// create recursive least-squares (RLS) equalizer object
//  _h      :   initial coefficients [size: _p x 1], default if NULL
//  _p      :   equalizer length (number of taps)
EQRLS() EQRLS(_create)(T *          _h,
                       unsigned int _p)
{
    if (_p==0)
        return liquid_error_config("eqrls_%s_create(), equalier length must be greater than 0",EXTENSION_FULL);

    EQRLS() q = (EQRLS()) malloc(sizeof(struct EQRLS(_s)));

    // set filter order, other parameters
    q->p      = _p;     // filter order
    q->lambda = 0.99f;  // learning rate
    q->delta  = 0.1f;   // initialization factor

    // allocate memory for matrices
    q->h0 = (T*) malloc((q->p)*sizeof(T));
    q->w0 = (T*) malloc((q->p)*sizeof(T));
    q->w1 = (T*) malloc((q->p)*sizeof(T));
    q->P0 = (T*) malloc((q->p)*(q->p)*sizeof(T));
    q->P1 = (T*) malloc((q->p)*(q->p)*sizeof(T));
    q->g  = (T*) malloc((q->p)*sizeof(T));

    q->xP0 =   (T*) malloc((q->p)*sizeof(T));
    q->gxl =   (T*) malloc((q->p)*(q->p)*sizeof(T));
    q->gxlP0 = (T*) malloc((q->p)*(q->p)*sizeof(T));

    q->buffer = WINDOW(_create)(q->p);

    // copy coefficients (if not NULL)
    if (_h == NULL) {
        // initial coefficients with delta at first index
        unsigned int i;
        for (i=0; i<q->p; i++)
            q->h0[i] = (i==q->p-1) ? 1.0 : 0.0;
    } else {
        // copy user-defined initial coefficients
        memmove(q->h0, _h, (q->p)*sizeof(T));
    }

    // reset equalizer
    EQRLS(_reset)(q);

    // return object
    return q;
}


// re-create recursive least-squares (RLS) equalizer object
//  _q  : old equalizer object
//  _h  : filter coefficients (NULL for {1,0,0...})
//  _p  : equalizer length (number of taps)
EQRLS() EQRLS(_recreate)(EQRLS()      _q,
                         T *          _h,
                         unsigned int _p)
{
    if (_q->p == _p) {
        // length hasn't changed; copy default coefficients
        // and return object
        unsigned int i;
        for (i=0; i<_q->p; i++)
            _q->h0[i] = _h[i];
        return _q;
    }

    // completely destroy old equalizer object
    EQRLS(_destroy)(_q);

    // create new one and return
    return EQRLS(_create)(_h,_p);
}

// copy object
EQRLS() EQRLS(_copy)(EQRLS() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firfilt_%s_copy(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy base parameters
    EQRLS() q_copy = (EQRLS()) malloc(sizeof(struct EQRLS(_s)));
    memmove(q_copy, q_orig, sizeof(struct EQRLS(_s)));

    // allocate and copy memory from original
    unsigned int p = q_copy->p; // filter order (for convenience)
    q_copy->h0    = (T*) liquid_malloc_copy(q_orig->h0,   p,   sizeof(T));
    q_copy->w0    = (T*) liquid_malloc_copy(q_orig->w0,   p,   sizeof(T));
    q_copy->w1    = (T*) liquid_malloc_copy(q_orig->w1,   p,   sizeof(T));
    q_copy->P0    = (T*) liquid_malloc_copy(q_orig->P0,   p*p, sizeof(T));
    q_copy->P1    = (T*) liquid_malloc_copy(q_orig->P1,   p*p, sizeof(T));
    q_copy->g     = (T*) liquid_malloc_copy(q_orig->g,    p,   sizeof(T));
    q_copy->xP0   = (T*) liquid_malloc_copy(q_orig->xP0,  p,   sizeof(T));
    q_copy->gxl   = (T*) liquid_malloc_copy(q_orig->gxl,  p*p, sizeof(T));
    q_copy->gxlP0 = (T*) liquid_malloc_copy(q_orig->gxlP0,p*p, sizeof(T));

    // copy window and buffer objects
    q_copy->buffer = WINDOW(_copy)(q_orig->buffer);

    return q_copy;
}

// destroy eqrls object
int EQRLS(_destroy)(EQRLS() _q)
{
    // free vectors and matrices
    free(_q->h0);
    free(_q->w0);
    free(_q->w1);
    free(_q->P0);
    free(_q->P1);
    free(_q->g);

    free(_q->xP0);
    free(_q->gxl);
    free(_q->gxlP0);

    // destroy window buffer
    WINDOW(_destroy)(_q->buffer);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print eqrls object internals
int EQRLS(_print)(EQRLS() _q)
{
    printf("<liquid.eqrls_%s, order=%u, lambda=%g, delta=%g>\n",
        EXTENSION_FULL, _q->p, _q->lambda, _q->delta);
    return LIQUID_OK;
}

// reset equalizer
int EQRLS(_reset)(EQRLS() _q)
{
    // reset input counter
    _q->n = 0;

    unsigned int i, j;
    // initialize...
    for (i=0; i<_q->p; i++) {
        for (j=0; j<_q->p; j++) {
            if (i==j)   _q->P0[(_q->p)*i + j] = 1 / (_q->delta);
            else        _q->P0[(_q->p)*i + j] = 0;
        }
    }

    // copy default coefficients
    memmove(_q->w0, _q->h0, (_q->p)*sizeof(T));

    // clear window object
    WINDOW(_reset)(_q->buffer);
    return LIQUID_OK;
}

// get learning rate of equalizer
float EQRLS(_get_bw)(EQRLS() _q)
{
    return _q->lambda;
}

// set learning rate of equalizer
//  _q     :   equalizer object
//  _lambda :   RLS learning rate (should be close to 1.0), 0 < _lambda < 1
int EQRLS(_set_bw)(EQRLS() _q,
                   float   _lambda)
{
    if (_lambda < 0.0f || _lambda > 1.0f)
        return liquid_error(LIQUID_EICONFIG,"eqrls_%s_set_bw(), learning rate must be in (0,1)", EXTENSION_FULL);

    // set internal value
    _q->lambda = _lambda;
    return LIQUID_OK;
}

// push sample into equalizer internal buffer
//  _q  :   equalizer object
//  _x  :   received sample
int EQRLS(_push)(EQRLS() _q,
                 T       _x)
{
    // push value into buffer
    WINDOW(_push)(_q->buffer, _x);
    return LIQUID_OK;
}

// execute internal dot product
//  _q      :   equalizer object
//  _y      :   output sample
int EQRLS(_execute)(EQRLS() _q,
                    T *     _y)
{
    // compute vector dot product
    T * r;      // read buffer
    WINDOW(_read)(_q->buffer, &r);
    DOTPROD(_run)(_q->w0, r, _q->p, _y);
    return LIQUID_OK;
}

// execute cycle of equalizer, filtering output
//  _q      :   equalizer object
//  _x      :   received sample
//  _d      :   desired output
//  _d_hat  :   filtered output
int EQRLS(_step)(EQRLS() _q,
                 T       _d,
                 T       _d_hat)
{
    unsigned int i,r,c;
    unsigned int p=_q->p;

    // compute error (a priori)
    T alpha = _d - _d_hat;

    // read buffer
    T * x;
    WINDOW(_read)(_q->buffer, &x);

    // compute gain vector
    for (c=0; c<p; c++) {
        _q->xP0[c] = 0;
        for (r=0; r<p; r++) {
            _q->xP0[c] += x[r] * matrix_access(_q->P0,p,p,r,c);
        }
    }

#ifdef DEBUG
    printf("x: ");
    for (i=0; i<p; i++)
        PRINTVAL(x[i]);
    printf("\n");

    DEBUG_PRINTF_CFLOAT(stdout,"    d",0,_d);
    DEBUG_PRINTF_CFLOAT(stdout,"_d_hat",0,_d_hat);
    DEBUG_PRINTF_CFLOAT(stdout,"error",0,alpha);

    printf("xP0: ");
    for (c=0; c<p; c++)
        PRINTVAL(_q->xP0[c]);
    printf("\n");
#endif
    // zeta = lambda + [x.']*[P0]*[conj(x)]
    _q->zeta = 0;
    for (c=0; c<p; c++) {
        T sum = _q->xP0[c] * conj(x[c]);
        _q->zeta += sum;
    }
    _q->zeta += _q->lambda;
#ifdef DEBUG
    printf("zeta : ");
    PRINTVAL(_q->zeta);
    printf("\n");
#endif
    for (r=0; r<p; r++) {
        _q->g[r] = 0;
        for (c=0; c<p; c++) {
            T sum = matrix_access(_q->P0,p,p,r,c) * conj(x[c]);
            _q->g[r] += sum;
        }
        _q->g[r] /= _q->zeta;
    }
#ifdef DEBUG
    printf("g: ");
    for (i=0; i<p; i++)
        PRINTVAL(_q->g[i]);
        //printf("%6.3f ", _q->g[i]);
    printf("\n");
#endif

    // update recursion matrix
    for (r=0; r<p; r++) {
        for (c=0; c<p; c++) {
            // gxl = [g] * [x.'] / lambda
            matrix_access(_q->gxl,p,p,r,c) = _q->g[r] * x[c] / _q->lambda;
        }
    }
    // multiply two [pxp] matrices: gxlP0 = gxl * P0
    MATRIX(_mul)(_q->gxl,  p,p,
                 _q->P0,   p,p,
                 _q->gxlP0,p,p);

    for (i=0; i<p*p; i++)
        _q->P1[i] = _q->P0[i] / _q->lambda - _q->gxlP0[i];

    // update weighting vector
    for (i=0; i<p; i++)
        _q->w1[i] = _q->w0[i] + alpha*(_q->g[i]);

#ifdef DEBUG
    printf("w0: \n");
    for (i=0; i<p; i++) {
        PRINTVAL(_q->w0[i]);
        printf("\n");
    }
    printf("w1: \n");
    for (i=0; i<p; i++) {
        PRINTVAL(_q->w1[i]);
        printf("\n");
    EQRLS(_print)(_q);
    }
#endif

    // copy old values
    memmove(_q->w0, _q->w1,   p*sizeof(T));
    memmove(_q->P0, _q->P1, p*p*sizeof(T));
    return LIQUID_OK;
}

// retrieve internal filter coefficients
//  _q      :   equalizer object
//  _w      :   weights [size: _p x 1]
int EQRLS(_get_weights)(EQRLS() _q,
                        T *     _w)
{
    // copy output weight vector, reversing order
    unsigned int i;
    for (i=0; i<_q->p; i++)
        _w[i] = _q->w1[_q->p-i-1];
    return LIQUID_OK;
}

// train equalizer object
//  _q      :   equalizer object
//  _w      :   initial weights / output weights
//  _x      :   received sample vector
//  _d      :   desired output vector
//  _n      :   vector length
int EQRLS(_train)(EQRLS()      _q,
                  T *          _w,
                  T *          _x,
                  T *          _d,
                  unsigned int _n)
{
    unsigned int i;
    if (_n < _q->p)
        return liquid_error(LIQUID_EIVAL,"eqrls_%s_train(), traning sequence less than filter order",EXTENSION_FULL);

    // reset equalizer state
    EQRLS(_reset)(_q);

    // copy initial weights into buffer
    for (i=0; i<_q->p; i++)
        _q->w0[i] = _w[_q->p - i - 1];

    T d_hat;
    for (i=0; i<_n; i++) {
        // push sample into internal buffer
        EQRLS(_push)(_q, _x[i]);

        // execute vector dot product
        EQRLS(_execute)(_q, &d_hat);

        // step through training cycle
        EQRLS(_step)(_q, _d[i], d_hat);
    }

    // copy output weight vector
    return EQRLS(_get_weights)(_q, _w);
}
