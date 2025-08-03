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

//
// AUTOTEST : fir group delay, n=3
//
void autotest_fir_groupdelay_n3()
{
    // create coefficients array
    float h[3] = {0.1, 0.2, 0.4};

    float tol = 1e-3f;
    unsigned int i;

    // create testing vectors
    float fc[4] = { 0.000,
                    0.125,
                    0.250,
                    0.375};
    
    float g0[4] = { 1.42857142857143,
                    1.54756605839643,
                    2.15384615384615,
                    2.56861651421767};


    // run tests
    float g;
    for (i=0; i<4; i++) {
        g = fir_group_delay(h, 3, fc[i]);
        CONTEND_DELTA( g, g0[i], tol );
    }

    // create filter
    firfilt_rrrf filter = firfilt_rrrf_create(h,3);

    // run tests again
    for (i=0; i<4; i++) {
        g = firfilt_rrrf_groupdelay(filter, fc[i]);
        CONTEND_DELTA( g, g0[i], tol );
    }

    // destroy filter
    firfilt_rrrf_destroy(filter);
}


//
// AUTOTEST : iir group delay, n=3
//
void autotest_iir_groupdelay_n3()
{
    // create coefficients array
    float b[3] = {0.20657210,  0.41314420, 0.20657210};
    float a[3] = {1.00000000, -0.36952737, 0.19581573};

    float tol = 1e-3f;
    unsigned int i;

    // create testing vectors
    float fc[4] = { 0.000,
                    0.125,
                    0.250,
                    0.375};
    
    float g0[4] = { 0.973248939389634,
                    1.366481121240365,
                    1.227756735863196,
                    0.651058521306726};

    // run tests
    float g;
    for (i=0; i<4; i++) {
        g = iir_group_delay(b, 3, a, 3, fc[i]);
        CONTEND_DELTA( g, g0[i], tol );
    }

    // create filter
    iirfilt_rrrf filter = iirfilt_rrrf_create(b,3,a,3);

    // run tests again
    for (i=0; i<4; i++) {
        g = iirfilt_rrrf_groupdelay(filter, fc[i]);
        CONTEND_DELTA( g, g0[i], tol );
    }

    // destroy filter
    iirfilt_rrrf_destroy(filter);
}


//
// AUTOTEST : iir group delay, n=8
//
void autotest_iir_groupdelay_n8()
{
    // create coefficients arrays (7th-order Butterworth)
    float b[8];
    b[  0] =   0.00484212;
    b[  1] =   0.03389481;
    b[  2] =   0.10168444;
    b[  3] =   0.16947407;
    b[  4] =   0.16947407;
    b[  5] =   0.10168444;
    b[  6] =   0.03389481;
    b[  7] =   0.00484212;

    float a[8];
    a[  0] =   1.00000000;
    a[  1] =  -1.38928008;
    a[  2] =   1.67502367;
    a[  3] =  -1.05389738;
    a[  4] =   0.50855154;
    a[  5] =  -0.14482945;
    a[  6] =   0.02625222;
    a[  7] =  -0.00202968;

    float tol = 1e-3f;
    unsigned int i;

    // create testing vectors
    float fc[7] = { 0.00000,
                    0.06250,
                    0.12500,
                    0.18750,
                    0.25000,
                    0.31250,
                    0.37500};

    float g0[7] = { 3.09280801068444,
                    3.30599360247944,
                    4.18341028373046,
                    7.71934054380586,
                    4.34330109915390,
                    2.60203085226210,
                    1.97868452107144};

    // run tests
    float g;
    for (i=0; i<7; i++) {
        g = iir_group_delay(b, 8, a, 8, fc[i]);
        CONTEND_DELTA( g, g0[i], tol );
    }

    //
    // test with iir filter (tf)
    //

    // create filter
    iirfilt_rrrf filter = iirfilt_rrrf_create(b,8,a,8);

    // run tests again
    for (i=0; i<7; i++) {
        g = iirfilt_rrrf_groupdelay(filter, fc[i]);
        CONTEND_DELTA( g, g0[i], tol );
    }

    // destroy filter
    iirfilt_rrrf_destroy(filter);
}


//
// AUTOTEST : iir group delay (second-order sections), n=8
//
void autotest_iir_groupdelay_sos_n8()
{
    // create coefficients arrays (7th-order Butterworth)
    float B[12] = {
        0.00484212,   0.00968423,   0.00484212,
        1.00000000,   2.00000000,   1.00000000,
        1.00000000,   2.00000000,   1.00000000,
        1.00000000,   1.00000000,   0.00000000};

    float A[12] = {
        1.00000000,  -0.33283597,   0.07707999,
        1.00000000,  -0.38797498,   0.25551325,
        1.00000000,  -0.51008475,   0.65066898,
        1.00000000,  -0.15838444,   0.00000000};

    float tol = 1e-3f;
    unsigned int i;

    // create testing vectors
    float fc[7] = { 0.00000,
                    0.06250,
                    0.12500,
                    0.18750,
                    0.25000,
                    0.31250,
                    0.37500};

    float g0[7] = { 3.09280801068444,
                    3.30599360247944,
                    4.18341028373046,
                    7.71934054380586,
                    4.34330109915390,
                    2.60203085226210,
                    1.97868452107144};

    //
    // test with iir filter (second-order sections)
    //

    // create filter
    iirfilt_rrrf filter = iirfilt_rrrf_create_sos(B,A,4);

    // run tests
    float g;
    for (i=0; i<7; i++) {
        g = iirfilt_rrrf_groupdelay(filter, fc[i]);
        CONTEND_DELTA( g, g0[i], tol );
    }

    // destroy filter
    iirfilt_rrrf_destroy(filter);
}

