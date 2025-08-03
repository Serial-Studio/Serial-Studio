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

// matrixcf_data_qrdecomp_A [size: 4 x 4]
float complex matrixcf_data_qrdecomp_A[] = {
    2.114020000000 +  -0.576040000000*_Complex_I /* ( 0, 0) */,
    0.417500000000 +   1.008330000000*_Complex_I /* ( 0, 1) */,
   -0.962640000000 +  -3.621960000000*_Complex_I /* ( 0, 2) */,
   -0.206790000000 +  -1.026680000000*_Complex_I /* ( 0, 3) */,
    0.008540000000 +   1.616260000000*_Complex_I /* ( 1, 0) */,
    0.846950000000 +  -0.327360000000*_Complex_I /* ( 1, 1) */,
   -1.018620000000 +  -1.107860000000*_Complex_I /* ( 1, 2) */,
   -1.788770000000 +   1.844560000000*_Complex_I /* ( 1, 3) */,
   -2.979010000000 +  -1.303840000000*_Complex_I /* ( 2, 0) */,
    0.522890000000 +   1.891100000000*_Complex_I /* ( 2, 1) */,
    1.325760000000 +  -0.367370000000*_Complex_I /* ( 2, 2) */,
    0.047170000000 +   0.206280000000*_Complex_I /* ( 2, 3) */,
    0.289700000000 +   0.642470000000*_Complex_I /* ( 3, 0) */,
   -0.559160000000 +   0.683020000000*_Complex_I /* ( 3, 1) */,
    1.406150000000 +   0.623980000000*_Complex_I /* ( 3, 2) */,
   -0.127670000000 +  -0.539970000000*_Complex_I /* ( 3, 3) */};

// matrixcf_data_qrdecomp_Q [size: 4 x 4]
float complex matrixcf_data_qrdecomp_Q[] = {
    0.491706158979 +  -0.133982845866*_Complex_I /* ( 0, 0) */,
    0.429660711419 +   0.559833033911*_Complex_I /* ( 0, 1) */,
   -0.309333641162 +  -0.278321211351*_Complex_I /* ( 0, 2) */,
    0.215207397547 +  -0.150957196713*_Complex_I /* ( 0, 3) */,
    0.001986343837 +   0.375930689639*_Complex_I /* ( 1, 0) */,
    0.242768204454 +   0.009257007128*_Complex_I /* ( 1, 1) */,
   -0.422306122793 +  -0.032511505165*_Complex_I /* ( 1, 2) */,
   -0.503566009661 +   0.605534385769*_Complex_I /* ( 1, 3) */,
   -0.692896739226 +  -0.303263998601*_Complex_I /* ( 2, 0) */,
    0.054111560749 +   0.468071856237*_Complex_I /* ( 2, 1) */,
   -0.082147488614 +   0.069653107384*_Complex_I /* ( 2, 2) */,
    0.279669645547 +   0.340721083028*_Complex_I /* ( 2, 3) */,
    0.067382179098 +   0.149433995875*_Complex_I /* ( 3, 0) */,
   -0.270466351267 +   0.384428384950*_Complex_I /* ( 3, 1) */,
   -0.285071449427 +   0.744704670261*_Complex_I /* ( 3, 2) */,
   -0.173581995183 +  -0.293616086507*_Complex_I /* ( 3, 3) */};

// matrixcf_data_qrdecomp_R [size: 4 x 4]
float complex matrixcf_data_qrdecomp_R[] = {
    4.299356356224 +   0.000000000000*_Complex_I /* ( 0, 0) */,
   -0.922616273377 +  -0.789487259898*_Complex_I /* ( 0, 1) */,
   -1.025768821795 +  -1.040664085433*_Complex_I /* ( 0, 2) */,
    0.541217397816 +  -0.002345615451*_Complex_I /* ( 0, 3) */,
    0.000000000000 +   0.000000000000*_Complex_I /* ( 1, 0) */,
    2.273733268802 +   0.000000000000*_Complex_I /* ( 1, 1) */,
   -2.939502710322 +  -2.626579524510*_Complex_I /* ( 1, 2) */,
   -1.154743344912 +   0.323209860623*_Complex_I /* ( 1, 3) */,
    0.000000000000 +   0.000000000000*_Complex_I /* ( 2, 0) */,
    0.000000000000 +   0.000000000000*_Complex_I /* ( 2, 1) */,
    1.701364174878 +   0.000000000000*_Complex_I /* ( 2, 2) */,
    0.689923063328 +  -0.348316412767*_Complex_I /* ( 2, 3) */,
    0.000000000000 +   0.000000000000*_Complex_I /* ( 3, 0) */,
    0.000000000000 +   0.000000000000*_Complex_I /* ( 3, 1) */,
    0.000000000000 +   0.000000000000*_Complex_I /* ( 3, 2) */,
    2.392371328442 +   0.000000000000*_Complex_I /* ( 3, 3) */};

