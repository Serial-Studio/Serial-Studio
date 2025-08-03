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

#include "autotest/autotest.h"
#include "liquid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "liquid.internal.h"

void autotest_optim_rosenbrock()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping rosenbrock config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif

    // optimum
    float v_ones[8] = {1,1,1,1,1,1,1,1};
    CONTEND_DELTA( liquid_rosenbrock(NULL, v_ones, 8), 0.0f, 1e-6f )
    CONTEND_DELTA( liquid_rosenbrock(NULL, v_ones, 1), 0.0f, 1e-6f )

    // very far from optimum
    float v_misc[8] = {0.3, 1.0, 4.5,-2.2, 6.7,-0.2, 1.1,-0.9,};
    CONTEND_GREATER_THAN( liquid_rosenbrock(NULL, v_misc, 8), 1000.0f )

    // invalid configuration
    CONTEND_EQUALITY( liquid_rosenbrock(NULL, v_misc, 0), 0.0f )
}

