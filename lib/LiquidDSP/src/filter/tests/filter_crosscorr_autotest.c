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

void autotest_filter_crosscorr_rrrf()
{
    // options
    float tol = 1e-3f;

    // input vectors
    int x_len = 16;
    float x[16] = {
          0.25887000,   0.11752000,   0.67812000,  -1.02480000, 
          1.46750000,  -0.67462000,   0.93029000,   0.98751000, 
          0.00969890,   1.05300000,   1.38100000,   1.47540000, 
          1.14110000,  -0.39480000,  -0.30426000,   1.58190000 };

    int y_len = 8;
    float y[8] = {
         -1.15920000,  -1.57390000,   0.65239000,  -0.54542000, 
         -0.97277000,   0.99115000,  -0.76247000,  -1.08210000 };


    // derived values
    unsigned int rxy_len = x_len + y_len - 1;
    float rxy[23];
    float rxy_test[23] = {
         -0.28013000,  -0.32455000,  -0.56685000,   0.45660000, 
         -0.39008000,  -1.95950000,   1.25850000,  -3.35780000, 
         -1.85760000,   1.07920000,  -5.31760000,  -2.18630000, 
         -2.05850000,  -3.52450000,  -0.90010000,  -4.55350000, 
         -4.17770000,  -1.09920000,  -5.13670000,  -1.76270000, 
          1.96850000,  -2.13700000,  -1.83370000};


    if (liquid_autotest_verbose)
        printf("testing corr(x,y):\n");

    // corr(x,y)
    int i;
    for (i=0; i<rxy_len; i++) {
        int lag = i - y_len + 1;
        rxy[i] = liquid_filter_crosscorr(x,x_len, y,y_len, lag);

        // print results
        if (liquid_autotest_verbose)
            printf("  rxy(%3d) = %12.8f (expected %12.8f, e=%12.4e)\n", lag, rxy[i], rxy_test[i], rxy[i]-rxy_test[i]);
    }
    for (i=0; i<rxy_len; i++)
        CONTEND_DELTA( rxy[i], rxy_test[i], tol );


    // derived values
    unsigned int ryx_len = x_len + y_len - 1;
    float ryx[23];
    float ryx_test[23] = {
         -1.83370000,  -2.13700000,   1.96850000,  -1.76270000, 
         -5.13670000,  -1.09920000,  -4.17770000,  -4.55350000, 
         -0.90010000,  -3.52450000,  -2.05850000,  -2.18630000, 
         -5.31760000,   1.07920000,  -1.85760000,  -3.35780000, 
          1.25850000,  -1.95950000,  -0.39008000,   0.45660000, 
         -0.56685000,  -0.32455000,  -0.28013000};
        
    if (liquid_autotest_verbose)
        printf("testing corr(y,x):\n");

    // corr(y,x)
    for (i=0; i<ryx_len; i++) {
        int lag = i - x_len + 1;
        ryx[i] = liquid_filter_crosscorr(y,y_len, x,x_len, lag);

        // print results
        if (liquid_autotest_verbose)
            printf("  ryx(%3d) = %12.8f (expected %12.8f, e=%12.4e)\n", lag, ryx[i], ryx_test[i], ryx[i]-ryx_test[i]);
    }
    for (i=0; i<ryx_len; i++)
        CONTEND_DELTA( ryx[i], ryx_test[i], tol );
}

