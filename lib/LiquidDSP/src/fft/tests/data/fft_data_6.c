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
// autotest fft data for 6-point transform
//

#include <complex.h>

float complex fft_test_x6[] = {
   -0.946868805918 +   0.048419613876*_Complex_I,
   -1.426556442325 +   1.356194807524*_Complex_I,
    0.262357323076 +   1.594616904796*_Complex_I,
   -1.032912520662 +   0.046391595464*_Complex_I,
   -0.271359734201 +  -2.390517158747*_Complex_I,
   -0.288151144041 +   0.071324517238*_Complex_I};

float complex fft_test_y6[] = {
   -3.703491324072 +   0.726430280150*_Complex_I,
    3.797148775593 +   1.637413185851*_Complex_I,
   -3.456423352393 +   1.227102112087*_Complex_I,
    1.791748889984 +  -2.221391560299*_Complex_I,
    1.220570696725 +  -1.669098764217*_Complex_I,
   -5.330766521347 +   0.590062429687*_Complex_I};

