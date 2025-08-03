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
// Matrix Gram-Schmidt Orthonormalization
//

#include <math.h>
#include "liquid.internal.h"

#define DEBUG_MATRIX_GRAMSCHMIDT 0

// compute projection of _u onto _v, store in _e
int MATRIX(_proj)(T *          _u,
                  T *          _v,
                  unsigned int _n,
                  T *          _e)
{
    // compute dot-product between _u and _v
    unsigned int i;
    T uv = 0.;
    T uu = 0.;
    for (i=0; i<_n; i++) {
        uv += _u[i] * _v[i];
        uu += _u[i] * _u[i];
    }

    // TODO : check magnitude of _uu
    T g = uv / uu;
    for (i=0; i<_n; i++)
        _e[i] = _u[i] * g;
    return LIQUID_OK;
}

// Orthnormalization using the Gram-Schmidt algorithm
int MATRIX(_gramschmidt)(T *          _x,
                         unsigned int _rx,
                         unsigned int _cx,
                         T *          _v)
{
    // validate input
    if (_rx == 0 || _cx == 0)
        return liquid_error(LIQUID_EICONFIG,"matrix_gramschmidt(), input matrix cannot have zero-length dimensions");

    unsigned int i;
    unsigned int j;
    unsigned int k;

    // copy _x to _u
    memmove(_v, _x, _rx * _cx * sizeof(T));

    unsigned int n = _rx;   // dimensionality of each vector
    T proj_ij[n];
    for (j=0; j<_cx; j++) {
        for (i=0; i<j; i++) {
            // v_j  <-  v_j - proj(v_i, v_j)

#if DEBUG_MATRIX_GRAMSCHMIDT
            printf("computing proj(v_%u, v_%u)\n", i, j);
#endif

            // compute proj(v_i, v_j)
            T vij = 0.;     // dotprod(v_i, v_j)
            T vii = 0.;     // dotprod(v_i, v_i)
            T ti;
            T tj;
            for (k=0; k<n; k++) {
                ti = matrix_access(_v, _rx, _cx, k, i);
                tj = matrix_access(_v, _rx, _cx, k, j);

                T prodij = ti * conj(tj);
                vij += prodij;

                T prodii = ti * conj(ti);
                vii += prodii;
            }
            // TODO : vii should be 1.0 from normalization step below
            T g = vij / vii;

            // complete projection
            for (k=0; k<n; k++)
                proj_ij[k] = matrix_access(_v, _rx, _cx, k, i) * g;

            // subtract projection from v_j
            for (k=0; k<n; k++)
                matrix_access(_v, _rx, _cx, k, j) -= proj_ij[k];
        }

        // normalize v_j
        T vjj = 0.;     // dotprod(v_j, v_j)
        T tj  = 0.;
        for (k=0; k<n; k++) {
            tj = matrix_access(_v, _rx, _cx, k, j);
            T prodjj = tj * conj(tj);
            vjj += prodjj;
        }
        // TODO : check magnitude of vjj
        T g = 1. / sqrt( creal(vjj) );
        for (k=0; k<n; k++)
            matrix_access(_v, _rx, _cx, k, j) *= g;

#if DEBUG_MATRIX_GRAMSCHMIDT
        MATRIX(_print)(_v, _rx, _cx);
#endif
    }
    return LIQUID_OK;
}

