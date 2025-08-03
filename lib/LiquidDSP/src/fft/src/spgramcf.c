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
// spectral periodogram API: complex floating-point
//

#include "liquid.internal.h"

// naming extensions (useful for print statements)
#define EXTENSION           "cf"

// name-mangling macros
#define ASGRAM(name)        LIQUID_CONCAT(asgramcf,name)
#define SPGRAM(name)        LIQUID_CONCAT(spgramcf,name)
#define SPWATERFALL(name)   LIQUID_CONCAT(spwaterfallcf,name)
#define WINDOW(name)        LIQUID_CONCAT(windowcf,name)
#define FFT(name)           LIQUID_CONCAT(fft,name)

#define T                   float           // primitive type (real)
#define TC                  float complex   // primitive type (complex)
#define TI                  float complex   // input type

#define TI_COMPLEX          1

#define PRINTVAL_T(X,F)     PRINTVAL_FLOAT(X,F)
#define PRINTVAL_TC(X,F)    PRINTVAL_CFLOAT(X,F)
#define PRINTVAL_TI(X,F)    PRINTVAL_CFLOAT(X,F)

// prototypes
#include "asgram.proto.c"
#include "spgram.proto.c"
#include "spwaterfall.proto.c"

