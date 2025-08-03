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
// P/N synchronizer
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "liquid.internal.h"

struct BSYNC(_s) {
    unsigned int n;     // sequence length

    bsequence sync_i;   // synchronization pattern (in-phase)
    bsequence sym_i;    // received symbols (in-phase)
//#if TC_COMPLEX==1
    bsequence sync_q;   // synchronization pattern (quadrature)
//#endif

//#if TI_COMPLEX==1
    bsequence sym_q;    // received symbols (quadrature)
//#endif
    TO rxy;             // cross correlation
};

BSYNC() BSYNC(_create)(unsigned int _n, TC * _v)
{
    BSYNC() fs = (BSYNC()) malloc(sizeof(struct BSYNC(_s)));
    fs->n = _n;

    fs->sync_i  = bsequence_create(fs->n);
#if TC_COMPLEX==1
    fs->sync_q  = bsequence_create(fs->n);
#endif

    fs->sym_i   = bsequence_create(fs->n);
#if TI_COMPLEX==1
    fs->sym_q   = bsequence_create(fs->n);
#endif

    unsigned int i;
    for (i=0; i<fs->n; i++) {
        bsequence_push(fs->sync_i, crealf(_v[i])>0);
#if TC_COMPLEX==1
        bsequence_push(fs->sync_q, cimagf(_v[i])>0);
#endif
    }

    return fs;
}

// TODO : test this method
BSYNC() BSYNC(_create_msequence)(unsigned int _g,
                                 unsigned int _k)
{
    // validate input
    if (_k == 0)
        return liquid_error_config("bsync_xxxt_create_msequence(), samples/symbol must be greater than zero");

    unsigned int m = liquid_msb_index(_g) - 1;

    // create/initialize msequence
    msequence ms = msequence_create(m, _g, 1);

    BSYNC() fs = (BSYNC()) malloc(sizeof(struct BSYNC(_s)));
    unsigned int n = msequence_get_length(ms);

    fs->sync_i  = bsequence_create(n * _k);
#if TC_COMPLEX==1
    fs->sync_q  = bsequence_create(n * _k);
#endif

    fs->sym_i   = bsequence_create(n * _k);
#if TI_COMPLEX==1
    fs->sym_q   = bsequence_create(n * _k);
#endif

    msequence_reset(ms);

#if 0
    bsequence_init_msequence(fs->sync_i,ms);
#if TC_COMPLEX==1
    msequence_reset(ms);
    bsequence_init_msequence(fs->sync_q,ms);
#endif
#else
    unsigned int i;
    unsigned int j;
    for (i=0; i<n; i++) {
        unsigned int bit = msequence_advance(ms);

        for (j=0; j<_k; j++) {
            bsequence_push(fs->sync_i, bit);
#if TC_COMPLEX==1
            bsequence_push(fs->sync_q, bit);
#endif
        }
    }
#endif

    msequence_destroy(ms);

    fs->n = _k*n;

    return fs;
}

int BSYNC(_destroy)(BSYNC() _fs)
{
    bsequence_destroy(_fs->sync_i);
#if TC_COMPLEX==1
    bsequence_destroy(_fs->sync_q);
#endif

    bsequence_destroy(_fs->sym_i);
#if TI_COMPLEX==1
    bsequence_destroy(_fs->sym_q);
#endif
    free(_fs);
    return LIQUID_OK;
}

int BSYNC(_print)(BSYNC() _fs)
{
    printf("<liquid.bsync_%s, len=%u>\n", EXTENSION_FULL, _fs->n);
    return LIQUID_OK;
}

int BSYNC(_correlate)(BSYNC() _fs, TI _sym, TO *_y)
{
    // push symbol into buffers
    bsequence_push(_fs->sym_i, crealf(_sym)>0.0 ? 1 : 0);
#if TI_COMPLEX==1
    bsequence_push(_fs->sym_q, cimagf(_sym)>0.0 ? 1 : 0);
#endif

    // compute dotprod
#if TC_COMPLEX==1 && TI_COMPLEX==1
    // cccx
    TO rxy_ii = 2.*bsequence_correlate(_fs->sync_i, _fs->sym_i) - (float)(_fs->n);
    TO rxy_qq = 2.*bsequence_correlate(_fs->sync_q, _fs->sym_q) - (float)(_fs->n);
    TO rxy_iq = 2.*bsequence_correlate(_fs->sync_i, _fs->sym_q) - (float)(_fs->n);
    TO rxy_qi = 2.*bsequence_correlate(_fs->sync_q, _fs->sym_i) - (float)(_fs->n);

    _fs->rxy = (rxy_ii - rxy_qq) + _Complex_I*(rxy_iq + rxy_qi);
#elif TI_COMPLEX==1
    // crcx
    float rxy_ii = 2.*bsequence_correlate(_fs->sync_i, _fs->sym_i) - (float)(_fs->n);
    float rxy_iq = 2.*bsequence_correlate(_fs->sync_i, _fs->sym_q) - (float)(_fs->n);

    _fs->rxy = rxy_ii + _Complex_I * rxy_iq;
#else
    // rrrx
    _fs->rxy = 2.*bsequence_correlate(_fs->sync_i, _fs->sym_i) - (float)(_fs->n);
#endif

    // divide by sequence length
    *_y = _fs->rxy / (float)(_fs->n);
    return LIQUID_OK;
}

