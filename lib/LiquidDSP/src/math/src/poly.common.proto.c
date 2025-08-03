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
// common polynomial routines
//  polyval
//  polyfit
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

T POLY(_val)(T * _p, unsigned int _k, T _x)
{
    unsigned int i;
    T xk = 1;
    T y = 0.0f;
    for (i=0; i<_k; i++) {
        y += _p[i]*xk;
        xk *= _x;
    }
    return y;
}

int POLY(_fit)(T *          _x,
               T *          _y,
               unsigned int _n,
               T *          _p,
               unsigned int _k)
{

    // ...
    T X[_n*_k];
    unsigned int r,c;
    T v;
    for (r=0; r<_n; r++) {
        v = 1;
        for (c=0; c<_k; c++) {
            matrix_access(X,_n,_k,r,c) = v;
            v *= _x[r];
        }
    }

    // compute transpose of X
    T Xt[_k*_n];
    memmove(Xt,X,_k*_n*sizeof(T));
    MATRIX(_trans)(Xt,_n,_k);

    // compute [X']*y
    T Xty[_k];
    MATRIX(_mul)(Xt, _k, _n,
                 _y, _n, 1,
                 Xty,_k, 1);

    // compute [X']*X
    T X2[_k*_k];
    MATRIX(_mul)(Xt, _k, _n,
                 X,  _n, _k,
                 X2, _k, _k);

    // compute inv([X']*X)
    T G[_k*_k];
    memmove(G,X2,_k*_k*sizeof(T));
    MATRIX(_inv)(G,_k,_k);

    // compute coefficients
    MATRIX(_mul)(G,  _k, _k,
                 Xty,_k, 1,
                 _p, _k, 1);
    return LIQUID_OK;
}

