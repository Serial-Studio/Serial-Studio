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
// sparse matrix API: short signed integer
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "liquid.internal.h"

// name-mangling macro
#define SMATRIX(name)       LIQUID_CONCAT(smatrixi,name)
#define EXTENSION           "i"

// primitive type
#define T                   short int

// category (float/int/bool)
#define SMATRIX_FLOAT       0
#define SMATRIX_INT         1
#define SMATRIX_BOOL        0

// print macros
#define PRINTVAL_ZERO()     printf(" .");
#define PRINTVAL(V)         printf(" %3d", V);

// prototypes
#include "smatrix.proto.c"

