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
// data for testing conjugate gradient solver
//

#include <complex.h>

// matrixf_data_cgsolve_A [size: 8 x 8]
float matrixf_data_cgsolve_A[] = {
   12.722920400000 /* ( 0, 0) */,
    0.000000000000 /* ( 0, 1) */,
   -7.952912300000 /* ( 0, 2) */,
    0.000000000000 /* ( 0, 3) */,
    4.111499800000 /* ( 0, 4) */,
    0.000000000000 /* ( 0, 5) */,
    0.000000000000 /* ( 0, 6) */,
    0.000000000000 /* ( 0, 7) */,
    0.000000000000 /* ( 1, 0) */,
    0.065151200000 /* ( 1, 1) */,
    0.000000000000 /* ( 1, 2) */,
   -0.218259800000 /* ( 1, 3) */,
    0.000000000000 /* ( 1, 4) */,
    0.000000000000 /* ( 1, 5) */,
    0.000000000000 /* ( 1, 6) */,
    0.000000000000 /* ( 1, 7) */,
   -7.952912300000 /* ( 2, 0) */,
    0.000000000000 /* ( 2, 1) */,
    5.031585200000 /* ( 2, 2) */,
    0.000000000000 /* ( 2, 3) */,
   -2.570038800000 /* ( 2, 4) */,
   -0.110545700000 /* ( 2, 5) */,
    0.000000000000 /* ( 2, 6) */,
    0.000000000000 /* ( 2, 7) */,
    0.000000000000 /* ( 3, 0) */,
   -0.218259800000 /* ( 3, 1) */,
    0.000000000000 /* ( 3, 2) */,
    0.733045600000 /* ( 3, 3) */,
    0.000000000000 /* ( 3, 4) */,
    0.000000000000 /* ( 3, 5) */,
    0.000000000000 /* ( 3, 6) */,
    0.000000000000 /* ( 3, 7) */,
    4.111499800000 /* ( 4, 0) */,
    0.000000000000 /* ( 4, 1) */,
   -2.570038800000 /* ( 4, 2) */,
    0.000000000000 /* ( 4, 3) */,
    1.338132900000 /* ( 4, 4) */,
    0.239381000000 /* ( 4, 5) */,
    0.078430200000 /* ( 4, 6) */,
    0.000000000000 /* ( 4, 7) */,
    0.000000000000 /* ( 5, 0) */,
    0.000000000000 /* ( 5, 1) */,
   -0.110545700000 /* ( 5, 2) */,
    0.000000000000 /* ( 5, 3) */,
    0.239381000000 /* ( 5, 4) */,
    7.472388300000 /* ( 5, 5) */,
    1.981894700000 /* ( 5, 6) */,
   -1.373365000000 /* ( 5, 7) */,
    0.000000000000 /* ( 6, 0) */,
    0.000000000000 /* ( 6, 1) */,
    0.000000000000 /* ( 6, 2) */,
    0.000000000000 /* ( 6, 3) */,
    0.078430200000 /* ( 6, 4) */,
    1.981894700000 /* ( 6, 5) */,
    3.489272600000 /* ( 6, 6) */,
    0.000000000000 /* ( 6, 7) */,
    0.000000000000 /* ( 7, 0) */,
    0.000000000000 /* ( 7, 1) */,
    0.000000000000 /* ( 7, 2) */,
    0.000000000000 /* ( 7, 3) */,
    0.000000000000 /* ( 7, 4) */,
   -1.373365000000 /* ( 7, 5) */,
    0.000000000000 /* ( 7, 6) */,
    2.063114900000 /* ( 7, 7) */};

// matrixf_data_cgsolve_x [size: 8 x 1]
float matrixf_data_cgsolve_x[] = {
    0.162052200000 /* ( 0, 0) */,
   -0.012720300000 /* ( 1, 0) */,
    1.043375100000 /* ( 2, 0) */,
    0.006205200000 /* ( 3, 0) */,
    0.878157000000 /* ( 4, 0) */,
    0.146412900000 /* ( 5, 0) */,
    0.782585200000 /* ( 6, 0) */,
   -0.825784600000 /* ( 7, 0) */};

// matrixf_data_cgsolve_b [size: 8 x 1]
float matrixf_data_cgsolve_b[] = {
   -2.625551095190 /* ( 0, 0) */,
   -0.002183088520 /* ( 1, 0) */,
    1.687960897575 /* ( 2, 0) */,
    0.007325024691 /* ( 3, 0) */,
   -0.743719348831 /* ( 4, 0) */,
    3.874032638311 /* ( 5, 0) */,
    3.089702075189 /* ( 6, 0) */,
   -1.904766864859 /* ( 7, 0) */};

