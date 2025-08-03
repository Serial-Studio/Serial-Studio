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
// data for testing matrix inversion
//

#include <complex.h>

// matrixf_data_inv_x [size: 5 x 5]
float matrixf_data_inv_x[] = {
    0.145655393600 /* ( 0, 0) */,
   -2.292126655579 /* ( 0, 1) */,
    0.928358852863 /* ( 0, 2) */,
    0.995244622231 /* ( 0, 3) */,
   -0.719965457916 /* ( 0, 4) */,
    1.625229239464 /* ( 1, 0) */,
    1.179069876671 /* ( 1, 1) */,
    0.023814691231 /* ( 1, 2) */,
   -0.458529949188 /* ( 1, 3) */,
    0.870123147964 /* ( 1, 4) */,
    1.599076509476 /* ( 2, 0) */,
    1.012132167816 /* ( 2, 1) */,
    0.240342438221 /* ( 2, 2) */,
   -0.663878023624 /* ( 2, 3) */,
    1.523158550262 /* ( 2, 4) */,
    1.400263786316 /* ( 3, 0) */,
   -0.016515849158 /* ( 3, 1) */,
    0.525676131248 /* ( 3, 2) */,
   -0.526886940002 /* ( 3, 3) */,
   -0.605886101723 /* ( 3, 4) */,
   -0.291201651096 /* ( 4, 0) */,
    0.635409533978 /* ( 4, 1) */,
    0.016531571746 /* ( 4, 2) */,
    0.113017730415 /* ( 4, 3) */,
   -0.886025428772 /* ( 4, 4) */};

// matrixf_data_inv_y [size: 5 x 5]
float matrixf_data_inv_y[] = {
    0.123047731616 /* ( 0, 0) */,
    1.264793339850 /* ( 0, 1) */,
   -0.888020214878 /* ( 0, 2) */,
    0.146648698334 /* ( 0, 3) */,
   -0.484762774689 /* ( 0, 4) */,
    0.031615676756 /* ( 1, 0) */,
   -0.041217620573 /* ( 1, 1) */,
    0.486809371567 /* ( 1, 2) */,
   -0.307386761818 /* ( 1, 3) */,
    0.980900315396 /* ( 1, 4) */,
    0.456515830075 /* ( 2, 0) */,
   -2.168499777786 /* ( 2, 1) */,
    2.469455722213 /* ( 2, 2) */,
    0.010642598564 /* ( 2, 3) */,
    1.737407148356 /* ( 2, 4) */,
    0.690799395919 /* ( 3, 0) */,
    1.532809684521 /* ( 3, 1) */,
   -0.611813824735 /* ( 3, 2) */,
   -1.028413056396 /* ( 3, 3) */,
    0.595460566672 /* ( 3, 4) */,
    0.078865348162 /* ( 4, 0) */,
   -0.290188077617 /* ( 4, 1) */,
    0.609005594780 /* ( 4, 2) */,
   -0.399620351004 /* ( 4, 3) */,
   -0.157493442155 /* ( 4, 4) */};

