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
// Matrix method base definitions
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int MATRIX(_print)(T *          _X,
                   unsigned int _R,
                   unsigned int _C)
{
    printf("matrix [%u x %u] : \n", _R, _C);
    unsigned int r,c;
    for (r=0; r<_R; r++) {
        for (c=0; c<_C; c++) {
            MATRIX_PRINT_ELEMENT(_X,_R,_C,r,c);
        }
        printf("\n");
    }
    return LIQUID_OK;
}

// initialize square matrix to the identity matrix
int MATRIX(_eye)(T * _x, unsigned int _n)
{
    unsigned int r,c,k=0;
    for (r=0; r<_n; r++) {
        for (c=0; c<_n; c++) {
            _x[k++] = r==c ? 1. : 0.;
        }
    }
    return LIQUID_OK;
}

// initialize matrix to ones
int MATRIX(_ones)(T *          _x,
                  unsigned int _r,
                  unsigned int _c)
{
    unsigned int k;
    for (k=0; k<_r*_c; k++)
        _x[k] = 1.;
    return LIQUID_OK;
}

// initialize matrix to zeros
int MATRIX(_zeros)(T *          _x,
                   unsigned int _r,
                   unsigned int _c)
{
    unsigned int k;
    for (k=0; k<_r*_c; k++)
        _x[k] = 0.;
    return LIQUID_OK;
}

