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

//
// Binary pre-demod synchronizer
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "liquid.internal.h"

struct BPRESYNC(_s) {
    unsigned int n;     // sequence length
    unsigned int m;     // number of binary synchronizers
    
    bsequence rx_i;     // received pattern (in-phase)
    bsequence rx_q;     // received pattern (quadrature)
    
    float * dphi;       // array of frequency offsets [size: m x 1]
    bsequence * sync_i; // synchronization pattern (in-phase)
    bsequence * sync_q; // synchronization pattern (quadrature)

    float * rxy;        // output correlation [size: m x 1]

    float n_inv;        // 1/n (pre-computed for speed)
};

// correlate input sequence with particular sequence index
//  _q      : pre-demod synchronizer object
//  _id     : sequence index
//  _rxy0   : positive frequency correlation output (non-conjugated)
//  _rxy1   : negative frequency correlation output (conjugated)
int BPRESYNC(_correlatex)(BPRESYNC()      _q,
                          unsigned int    _id,
                          float complex * _rxy0,
                          float complex * _rxy1);

// create binary pre-demod synchronizer
//  _v          :   baseband sequence
//  _n          :   baseband sequence length
//  _dphi_max   :   maximum absolute frequency deviation
//  _m          :   number of correlators
BPRESYNC() BPRESYNC(_create)(TC *         _v,
                             unsigned int _n,
                             float        _dphi_max,
                             unsigned int _m)
{
    // validate input
    if (_n < 1)
        return liquid_error_config("bpresync_%s_create(), invalid input length", EXTENSION_FULL);
    if (_m == 0)
        return liquid_error_config("bpresync_%s_create(), number of correlators must be at least 1", EXTENSION_FULL);

    // allocate main object memory and initialize
    BPRESYNC() _q = (BPRESYNC()) malloc(sizeof(struct BPRESYNC(_s)));
    _q->n = _n;
    _q->m = _m;

    _q->n_inv = 1.0f / (float)(_q->n);

    unsigned int i;

    // create internal receive buffers
    _q->rx_i = bsequence_create(_q->n);
    _q->rx_q = bsequence_create(_q->n);

    // create internal array of frequency offsets
    _q->dphi = (float*) malloc( _q->m*sizeof(float) );

    // create internal synchronizers
    _q->sync_i = (bsequence*) malloc( _q->m*sizeof(bsequence) );
    _q->sync_q = (bsequence*) malloc( _q->m*sizeof(bsequence) );

    for (i=0; i<_q->m; i++) {

        _q->sync_i[i] = bsequence_create(_q->n);
        _q->sync_q[i] = bsequence_create(_q->n);

        // generate signal with frequency offset
        _q->dphi[i] = (float)i / (float)(_q->m-1)*_dphi_max;
        unsigned int k;
        for (k=0; k<_q->n; k++) {
            TC v_prime = _v[k] * cexpf(-_Complex_I*k*_q->dphi[i]);
            bsequence_push(_q->sync_i[i], crealf(v_prime)>0);
            bsequence_push(_q->sync_q[i], cimagf(v_prime)>0);
        }
    }

    // allocate memory for cross-correlation
    _q->rxy = (float*) malloc( _q->m*sizeof(float) );

    // reset object
    BPRESYNC(_reset)(_q);

    return _q;
}

