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
// autotest fft data for 4-point transform
//

#include <complex.h>

float complex fft_test_x4[] = {
   -2.218920151449 +  -1.079004048069*_Complex_I,
    0.045264423484 +   0.426155393025*_Complex_I,
    0.218614474268 +  -0.334711618319*_Complex_I,
    2.182538230032 +   1.706944462070*_Complex_I};

float complex fft_test_y4[] = {
    0.227496976335 +   0.719384188708*_Complex_I,
   -3.718323694762 +   1.392981376798*_Complex_I,
   -4.228108330697 +  -3.546815521483*_Complex_I,
   -1.156745556672 +  -2.881566236299*_Complex_I};

