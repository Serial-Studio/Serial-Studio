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
// sparse matrix API: boolean
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "liquid.internal.h"

// name-mangling macro
#define SMATRIX(name)       LIQUID_CONCAT(smatrixb,name)
#define EXTENSION           "b"

// primitive type
#define T                   unsigned char

// category (float/int/bool)
#define SMATRIX_FLOAT       0
#define SMATRIX_INT         0
#define SMATRIX_BOOL        1

// print macros
#define PRINTVAL_ZERO()     printf(" .");
#define PRINTVAL(V)         printf(" %1u", V);

// prototypes
#include "smatrix.proto.c"

// 
// smatrix cross methods
//

// multiply sparse binary matrix by floating-point matrix
//  _q  :   sparse matrix [size: A->M x A->N]
//  _x  :   input vector  [size:  mx  x  nx ]
//  _y  :   output vector [size:  my  x  ny ]
int smatrixb_mulf(smatrixb     _A,
                  float *      _x,
                  unsigned int _mx,
                  unsigned int _nx,
                  float *      _y,
                  unsigned int _my,
                  unsigned int _ny)
{
    // ensure lengths are valid
    if (_my != _A->M || _ny != _nx || _A->N != _mx )
        return liquid_error(LIQUID_EIRANGE,"matrix_mul(), invalid dimensions");

    unsigned int i;
    unsigned int j;

    // clear output matrix
    for (i=0; i<_my*_ny; i++)
        _y[i] = 0.0f;

    //
    for (i=0; i<_A->M; i++) {
        // find non-zero column entries in this row
        unsigned int p;
        for (p=0; p<_A->num_mlist[i]; p++) {
            for (j=0; j<_ny; j++) {
                //_y(i,j) += _x( _A->mlist[i][p], j);
                _y[i*_ny + j] += _x[ _A->mlist[i][p]*_nx + j];
            }
        }
    }
    return LIQUID_OK;
}

// multiply sparse binary matrix by floating-point vector
//  _q  :   sparse matrix
//  _x  :   input vector [size: _N x 1]
//  _y  :   output vector [size: _M x 1]
int smatrixb_vmulf(smatrixb _q,
                   float *  _x,
                   float *  _y)
{
    unsigned int i;
    unsigned int j;
    
    for (i=0; i<_q->M; i++) {

        // reset total
        _y[i] = 0.0f;

        // only accumulate values on non-zero entries
        for (j=0; j<_q->num_mlist[i]; j++)
            _y[i] += _x[ _q->mlist[i][j] ];
    }
    return LIQUID_OK;
}

