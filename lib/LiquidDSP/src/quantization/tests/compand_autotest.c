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
#include "liquid.internal.h"

void autotest_compand_float() {
    float x = -1.0f;
    float mu=255.0f;
    unsigned int n=30;

    float dx = 2/(float)(n);
    float y;
    float x_hat;
    float tol = 1e-6f;

    unsigned int i;
    for (i=0; i<n; i++) {
        y = compress_mulaw(x,mu);
        x_hat = expand_mulaw(y,mu);

        if (liquid_autotest_verbose)
            printf("%8.4f -> %8.4f -> %8.4f\n", x, y, x_hat);

        CONTEND_DELTA(x,x_hat,tol);

        x += dx;
        x = (x > 1.0f) ? 1.0f : x;
    }
}


void autotest_compand_cfloat() {
    float complex x = -0.707f - 0.707f*_Complex_I;
    float mu=255.0f;
    unsigned int n=30;

    float complex dx = 2*(0.707f +0.707f* _Complex_I)/(float)(n);
    float complex y;
    float complex z;
    float tol = 1e-6f;

    unsigned int i;
    for (i=0; i<n; i++) {
        compress_cf_mulaw(x,mu,&y);
        expand_cf_mulaw(y,mu,&z);

        if (liquid_autotest_verbose) {
            printf("%8.4f +j%8.4f > ", crealf(x), cimagf(x));
            printf("%8.4f +j%8.4f > ", crealf(y), cimagf(y));
            printf("%8.4f +j%8.4f\n",  crealf(z), cimagf(z));
        }

        CONTEND_DELTA(crealf(x),crealf(z),tol);
        CONTEND_DELTA(cimagf(x),cimagf(z),tol);

        x += dx;
        //x = (x > 1.0f) ? 1.0f : x;
    }
}

