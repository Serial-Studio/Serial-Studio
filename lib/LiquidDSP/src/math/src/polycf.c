/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// Complex floating-point polynomials (single precision)
// 

#include "liquid.internal.h"

#define MATRIX(name)    LIQUID_CONCAT(matrixcf, name)
#define POLY(name)      LIQUID_CONCAT(polycf,   name)
#define POLY_NAME       "polycf"
#define EXTENSION       "cf"
#define T               float complex
#define TC              float complex

#define T_COMPLEX       1
#define TI_COMPLEX      1

#define T_ABS(X)        cabsf(X)
#define TC_ABS(X)       cabsf(X)

// prototypes
#include "poly.common.proto.c"
#include "poly.expand.proto.c"
#include "poly.lagrange.proto.c"

// finds the complex roots of the polynomial
//  _p      :   polynomial array, ascending powers [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _roots  :   resulting complex roots [size: _k-1 x 1]
int polycf_findroots(float complex * _p,
                     unsigned int    _k,
                     float complex * _roots)
{
    return liquid_error(LIQUID_ENOIMP,"polycf_findroots(), complex root-finding not yet supported");
}

