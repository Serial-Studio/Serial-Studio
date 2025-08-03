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
// autotest fft data for 7-point transform
//

#include <complex.h>

float complex fft_test_x7[] = {
    0.325737557343 +   0.347762560645*_Complex_I,
   -0.464568614672 +   1.344201995758*_Complex_I,
   -1.458140194879 +   0.983317270098*_Complex_I,
    1.679041515327 +   1.025013762005*_Complex_I,
   -0.178483024495 +  -0.711524629930*_Complex_I,
    0.986194459374 +  -1.709315563086*_Complex_I,
    0.387998802736 +  -1.150726066104*_Complex_I};

float complex fft_test_y7[] = {
    1.277780500734 +   0.128729329387*_Complex_I,
    4.360250363806 +   2.591163135631*_Complex_I,
    1.609972293897 +   2.377175130550*_Complex_I,
    0.436888889637 +  -3.701058823864*_Complex_I,
   -0.903757801309 +   3.003131513942*_Complex_I,
    1.797162255231 +  -0.068636624441*_Complex_I,
   -6.298133600593 +  -1.896165736688*_Complex_I};

