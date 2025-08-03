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
// data for testing Gram-Schmidt orthonormalization
//

#include <complex.h>

// matrixf_data_gramschmidt_A [size: 4 x 3]
float matrixf_data_gramschmidt_A[] = {
    1.000000000000 /* ( 0, 0) */,
    2.000000000000 /* ( 0, 1) */,
    1.000000000000 /* ( 0, 2) */,
    0.000000000000 /* ( 1, 0) */,
    2.000000000000 /* ( 1, 1) */,
    0.000000000000 /* ( 1, 2) */,
    2.000000000000 /* ( 2, 0) */,
    3.000000000000 /* ( 2, 1) */,
    1.000000000000 /* ( 2, 2) */,
    1.000000000000 /* ( 3, 0) */,
    1.000000000000 /* ( 3, 1) */,
    0.000000000000 /* ( 3, 2) */};

// matrixf_data_gramschmidt_V [size: 4 x 3]
float matrixf_data_gramschmidt_V[] = {
    0.408248290464 /* ( 0, 0) */,
    0.235702260396 /* ( 0, 1) */,
    0.666666666667 /* ( 0, 2) */,
    0.000000000000 /* ( 1, 0) */,
    0.942809041582 /* ( 1, 1) */,
   -0.333333333333 /* ( 1, 2) */,
    0.816496580928 /* ( 2, 0) */,
    0.000000000000 /* ( 2, 1) */,
    0.000000000000 /* ( 2, 2) */,
    0.408248290464 /* ( 3, 0) */,
   -0.235702260396 /* ( 3, 1) */,
   -0.666666666667 /* ( 3, 2) */};

