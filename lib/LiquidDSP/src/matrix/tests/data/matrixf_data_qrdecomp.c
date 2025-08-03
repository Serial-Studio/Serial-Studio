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
// data for testing Q/R decomposition
//

#include <complex.h>

// matrixf_data_qrdecomp_A [size: 4 x 4]
float matrixf_data_qrdecomp_A[] = {
    1.000000000000 /* ( 0, 0) */,
    2.000000000000 /* ( 0, 1) */,
    3.000000000000 /* ( 0, 2) */,
    4.000000000000 /* ( 0, 3) */,
    5.000000000000 /* ( 1, 0) */,
    5.000000000000 /* ( 1, 1) */,
    7.000000000000 /* ( 1, 2) */,
    8.000000000000 /* ( 1, 3) */,
    6.000000000000 /* ( 2, 0) */,
    4.000000000000 /* ( 2, 1) */,
    8.000000000000 /* ( 2, 2) */,
    7.000000000000 /* ( 2, 3) */,
    1.000000000000 /* ( 3, 0) */,
    0.000000000000 /* ( 3, 1) */,
    3.000000000000 /* ( 3, 2) */,
    1.000000000000 /* ( 3, 3) */};

// matrixf_data_qrdecomp_Q [size: 4 x 4]
float matrixf_data_qrdecomp_Q[] = {
    0.125988157670 /* ( 0, 0) */,
    0.617707763884 /* ( 0, 1) */,
    0.571886263590 /* ( 0, 2) */,
    0.524890659168 /* ( 0, 3) */,
    0.629940788349 /* ( 1, 0) */,
    0.494166211107 /* ( 1, 1) */,
   -0.137252703262 /* ( 1, 2) */,
   -0.583211843520 /* ( 1, 3) */,
    0.755928946018 /* ( 2, 0) */,
   -0.444749589997 /* ( 2, 1) */,
   -0.114377252718 /* ( 2, 2) */,
    0.466569474816 /* ( 2, 3) */,
    0.125988157670 /* ( 3, 0) */,
   -0.420041279441 /* ( 3, 1) */,
    0.800640769025 /* ( 3, 2) */,
   -0.408248290464 /* ( 3, 3) */};

// matrixf_data_qrdecomp_R [size: 4 x 4]
float matrixf_data_qrdecomp_R[] = {
    7.937253933194 /* ( 0, 0) */,
    6.425396041157 /* ( 0, 1) */,
   11.212946032607 /* ( 0, 2) */,
   10.960969717268 /* ( 0, 3) */,
    0.000000000000 /* ( 1, 0) */,
    1.927248223319 /* ( 1, 1) */,
    0.494166211107 /* ( 1, 2) */,
    2.890872334978 /* ( 1, 3) */,
    0.000000000000 /* ( 2, 0) */,
    0.000000000000 /* ( 2, 1) */,
    2.241794153271 /* ( 2, 2) */,
    1.189523428266 /* ( 2, 3) */,
    0.000000000000 /* ( 3, 0) */,
    0.000000000000 /* ( 3, 1) */,
    0.000000000000 /* ( 3, 2) */,
    0.291605921760 /* ( 3, 3) */};

