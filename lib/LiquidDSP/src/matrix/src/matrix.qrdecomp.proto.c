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

//
// Matrix Q/R decomposition method definitions
//

#include <math.h>
#include "liquid.internal.h"

#define DEBUG_MATRIX_QRDECOMP 1

// Q/R decomposition using the Gram-Schmidt algorithm
int MATRIX(_qrdecomp_gramschmidt)(T *          _x,
                                  unsigned int _m,
                                  unsigned int _n,
                                  T *          _q,
                                  T *          _r)
{
    // validate input
    if (_m != _n)
        return liquid_error(LIQUID_EIRANGE,"matrix_qrdecomp_gramschmidt(), input matrix not square");

    unsigned int n = _m;
    unsigned int i,j,k;

    // generate and initialize matrices
    T e[n*n];   // normalized...
    for (i=0; i<n*n; i++)
        e[i] = 0.0f;

    for (k=0; k<n; k++) {
        // e(i,k) <- _x(i,k)
        for (i=0; i<n; i++)
            matrix_access(e,n,n,i,k) = matrix_access(_x,n,n,i,k);

        // subtract...
        for (i=0; i<k; i++) {
            // compute dot product _x(:,k) * e(:,i)
            T g = 0;
            for (j=0; j<n; j++) {
                T prod = matrix_access(_x,n,n,j,k) * conj( matrix_access(e,n,n,j,i) );
                g += prod;
            }
            //printf("  i=%2u, g = %12.4e\n", i, crealf(g));
            for (j=0; j<n; j++)
                matrix_access(e,n,n,j,k) -= matrix_access(e,n,n,j,i) * g;
        }

        // compute e_k = e_k / |e_k|
        float ek = 0.0f;
        T ak;
        TP ak2;
        for (i=0; i<n; i++) {
            ak  = matrix_access(e,n,n,i,k);
            ak2 = T_ABS(ak);
            ak2 = ak2 * ak2;
            ek += ak2;
        }
        ek = sqrtf(ek);

        // normalize e
        for (i=0; i<n; i++)
            matrix_access(e,n,n,i,k) /= ek;
    }

    // move Q
    memmove(_q, e, n*n*sizeof(T));

    // compute R
    // j : row
    // k : column
    for (j=0; j<n; j++) {
        for (k=0; k<n; k++) {
            if (k < j) {
                matrix_access(_r,n,n,j,k) = 0.0f;
            } else {
                // compute dot product between and Q(:,j) and _x(:,k)
                T g = 0;
                for (i=0; i<n; i++) {
                    T prod = conj( matrix_access(_q,n,n,i,j) ) * matrix_access(_x,n,n,i,k);
                    g += prod;
                }
                matrix_access(_r,n,n,j,k) = g;
            }
        }
    }
    return LIQUID_OK;
}

