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

#include "autotest/autotest.h"
#include "liquid.h"

void autotest_fft_shift_4()
{
    float complex x[] = {
        0 + 0*_Complex_I,
        1 + 1*_Complex_I,
        2 + 2*_Complex_I,
        3 + 3*_Complex_I
    };

    float complex test[] = {
        2 + 2*_Complex_I,
        3 + 3*_Complex_I,
        0 + 0*_Complex_I,
        1 + 1*_Complex_I
    };

    fft_shift(x,4);

    CONTEND_SAME_DATA(x,test,4*sizeof(float complex));
}

void autotest_fft_shift_8()
{
    float complex x[] = {
        0 + 0*_Complex_I,
        1 + 1*_Complex_I,
        2 + 2*_Complex_I,
        3 + 3*_Complex_I,
        4 + 4*_Complex_I,
        5 + 5*_Complex_I,
        6 + 6*_Complex_I,
        7 + 7*_Complex_I
    };

    float complex test[] = {
        4 + 4*_Complex_I,
        5 + 5*_Complex_I,
        6 + 6*_Complex_I,
        7 + 7*_Complex_I,
        0 + 0*_Complex_I,
        1 + 1*_Complex_I,
        2 + 2*_Complex_I,
        3 + 3*_Complex_I
    };

    fft_shift(x,8);

    CONTEND_SAME_DATA(x,test,8*sizeof(float complex));
}

