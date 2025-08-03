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
// Float buffer
//

#include "liquid.internal.h"

// naming extensions (useful for print statements)
#define EXTENSION       "f"

#define BUFFER_TYPE_FLOAT

#define CBUFFER(name)   LIQUID_CONCAT(cbufferf, name)
#define WDELAY(name)    LIQUID_CONCAT(wdelayf,  name)
#define WINDOW(name)    LIQUID_CONCAT(windowf,  name)

#define T float
#define BUFFER_PRINT_LINE(B,I) \
    printf("  : %12.8f", B->v[I]);
#define BUFFER_PRINT_VALUE(V) \
    printf("  : %12.4e", V);

// prototypes
#include "cbuffer.proto.c"
#include "wdelay.proto.c"
#include "window.proto.c"

