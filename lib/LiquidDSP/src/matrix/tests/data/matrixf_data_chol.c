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
// data for testing Cholesky decomposition
//

#include <complex.h>

// matrixf_data_chol_L [size: 4 x 4]
float matrixf_data_chol_L[] = {
    1.010000000000 /* ( 0, 0) */,
    0.000000000000 /* ( 0, 1) */,
    0.000000000000 /* ( 0, 2) */,
    0.000000000000 /* ( 0, 3) */,
   -1.420000000000 /* ( 1, 0) */,
    0.500000000000 /* ( 1, 1) */,
    0.000000000000 /* ( 1, 2) */,
    0.000000000000 /* ( 1, 3) */,
    0.320000000000 /* ( 2, 0) */,
    2.010000000000 /* ( 2, 1) */,
    0.300000000000 /* ( 2, 2) */,
    0.000000000000 /* ( 2, 3) */,
   -1.020000000000 /* ( 3, 0) */,
   -0.320000000000 /* ( 3, 1) */,
   -1.650000000000 /* ( 3, 2) */,
    1.070000000000 /* ( 3, 3) */};

// matrixf_data_chol_A [size: 4 x 4]
float matrixf_data_chol_A[] = {
    1.020100000000 /* ( 0, 0) */,
   -1.434200000000 /* ( 0, 1) */,
    0.323200000000 /* ( 0, 2) */,
   -1.030200000000 /* ( 0, 3) */,
   -1.434200000000 /* ( 1, 0) */,
    2.266400000000 /* ( 1, 1) */,
    0.550600000000 /* ( 1, 2) */,
    1.288400000000 /* ( 1, 3) */,
    0.323200000000 /* ( 2, 0) */,
    0.550600000000 /* ( 2, 1) */,
    4.232500000000 /* ( 2, 2) */,
   -1.464600000000 /* ( 2, 3) */,
   -1.030200000000 /* ( 3, 0) */,
    1.288400000000 /* ( 3, 1) */,
   -1.464600000000 /* ( 3, 2) */,
    5.010200000000 /* ( 3, 3) */};

