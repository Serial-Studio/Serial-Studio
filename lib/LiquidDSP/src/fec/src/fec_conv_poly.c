/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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
// convolutional code polynomials
//

#include "liquid.internal.h"

#if LIBFEC_ENABLED
#include <fec.h>

int fec_conv27_poly[2]  = {V27POLYA,
                           V27POLYB};

int fec_conv29_poly[2]  = {V29POLYA,
                           V29POLYB};

int fec_conv39_poly[3]  = {V39POLYA,
                           V39POLYB,
                           V39POLYC};

int fec_conv615_poly[6] = {V615POLYA,
                           V615POLYB,
                           V615POLYC,
                           V615POLYD,
                           V615POLYE,
                           V615POLYF};

#else

int fec_conv27_poly[2]  = {0,0};
int fec_conv29_poly[2]  = {0,0};
int fec_conv39_poly[3]  = {0,0,0};
int fec_conv615_poly[6] = {0,0,0,0,0,0};

#endif

