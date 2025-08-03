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
// Solve linear system of equations
//

#include <string.h>

#include "liquid.internal.h"

// solve linear system of n equations: Ax = b
//  _A      :   system matrix [size: _n x _n]
//  _n      :   system size
//  _b      :   equality vector [size: _n x 1]
//  _x      :   solution vector [size: _n x 1]
//  _opts   :   options (ignored for now)
int MATRIX(_linsolve)(T *          _A,
                      unsigned int _n,
                      T *          _b,
                      T *          _x,
                      void *       _opts)
{
#if 0
    T A_inv[_n*_n];
    memmove(A_inv, _A, _n*_n*sizeof(T));
    MATRIX(_inv)(A_inv,_n,_n);

    MATRIX(_mul)(A_inv, _n, _n,
                 _b,    _n,  1,
                 _x,    _n,  1);
#else
    unsigned int r;
    unsigned int c;

    // compute augmented matrix M [size: _n x _n+1] as
    // T:
    //  A11 A12 A13 ... A1n b1
    //  A21 A22 A23 ... A2n b2
    //  A31 A32 A33 ... A3n b3
    //  ...
    //  An1 An2 An3 ... Ann bn
    T M[_n*_n + _n];    // allocate array
    unsigned int m=0;   // output matrix index counter
    unsigned int a=0;   // input matrix index counter
    for (r=0; r<_n; r++) {
        for (c=0; c<_n; c++)
            M[m++] = _A[a++];

        M[m++] = _b[r];
    }

    // run Gauss-Jordan elimination on M
    // T:
    //  1   0   0   ... 0   x1
    //  0   1   0   ... 0   x2
    //  0   0   1   ... 0   x3
    //  ...
    //  0   0   0   ... 1   xn
    MATRIX(_gjelim)(M, _n, _n+1);

    // copy result from right-most column of M
    for (r=0; r<_n; r++)
        _x[r] = M[(_n+1)*(r+1)-1];
#endif
    return LIQUID_OK;
}

