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
// Floating-point polynomials (double precision)
// 

#include "liquid.internal.h"

#define MATRIX(name)    LIQUID_CONCAT(matrix, name)
#define POLY(name)      LIQUID_CONCAT(poly,   name)
#define POLY_NAME       "poly"
#define EXTENSION       ""
#define T               double
#define TC              double complex

#define T_COMPLEX       0
#define TI_COMPLEX      1

#define T_ABS(X)        fabs(X)
#define TC_ABS(X)       cabs(X)

// prototypes
#include "poly.common.proto.c"
#include "poly.expand.proto.c"
#include "poly.lagrange.proto.c"

// finds the complex roots of the polynomial
//  _p      :   polynomial array, ascending powers [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _roots  :   resulting complex roots [size: _k-1 x 1]
int poly_findroots(double *         _p,
                   unsigned int     _k,
                   double complex * _roots)
{
    // find roots of polynomial using Bairstow's method (more
    // accurate and reliable than Durand-Kerner)
    liquid_poly_findroots_bairstow(_p,_k,_roots);

    // sort roots for consistent ordering
    qsort(_roots, _k-1, sizeof(double complex), &liquid_poly_sort_roots_compare);
    return LIQUID_OK;
}

