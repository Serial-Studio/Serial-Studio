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
// Matrix method definitions
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// add elements of two matrices
//  _X      :   1st input matrix [size: _R x _C]
//  _Y      :   2nd input matrix [size: _R x _C]
//  _Z      :   output matrix [size: _R x _C]
//  _R      :   number of rows
//  _C      :   number of columns
int MATRIX(_add)(T *          _X,
                 T *          _Y,
                 T *          _Z,
                 unsigned int _R,
                 unsigned int _C)
{
    unsigned int i;
    for (i=0; i<(_R*_C); i++)
        _Z[i] = _X[i] + _Y[i];
    return LIQUID_OK;
}

// subtract elements of two matrices
//  _X      :   1st input matrix [size: _R x _C]
//  _Y      :   2nd input matrix [size: _R x _C]
//  _Z      :   output matrix [size: _R x _C]
//  _R      :   number of rows
//  _C      :   number of columns
int MATRIX(_sub)(T *          _X,
                 T *          _Y,
                 T *          _Z,
                 unsigned int _R,
                 unsigned int _C)
{
    unsigned int i;
    for (i=0; i<(_R*_C); i++)
        _Z[i] = _X[i] - _Y[i];
    return LIQUID_OK;
}

// point-wise multiplication
//  _X      :   1st input matrix [size: _R x _C]
//  _Y      :   2nd input matrix [size: _R x _C]
//  _Z      :   output matrix [size: _R x _C]
//  _R      :   number of rows
//  _C      :   number of columns
int MATRIX(_pmul)(T *          _X,
                  T *          _Y,
                  T *          _Z,
                  unsigned int _R,
                  unsigned int _C)
{
    unsigned int i;
    for (i=0; i<(_R*_C); i++)
        _Z[i] = _X[i] * _Y[i];
    return LIQUID_OK;
}

// point-wise division
//  _X      :   1st input matrix [size: _R x _C]
//  _Y      :   2nd input matrix [size: _R x _C]
//  _Z      :   output matrix [size: _R x _C]
//  _R      :   number of rows
//  _C      :   number of columns
int MATRIX(_pdiv)(T *          _X,
                  T *          _Y,
                  T *          _Z,
                  unsigned int _R,
                  unsigned int _C)
{
    unsigned int i;
    for (i=0; i<(_R*_C); i++)
        _Z[i] = _X[i] / _Y[i];
    return LIQUID_OK;
}


// multiply two matrices together
int MATRIX(_mul)(T * _X, unsigned int _XR, unsigned int _XC,
                 T * _Y, unsigned int _YR, unsigned int _YC,
                 T * _Z, unsigned int _ZR, unsigned int _ZC)
{
    // ensure lengths are valid
    if (_ZR != _XR || _ZC != _YC || _XC != _YR )
        return liquid_error(LIQUID_EIRANGE,"matrix_mul(), invalid dimensions");

    unsigned int r, c, i;
    for (r=0; r<_ZR; r++) {
        for (c=0; c<_ZC; c++) {
            // z(i,j) = dotprod( x(i,:), y(:,j) )
            T sum=0.0f;
            for (i=0; i<_XC; i++) {
                sum += matrix_access(_X,_XR,_XC,r,i) *
                       matrix_access(_Y,_YR,_YC,i,c);
            }
            matrix_access(_Z,_ZR,_ZC,r,c) = sum;
#ifdef DEBUG
            printf("z(%u,%u) = ", r, c);
            MATRIX_PRINT_ELEMENT(_Z,_ZR,_ZC,r,c);
            printf("\n");
#endif
        }
    }
    return LIQUID_OK;
}

// augment matrices x and y:
//  z = [x | y]
int MATRIX(_aug)(T * _x, unsigned int _rx, unsigned int _cx,
                 T * _y, unsigned int _ry, unsigned int _cy,
                 T * _z, unsigned int _rz, unsigned int _cz)
{
    // ensure lengths are valid
    if (_rz != _rx || _rz != _ry || _rx != _ry || _cz != _cx + _cy)
        return liquid_error(LIQUID_EIRANGE,"matrix_aug(), invalid dimensions");

    // TODO: improve speed with memmove
    unsigned int r, c, n;
    for (r=0; r<_rz; r++) {
        n=0;
        for (c=0; c<_cx; c++)
            matrix_access(_z,_rz,_cz,r,n++) = matrix_access(_x,_rx,_cx,r,c);
        for (c=0; c<_cy; c++)
            matrix_access(_z,_rz,_cz,r,n++) = matrix_access(_y,_ry,_cy,r,c);
    }
    return LIQUID_OK;
}

// solve set of linear equations
int MATRIX(_div)(T *          _X,
                 T *          _Y,
                 T *          _Z,
                 unsigned int _n)
{
    // compute inv(_Y)
    T Y_inv[_n*_n];
    memmove(Y_inv, _Y, _n*_n*sizeof(T));
    MATRIX(_inv)(Y_inv,_n,_n);

    // _Z = _X * inv(_Y)
    return
    MATRIX(_mul)(_X,    _n, _n,
                 Y_inv, _n, _n,
                 _Z,    _n, _n);
}