int BPRESYNC(_destroy)(BPRESYNC() _q)
{
    unsigned int i;

    // free received symbol buffers
    bsequence_destroy(_q->rx_i);
    bsequence_destroy(_q->rx_q);

    // free internal syncrhonizer objects
    for (i=0; i<_q->m; i++) {
        bsequence_destroy(_q->sync_i[i]);
        bsequence_destroy(_q->sync_q[i]);
    }
    free(_q->sync_i);
    free(_q->sync_q);

    // free internal frequency offset array
    free(_q->dphi);

    // free internal cross-correlation array
    free(_q->rxy);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

int BPRESYNC(_print)(BPRESYNC() _q)
{
    printf("<liquid.bpresync_%s, n=%u>\n", EXTENSION_FULL, _q->n);
    return LIQUID_OK;
}

int BPRESYNC(_reset)(BPRESYNC() _q)
{
    unsigned int i;
    for (i=0; i<_q->n; i++) {
        bsequence_push(_q->rx_i, (i+0) % 2);
        bsequence_push(_q->rx_q, (i+1) % 2);
    }
    return LIQUID_OK;
}

/* push input sample into pre-demod synchronizer            */
/*  _q          :   pre-demod synchronizer object           */
/*  _x          :   input sample                            */
int BPRESYNC(_push)(BPRESYNC() _q,
                    TI         _x)
{
    // push symbol into buffers
    bsequence_push(_q->rx_i, REAL(_x)>0);
    bsequence_push(_q->rx_q, IMAG(_x)>0);
    return LIQUID_OK;
}

/* correlate input sequence                                 */
/*  _q          :   pre-demod synchronizer object           */
/*  _rxy        :   output cross correlation                */
/*  _dphi_hat   :   output frequency offset estimate        */
int BPRESYNC(_execute)(BPRESYNC() _q,
                       TO *       _rxy,
                       float *    _dphi_hat)
{
    unsigned int i;
    float complex rxy_max = 0;  // maximum cross-correlation
    float abs_rxy_max = 0;      // absolute value of rxy_max
    float complex rxy0;
    float complex rxy1;
    float dphi_hat = 0.0f;
    for (i=0; i<_q->m; i++)  {

        BPRESYNC(_correlatex)(_q, i, &rxy0, &rxy1);

        // check non-conjugated value
        if ( ABS(rxy0) > abs_rxy_max ) {
            rxy_max     = rxy0;
            abs_rxy_max = ABS(rxy0);
            dphi_hat    = _q->dphi[i];
        }

        // check conjugated value
        if ( ABS(rxy1) > abs_rxy_max ) {
            rxy_max     = rxy1;
            abs_rxy_max = ABS(rxy1);
            dphi_hat    = -_q->dphi[i];
        }
    }

    *_rxy      = rxy_max;
    *_dphi_hat = dphi_hat;
    return LIQUID_OK;
}

//
// internal methods
//

// correlate input sequence with particular sequence index
//  _q      : pre-demod synchronizer object
//  _id     : sequence index
//  _rxy0   : positive frequency correlation output (non-conjugated)
//  _rxy1   : negative frequency correlation output (conjugated)
int BPRESYNC(_correlatex)(BPRESYNC()      _q,
                          unsigned int    _id,
                          float complex * _rxy0,
                          float complex * _rxy1)
{
    // validate input...
    if (_id >= _q->m)
        return liquid_error(LIQUID_EICONFIG,"bpresync_%s_correlatex(), invalid id", EXTENSION_FULL);

    // compute correlations
    int rxy_ii = 2*bsequence_correlate(_q->sync_i[_id], _q->rx_i) - (int)(_q->n);
    int rxy_qq = 2*bsequence_correlate(_q->sync_q[_id], _q->rx_q) - (int)(_q->n);
    int rxy_iq = 2*bsequence_correlate(_q->sync_i[_id], _q->rx_q) - (int)(_q->n);
    int rxy_qi = 2*bsequence_correlate(_q->sync_q[_id], _q->rx_i) - (int)(_q->n);

    // non-conjugated
    int rxy_i0 = rxy_ii - rxy_qq;
    int rxy_q0 = rxy_iq + rxy_qi;
    *_rxy0 = (rxy_i0 + rxy_q0 * _Complex_I) * _q->n_inv;

    // conjugated
    int rxy_i1 = rxy_ii + rxy_qq;
    int rxy_q1 = rxy_iq - rxy_qi;
    *_rxy1 = (rxy_i1 + rxy_q1 * _Complex_I) * _q->n_inv;
    return LIQUID_OK;
}

