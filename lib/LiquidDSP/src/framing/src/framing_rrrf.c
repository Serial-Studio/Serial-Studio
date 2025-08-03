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

// real floating-point synchronization and supporting objects

#include "liquid.internal.h"

// naming extensions (useful for print statements)
#define EXTENSION_SHORT     "f"
#define EXTENSION_FULL      "rrrf"

#define PRINTVAL(x)         printf("%12.4e + j%12.4e", crealf(x), cimagf(x))

#define T                   float
#define TO                  float
#define TC                  float
#define TI                  float
#define ABS(X)              cabsf(X)
#define REAL(X)             crealf(X)
#define IMAG(X)             cimagf(X)

#define TO_COMPLEX          0
#define TC_COMPLEX          0
#define TI_COMPLEX          0

// supporting references
#define AGC(name)           LIQUID_CONCAT(agc_rrrf,name)
#define BSYNC(name)         LIQUID_CONCAT(bsync_rrrf,name)
#define DOTPROD(name)       LIQUID_CONCAT(dotprod_rrrf,name)
#define EQLMS(name)         LIQUID_CONCAT(eqlms_rrrf,name)
#define MODEM(name)         LIQUID_CONCAT(modemcf,name)
#define NCO(name)           LIQUID_CONCAT(nco_rrrf,name)
#define SYMSYNC(name)       LIQUID_CONCAT(symsync_rrrf,name)
#define WINDOW(name)        LIQUID_CONCAT(windowf,name)

// object references
#define BSYNC(name)         LIQUID_CONCAT(bsync_rrrf,name)

// prototypes
#include "bsync.proto.c"

