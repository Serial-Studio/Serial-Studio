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
// Floating-point equalizers
//

#include "liquid.internal.h"

// naming extensions (useful for print statements)
#define EXTENSION_SHORT "f"
#define EXTENSION_FULL  "rrrf"

#define EQLMS(name)     LIQUID_CONCAT(eqlms_rrrf,name)
#define EQRLS(name)     LIQUID_CONCAT(eqrls_rrrf,name)

#define DOTPROD(name)   LIQUID_CONCAT(dotprod_rrrf,name)
#define WINDOW(name)    LIQUID_CONCAT(windowf,name)
#define MATRIX(name)    LIQUID_CONCAT(matrixf,name)

#define T_COMPLEX       0
#define T               float

#define PRINTVAL(V)     printf("%5.2f ", V);

// prototypes
#include "eqlms.proto.c"
#include "eqrls.proto.c"

