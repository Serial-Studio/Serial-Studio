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
// Matrix Cholesky decomposition method definitions
//

#include <math.h>
#include "liquid.internal.h"

#define DEBUG_MATRIX_CHOL 0

// Compute Cholesky decomposition of a symmetric/Hermitian positive-
// definite matrix as A = L * L^T
//  _a      :   input square matrix [size: _n x _n]
//  _n      :   input matrix dimension
//  _l      :   output lower-triangular matrix
int MATRIX(_chol)(T *          _a,
                  unsigned int _n,
                  T *          _l)
{
    // reset L
    unsigned int i;
    for (i=0; i<_n*_n; i++)
        _l[i] = 0.0;

    unsigned int j;
    unsigned int k;
    T  a_jj;
    T  l_jj;
    T  l_ik;
    T  l_jk;
    TP t0;
    T  t1;
    for (j=0; j<_n; j++) {
        // assert that a_jj is real, positive
        a_jj = matrix_access(_a,_n,_n,j,j);
        if ( creal(a_jj) < 0.0 )
            return liquid_error(LIQUID_EICONFIG,"matrix_chol(), matrix is not positive definite (real{A[%u,%u]} = %12.4e < 0)",j,j,creal(a_jj));
#if T_COMPLEX
        if ( fabs(cimag(a_jj)) > 0.0 )
            return liquid_error(LIQUID_EICONFIG,"matrix_chol(), matrix is not positive definite (|imag{A[%u,%u]}| = %12.4e > 0)",j,j,fabs(cimag(a_jj)));
#endif

        // compute l_jj and store it in output matrix
        t0 = 0.0;
        for (k=0; k<j; k++) {
            l_jk = matrix_access(_l,_n,_n,j,k);
#if T_COMPLEX
            t0 += creal( l_jk * conj(l_jk) );
#else
            t0 += l_jk * l_jk;
#endif
        }
        // test to ensure a_jj > t0
        if ( creal(a_jj) < t0 )
            return liquid_error(LIQUID_EICONFIG,"matrix_chol(), matrix is not positive definite (real{A[%u,%u]} = %12.4e < %12.4e)",j,j,creal(a_jj),t0);

        l_jj = sqrt( a_jj - t0 );
        matrix_access(_l,_n,_n,j,j) = l_jj;

        for (i=j+1; i<_n; i++) {
            t1 = matrix_access(_a,_n,_n,i,j);
            for (k=0; k<j; k++) {
                l_ik = matrix_access(_l,_n,_n,i,k);
                l_jk = matrix_access(_l,_n,_n,j,k);
#if T_COMPLEX
                t1 -= l_ik * conj(l_jk);
#else
                t1 -= l_ik * l_jk;
#endif
            }
            // TODO : store inverse of l_jj to reduce number of divisions
            matrix_access(_l,_n,_n,i,j) = t1 / l_jj;
        }
    }
    return LIQUID_OK;
}