// matrix determinant (2 x 2)
T MATRIX(_det2x2)(T *          _X,
                  unsigned int _r,
                  unsigned int _c)
{
    // validate input
    if (_r != 2 || _c != 2)
        return liquid_error(LIQUID_EIRANGE,"matrix_det2x2(), invalid dimensions");

    return _X[0]*_X[3] - _X[1]*_X[2];
}

// matrix determinant (n x n)
T MATRIX(_det)(T *          _X,
               unsigned int _r,
               unsigned int _c)
{
    // validate input
    if (_r != _c)
        return liquid_error(LIQUID_EIRANGE,"matrix_det(), matrix must be square");

    unsigned int n = _r;
    if (n==2) return MATRIX(_det2x2)(_X,2,2);

    // compute L/U decomposition (Doolittle's method)
    T L[n*n]; // lower
    T U[n*n]; // upper
    T P[n*n]; // permutation
    MATRIX(_ludecomp_doolittle)(_X,n,n,L,U,P);

    // evaluate along the diagonal of U
    T det = 1.0;
    unsigned int i;
    for (i=0; i<n; i++)
        det *= matrix_access(U,n,n,i,i);

    return det;
}

// compute matrix transpose
int MATRIX(_trans)(T *          _X,
                   unsigned int _XR,
                   unsigned int _XC)
{
    // compute Hermitian transpose
    MATRIX(_hermitian)(_X,_XR,_XC);
    
    // conjugate elements
    unsigned int i;
    for (i=0; i<_XR*_XC; i++)
        _X[i] = conj(_X[i]);
    return LIQUID_OK;
}

// compute matrix Hermitian transpose
int MATRIX(_hermitian)(T *          _X,
                       unsigned int _XR,
                       unsigned int _XC)
{
    T y[_XR*_XC];
    memmove(y,_X,_XR*_XC*sizeof(T));

    unsigned int r,c;
    for (r=0; r<_XR; r++) {
        for (c=0; c<_XC; c++) {
            matrix_access(_X,_XC,_XR,c,r) = matrix_access(y,_XR,_XC,r,c);
        }
    }
    return LIQUID_OK;
}

// compute x*x' on m x n matrix, result: m x m
int MATRIX(_mul_transpose)(T *          _x,
                           unsigned int _m,
                           unsigned int _n,
                           T *          _xxT)
{
    unsigned int r;
    unsigned int c;
    unsigned int i;

    // clear _xxT
    for (i=0; i<_m*_m; i++)
        _xxT[i] = 0.0f;

    // 
    T sum = 0;
    for (r=0; r<_m; r++) {

        for (c=0; c<_m; c++) {
            sum = 0.0f;

            for (i=0; i<_n; i++) {
                T prod =        matrix_access(_x,_m,_n,r,i) *
                         conjf( matrix_access(_x,_m,_n,c,i) );
                sum += prod;
            }

            matrix_access(_xxT,_m,_m,r,c) = sum;
        }
    }
    return LIQUID_OK;
}


// compute x'*x on m x n matrix, result: n x n
int MATRIX(_transpose_mul)(T *          _x,
                           unsigned int _m,
                           unsigned int _n,
                           T *          _xTx)
{
    unsigned int r;
    unsigned int c;
    unsigned int i;

    // clear _xTx
    for (i=0; i<_n*_n; i++)
        _xTx[i] = 0.0f;

    // 
    T sum = 0;
    for (r=0; r<_n; r++) {

        for (c=0; c<_n; c++) {
            sum = 0.0f;

            for (i=0; i<_m; i++) {
                T prod = conjf( matrix_access(_x,_m,_n,i,r) ) *
                                matrix_access(_x,_m,_n,i,c);
                sum += prod;
            }

            matrix_access(_xTx,_n,_n,r,c) = sum;
        }
    }
    return LIQUID_OK;
}


// compute x*x.' on m x n matrix, result: m x m
int MATRIX(_mul_hermitian)(T *          _x,
                           unsigned int _m,
                           unsigned int _n,
                           T *          _xxH)
{
    unsigned int r;
    unsigned int c;
    unsigned int i;

    // clear _xxH
    for (i=0; i<_m*_m; i++)
        _xxH[i] = 0.0f;

    // 
    T sum = 0;
    for (r=0; r<_m; r++) {

        for (c=0; c<_m; c++) {
            sum = 0.0f;

            for (i=0; i<_n; i++) {
                sum += matrix_access(_x,_m,_n,r,i) *
                       matrix_access(_x,_m,_n,c,i);
            }

            matrix_access(_xxH,_m,_m,r,c) = sum;
        }
    }
    return LIQUID_OK;
}


// compute x.'*x on m x n matrix, result: n x n
int MATRIX(_hermitian_mul)(T *          _x,
                           unsigned int _m,
                           unsigned int _n,
                           T *          _xHx)
{
    unsigned int r;
    unsigned int c;
    unsigned int i;

    // clear _xHx
    for (i=0; i<_n*_n; i++)
        _xHx[i] = 0.0f;

    // 
    T sum = 0;
    for (r=0; r<_n; r++) {

        for (c=0; c<_n; c++) {
            sum = 0.0f;

            for (i=0; i<_m; i++) {
                sum += matrix_access(_x,_m,_n,i,r) *
                       matrix_access(_x,_m,_n,i,c);
            }

            matrix_access(_xHx,_n,_n,r,c) = sum;
        }
    }
    return LIQUID_OK;
}

