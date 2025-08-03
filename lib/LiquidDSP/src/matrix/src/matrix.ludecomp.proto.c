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
// Matrix L/U decomposition method definitions
//

#include "liquid.internal.h"

// L/U/P decomposition, Crout's method
int MATRIX(_ludecomp_crout)(T *          _x,
                            unsigned int _rx,
                            unsigned int _cx,
                            T *          _l,
                            T *          _u,
                            T *          _p)
{
    // validate input
    if (_rx != _cx)
        return liquid_error(LIQUID_EICONFIG,"matrix_ludecomp_crout(), input matrix not square");

    unsigned int n = _rx;

    // reset L, U
    unsigned int i;
    for (i=0; i<n*n; i++) {
        _l[i] = 0.0;
        _u[i] = 0.0;
        _p[i] = 0.0;
    }

    unsigned int j,k,t;
    T l_ik, u_kj;
    for (k=0; k<n; k++) {
        for (i=k; i<n; i++) {
            l_ik = matrix_access(_x,n,n,i,k);
            for (t=0; t<k; t++) {
                l_ik -= matrix_access(_l,n,n,i,t)*
                        matrix_access(_u,n,n,t,k);
            }
            matrix_access(_l,n,n,i,k) = l_ik;
        }

        for (j=k; j<n; j++) {
            // set upper triangular matrix to unity on diagonal
            if (j==k) {
                matrix_access(_u,n,n,k,j) = 1.0f;
                continue;
            }

            u_kj = matrix_access(_x,n,n,k,j);
            for (t=0; t<k; t++) {
                u_kj -= matrix_access(_l,n,n,k,t)*
                        matrix_access(_u,n,n,t,j);
            }
            u_kj /= matrix_access(_l,n,n,k,k);
            matrix_access(_u,n,n,k,j) = u_kj;
        }
    }

    // set output permutation matrix to identity matrix
    return MATRIX(_eye)(_p,n);
}

// L/U/P decomposition, Doolittle's method
int MATRIX(_ludecomp_doolittle)(T *          _x,
                                unsigned int _rx,
                                unsigned int _cx,
                                T *          _l,
                                T *          _u,
                                T *          _p)
{
    // validate input
    if (_rx != _cx)
        return liquid_error(LIQUID_EICONFIG,"matrix_ludecomp_doolittle(), input matrix not square");

    unsigned int n = _rx;

    // reset L, U
    unsigned int i;
    for (i=0; i<n*n; i++) {
        _l[i] = 0.0;
        _u[i] = 0.0;
        _p[i] = 0.0;
    }

    unsigned int j,k,t;
    T u_kj, l_ik;
    for (k=0; k<n; k++) {
        // compute upper triangular matrix
        for (j=k; j<n; j++) {
            u_kj = matrix_access(_x,n,n,k,j);
            for (t=0; t<k; t++) {
                u_kj -= matrix_access(_l,n,n,k,t)*
                        matrix_access(_u,n,n,t,j);
            }
            matrix_access(_u,n,n,k,j) = u_kj;
        }

        // compute lower triangular matrix
        for (i=k; i<n; i++) {
            // set lower triangular matrix to unity on diagonal
            if (i==k) {
                matrix_access(_l,n,n,i,k) = 1.0f;
                continue;
            }

            l_ik = matrix_access(_x,n,n,i,k);
            for (t=0; t<k; t++) {
                l_ik -= matrix_access(_l,n,n,i,t)*
                        matrix_access(_u,n,n,t,k);
            }
            l_ik /= matrix_access(_u,n,n,k,k);
            matrix_access(_l,n,n,i,k) = l_ik;
        }
    }

    // set output permutation matrix to identity matrix
    return MATRIX(_eye)(_p,n);
}

