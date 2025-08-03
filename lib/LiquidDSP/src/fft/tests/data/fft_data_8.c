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
// autotest fft data for 8-point transform
//

#include <complex.h>

float complex fft_test_x8[] = {
    1.143832659273 +   0.058730029889*_Complex_I,
   -0.094390429919 +   0.229144161540*_Complex_I,
   -0.231936945111 +   0.250418514706*_Complex_I,
    0.180568135767 +  -0.869698396678*_Complex_I,
   -0.345282052584 +   1.176003338020*_Complex_I,
    0.544428216952 +  -0.610473584454*_Complex_I,
    0.928035714223 +   0.647778401795*_Complex_I,
    0.441211141066 +  -1.176622015089*_Complex_I};

float complex fft_test_y8[] = {
    2.566466439667 +  -0.294719550271*_Complex_I,
    1.635071437815 +   1.055386414782*_Complex_I,
    1.767442826430 +   0.508277941207*_Complex_I,
    2.964612333261 +  -2.017902163711*_Complex_I,
    0.422832311935 +   4.560580119089*_Complex_I,
    0.548438211721 +  -0.969987712376*_Complex_I,
   -1.562539151277 +   0.164794961607*_Complex_I,
    0.808336864628 +  -2.536589771219*_Complex_I};

