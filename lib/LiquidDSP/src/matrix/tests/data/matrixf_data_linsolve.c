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
// data for testing linear solver
//

#include <complex.h>

// matrixf_data_linsolve_A [size: 5 x 5]
float matrixf_data_linsolve_A[] = {
    0.359868824482 /* ( 0, 0) */,
   -0.821193814278 /* ( 0, 1) */,
   -0.267460018396 /* ( 0, 2) */,
    0.886115014553 /* ( 0, 3) */,
    0.153591111302 /* ( 0, 4) */,
    0.298885852098 /* ( 1, 0) */,
   -1.239024162292 /* ( 1, 1) */,
   -0.948822617531 /* ( 1, 2) */,
   -0.779868483543 /* ( 1, 3) */,
   -0.334943383932 /* ( 1, 4) */,
   -0.071195065975 /* ( 2, 0) */,
    0.763968944550 /* ( 2, 1) */,
    0.294695496559 /* ( 2, 2) */,
    0.060610540211 /* ( 2, 3) */,
    0.016189640388 /* ( 2, 4) */,
    1.150504231453 /* ( 3, 0) */,
   -0.605459213257 /* ( 3, 1) */,
    0.055004067719 /* ( 3, 2) */,
    1.185544967651 /* ( 3, 3) */,
    0.555612862110 /* ( 3, 4) */,
    1.054118633270 /* ( 4, 0) */,
   -0.494105964899 /* ( 4, 1) */,
   -0.824876368046 /* ( 4, 2) */,
    0.667240202427 /* ( 4, 3) */,
    1.367745161057 /* ( 4, 4) */};

// matrixf_data_linsolve_x [size: 5 x 1]
float matrixf_data_linsolve_x[] = {
   -0.848446607590 /* ( 0, 0) */,
    1.041861057281 /* ( 1, 0) */,
    0.453321367502 /* ( 2, 0) */,
   -0.949143886566 /* ( 3, 0) */,
    1.200846910477 /* ( 4, 0) */};

// matrixf_data_linsolve_b [size: 5 x 1]
float matrixf_data_linsolve_b[] = {
   -1.938755917550 /* ( 0, 0) */,
   -1.636609601788 /* ( 1, 0) */,
    0.951859625938 /* ( 2, 0) */,
   -2.040058038471 /* ( 3, 0) */,
   -0.773941632607 /* ( 4, 0) */};

