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
// data for testing matrix multiplication
//

#include <complex.h>

// matrixcf_data_mul_x [size: 5 x 4]
float complex matrixcf_data_mul_x[] = {
    1.131277322769 +  -2.908640623093*_Complex_I /* ( 0, 0) */,
    0.847201466560 +  -1.637244105339*_Complex_I /* ( 0, 1) */,
   -2.173580169678 +   0.096817605197*_Complex_I /* ( 0, 2) */,
    0.792498826981 +  -0.358158409595*_Complex_I /* ( 0, 3) */,
   -0.243082717061 +   0.824095964432*_Complex_I /* ( 1, 0) */,
   -1.889652967453 +   0.283876717091*_Complex_I /* ( 1, 1) */,
    0.044418141246 +  -0.882465064526*_Complex_I /* ( 1, 2) */,
    0.515216410160 +  -0.366532146931*_Complex_I /* ( 1, 3) */,
    0.579283773899 +   1.173513293266*_Complex_I /* ( 2, 0) */,
    0.059265002608 +  -0.497733235359*_Complex_I /* ( 2, 1) */,
   -0.877321839333 +   0.404732406139*_Complex_I /* ( 2, 2) */,
   -0.794282734394 +   0.456011295319*_Complex_I /* ( 2, 3) */,
   -1.174414634705 +  -1.358565688133*_Complex_I /* ( 3, 0) */,
   -0.418152034283 +   1.380919337273*_Complex_I /* ( 3, 1) */,
   -0.747197151184 +   1.584241986275*_Complex_I /* ( 3, 2) */,
   -0.522293865681 +  -0.573823392391*_Complex_I /* ( 3, 3) */,
   -1.866284489632 +  -0.199214607477*_Complex_I /* ( 4, 0) */,
   -0.453905433416 +   0.452787637711*_Complex_I /* ( 4, 1) */,
    1.989426016808 +  -1.771166682243*_Complex_I /* ( 4, 2) */,
    2.234328985214 +   0.855401337147*_Complex_I /* ( 4, 3) */};

// matrixcf_data_mul_y [size: 4 x 3]
float complex matrixcf_data_mul_y[] = {
    0.122429788113 +  -1.041572093964*_Complex_I /* ( 0, 0) */,
   -1.123313307762 +  -1.396123170853*_Complex_I /* ( 0, 1) */,
   -0.318034142256 +  -0.537796914577*_Complex_I /* ( 0, 2) */,
    0.096901215613 +  -0.035752061754*_Complex_I /* ( 1, 0) */,
    0.423960685730 +  -0.379842221737*_Complex_I /* ( 1, 1) */,
   -0.184147700667 +   0.022100308910*_Complex_I /* ( 1, 2) */,
    0.189968794584 +   0.919595599174*_Complex_I /* ( 2, 0) */,
    0.621766507626 +  -0.634516119957*_Complex_I /* ( 2, 1) */,
    0.605251312256 +   1.410223841667*_Complex_I /* ( 2, 2) */,
    0.427330523729 +   0.042397715151*_Complex_I /* ( 3, 0) */,
    0.204851210117 +   0.611065924168*_Complex_I /* ( 3, 1) */,
    0.562124013901 +   0.047597970814*_Complex_I /* ( 3, 2) */};

// matrixcf_data_mul_z [size: 5 x 3]
float complex matrixcf_data_mul_z[] = {
   -3.015598273252 +  -3.823225604286*_Complex_I /* ( 0, 0) */,
   -6.503138041472 +   2.522251659946*_Complex_I /* ( 0, 1) */,
   -3.033435877267 +  -2.533375977709*_Complex_I /* ( 0, 2) */,
    1.711291176504 +   0.187568584413*_Complex_I /* ( 1, 0) */,
    0.527484730969 +  -0.085346610822*_Complex_I /* ( 1, 1) */,
    2.440625470928 +  -0.878385559540*_Complex_I /* ( 1, 2) */,
    0.383559143593 +  -1.078745633782*_Complex_I /* ( 2, 0) */,
    0.093675017974 +  -1.944126015771*_Complex_I /* ( 2, 1) */,
   -1.122987739839 +  -1.365514815630*_Complex_I /* ( 2, 2) */,
   -3.347645581625 +   0.552152171890*_Complex_I /* ( 3, 0) */,
    0.554058303745 +   4.932442551750*_Complex_I /* ( 3, 1) */,
   -3.263304464031 +   0.357861697730*_Complex_I /* ( 3, 2) */,
    2.461434774758 +   3.932854324787*_Complex_I /* ( 4, 0) */,
    1.845966920717 +   2.370697350446*_Complex_I /* ( 4, 1) */,
    5.477082880684 +   3.294354034834*_Complex_I /* ( 4, 2) */};

