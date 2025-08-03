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
// autotest fft data for 5-point transform
//

#include <complex.h>

float complex fft_test_x5[] = {
    1.043452789296 +  -0.216675780077*_Complex_I,
   -0.039259154719 +  -0.756503590362*_Complex_I,
   -1.378329383804 +  -1.629692578129*_Complex_I,
    0.695728357044 +  -2.639675956000*_Complex_I,
   -0.019932891052 +   0.123958045411*_Complex_I};

float complex fft_test_y5[] = {
    0.301659716765 +  -5.118589859158*_Complex_I,
    1.333681830770 +   4.279329517647*_Complex_I,
   -0.597668794979 +  -2.985429553632*_Complex_I,
    2.358478480201 +   0.936943320049*_Complex_I,
    1.821112713724 +   1.804367674708*_Complex_I};

