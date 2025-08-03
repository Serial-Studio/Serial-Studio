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
// Matrix inverse method definitions
//

#include "liquid.internal.h"

int MATRIX(_inv)(T * _X, unsigned int _XR, unsigned int _XC)
{
    // ensure lengths are valid
    if (_XR != _XC )
        return liquid_error(LIQUID_EICONFIG,"matrix_inv(), invalid dimensions");

    // X:
    //  x11 x12 ... x1n
    //  x21 x22 ... x2n
    //  ...
    //  xn1 xn2 ... xnn

    // allocate temporary memory
    T x[2*_XR*_XC];
    unsigned int xr = _XR;
    unsigned int xc = _XC*2;

    // x:
    //  x11 x12 ... x1n 1   0   ... 0
    //  x21 x22 ... x2n 0   1   ... 0
    //  ...
    //  xn1 xn2 ... xnn 0   0   ... 1
    unsigned int r,c;
    for (r=0; r<_XR; r++) {
        // copy matrix elements
        for (c=0; c<_XC; c++)
            matrix_access(x,xr,xc,r,c) = matrix_access(_X,_XR,_XC,r,c);

        // append identity matrix
        for (c=0; c<_XC; c++)
            matrix_access(x,xr,xc,r,_XC+c) = (r==c) ? 1 : 0;
    }

    // perform Gauss-Jordan elimination on x
    // x:
    //  1   0   ... 0   y11 y12 ... y1n
    //  0   1   ... 0   y21 y22 ... y2n
    //  ...
    //  0   0   ... 1   yn1 yn2 ... ynn
    MATRIX(_gjelim)(x,xr,xc);

    // copy result from right half of x
    for (r=0; r<_XR; r++) {
        for (c=0; c<_XC; c++)
            matrix_access(_X,_XR,_XC,r,c) = matrix_access(x,xr,xc,r,_XC+c);
    }
    return LIQUID_OK;
}

// Gauss-Jordan elmination
int MATRIX(_gjelim)(T * _X, unsigned int _XR, unsigned int _XC)
{
    unsigned int r, c;

    // choose pivot rows based on maximum element along column
    float v;
    float v_max=0.;
    unsigned int r_opt=0;
    unsigned int r_hat;
    for (r=0; r<_XR; r++) {

        // check values along this column and find the maximum
        for (r_hat=r; r_hat<_XR; r_hat++) {
            v = T_ABS( matrix_access(_X,_XR,_XC,r_hat,r) );
            // swap rows if necessary
            if (v > v_max || r_hat==r) {
                r_opt = r_hat;
                v_max = v;
            }
        }

        // if the maximum is zero, matrix is singular
        if (v_max == 0.0f)
            return liquid_error(LIQUID_EICONFIG,"matrix_gjelim(), matrix singular to machine precision");

        // if row does not match column (e.g. maximum value does not
        // lie on the diagonal) swap the rows
        if (r != r_opt) {
            MATRIX(_swaprows)(_X,_XR,_XC,r,r_opt);
        }

        // pivot on the diagonal element
        MATRIX(_pivot)(_X,_XR,_XC,r,r);
    }

    // scale by diagonal
    T g;
    for (r=0; r<_XR; r++) {
        g = 1 / matrix_access(_X,_XR,_XC,r,r);
        for (c=0; c<_XC; c++)
            matrix_access(_X,_XR,_XC,r,c) *= g;
    }
    return LIQUID_OK;
}

// pivot on element _r, _c
int MATRIX(_pivot)(T * _X, unsigned int _XR, unsigned int _XC, unsigned int _r, unsigned int _c)
{
    T v = matrix_access(_X,_XR,_XC,_r,_c);
    if (v==0)
        return liquid_error(LIQUID_EICONFIG,"matrix_pivot(), pivoting on zero");

    unsigned int r,c;

    // pivot using back-substitution
    T g;    // multiplier
    for (r=0; r<_XR; r++) {

        // skip over pivot row
        if (r == _r)
            continue;

        // compute multiplier
        g = matrix_access(_X,_XR,_XC,r,_c) / v;

        // back-substitution
        for (c=0; c<_XC; c++) {
            matrix_access(_X,_XR,_XC,r,c) = g*matrix_access(_X,_XR,_XC,_r,c) -
                                              matrix_access(_X,_XR,_XC, r,c);
        }
    }
    return LIQUID_OK;
}

int MATRIX(_swaprows)(T * _X, unsigned int _XR, unsigned int _XC, unsigned int _r1, unsigned int _r2)
{
    if (_r1 == _r2)
        return LIQUID_OK;

    unsigned int c;
    T v_tmp;
    for (c=0; c<_XC; c++) {
        v_tmp = matrix_access(_X,_XR,_XC,_r1,c);
        matrix_access(_X,_XR,_XC,_r1,c) = matrix_access(_X,_XR,_XC,_r2,c);
        matrix_access(_X,_XR,_XC,_r2,c) = v_tmp;
    }
    return LIQUID_OK;
}
