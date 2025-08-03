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

// matrixcf_data_linsolve_A [size: 5 x 5]
float complex matrixcf_data_linsolve_A[] = {
   -0.482884645462 +  -0.221723198891*_Complex_I /* ( 0, 0) */,
   -0.387645065784 +   0.086682170630*_Complex_I /* ( 0, 1) */,
    1.580931067467 +   0.883717715740*_Complex_I /* ( 0, 2) */,
    1.570333838463 +   1.783135294914*_Complex_I /* ( 0, 3) */,
   -1.081483244896 +  -0.691094517708*_Complex_I /* ( 0, 4) */,
    0.248138338327 +  -0.250954031944*_Complex_I /* ( 1, 0) */,
    0.790891706944 +  -0.313775628805*_Complex_I /* ( 1, 1) */,
   -0.146090522408 +  -1.320674061775*_Complex_I /* ( 1, 2) */,
    0.672296106815 +   1.346951484680*_Complex_I /* ( 1, 3) */,
   -0.352442741394 +   0.056975554675*_Complex_I /* ( 1, 4) */,
    0.707973957062 +  -0.069402769208*_Complex_I /* ( 2, 0) */,
   -0.894841134548 +  -1.854133605957*_Complex_I /* ( 2, 1) */,
    0.397095054388 +  -0.924011290073*_Complex_I /* ( 2, 2) */,
    0.054669041187 +   0.017023870721*_Complex_I /* ( 2, 3) */,
    0.515784740448 +  -0.455956429243*_Complex_I /* ( 2, 4) */,
    0.570774257183 +   0.538610219955*_Complex_I /* ( 3, 0) */,
   -0.389531791210 +   0.200702637434*_Complex_I /* ( 3, 1) */,
    0.159817531705 +   1.283960223198*_Complex_I /* ( 3, 2) */,
    1.571215510368 +   0.574963092804*_Complex_I /* ( 3, 3) */,
   -2.452192783356 +  -0.583715677261*_Complex_I /* ( 3, 4) */,
   -0.603657603264 +   0.617622077465*_Complex_I /* ( 4, 0) */,
    0.935181498528 +   0.949800848961*_Complex_I /* ( 4, 1) */,
    0.043205004185 +   1.351160168648*_Complex_I /* ( 4, 2) */,
    0.674502849579 +   0.340750336647*_Complex_I /* ( 4, 3) */,
   -0.241452947259 +   1.540177464485*_Complex_I /* ( 4, 4) */};

// matrixcf_data_linsolve_x [size: 5 x 1]
float complex matrixcf_data_linsolve_x[] = {
   -0.686784207821 +   0.516409814358*_Complex_I /* ( 0, 0) */,
    0.725918948650 +  -0.725804686546*_Complex_I /* ( 1, 0) */,
    0.048043362796 +   1.415739893913*_Complex_I /* ( 2, 0) */,
    1.184294700623 +  -1.108955144882*_Complex_I /* ( 3, 0) */,
    1.000079274178 +   0.117630988359*_Complex_I /* ( 4, 0) */};

// matrixcf_data_linsolve_b [size: 5 x 1]
float complex matrixcf_data_linsolve_b[] = {
    1.889372086452 +   2.079795053851*_Complex_I /* ( 0, 0) */,
    4.099006087145 +   0.093571115573*_Complex_I /* ( 1, 0) */,
   -0.465385431770 +  -0.201195243205*_Complex_I /* ( 2, 0) */,
   -2.502649126311 +  -1.292489487343*_Complex_I /* ( 3, 0) */,
    0.307098947642 +   0.568345470088*_Complex_I /* ( 4, 0) */};

