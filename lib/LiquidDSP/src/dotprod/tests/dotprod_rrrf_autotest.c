/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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

#include <string.h>

#include "autotest/autotest.h"
#include "liquid.internal.h"

// 
// AUTOTEST: basic dot product
//
void autotest_dotprod_rrrf_basic()
{
    float tol = 1e-6;   // error tolerance
    float y;            // return value

    float h[16] = {
        1, -1, 1, -1, 1, -1, 1, -1,
        1, -1, 1, -1, 1, -1, 1, -1};

    float x0[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float test0 = 0;
    dotprod_rrrf_run(h,x0,16,&y);
    CONTEND_DELTA(y,  test0, tol);
    dotprod_rrrf_run4(h,x0,16,&y);
    CONTEND_DELTA(y,  test0, tol);

    float x1[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    float test1 = 0;
    dotprod_rrrf_run(h,x1,16,&y);
    CONTEND_DELTA(y,  test1, tol);
    dotprod_rrrf_run4(h,x1,16,&y);
    CONTEND_DELTA(y,  test1, tol);

    float x2[16] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
    float test2 = -8;
    dotprod_rrrf_run(h,x2,16,&y);
    CONTEND_DELTA(y,  test2, tol);
    dotprod_rrrf_run4(h,x2,16,&y);
    CONTEND_DELTA(y,  test2, tol);

    float x3[16] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
    float test3 = 8;
    dotprod_rrrf_run(h,x3,16,&y);
    CONTEND_DELTA(y,  test3, tol);
    dotprod_rrrf_run4(h,x3,16,&y);
    CONTEND_DELTA(y,  test3, tol);

    float test4 = 16;
    dotprod_rrrf_run(h,h,16,&y);
    CONTEND_DELTA(y,  test4, tol);
    dotprod_rrrf_run4(h,h,16,&y);
    CONTEND_DELTA(y,  test4, tol);

}

// 
// AUTOTEST: uneven dot product
//
void autotest_dotprod_rrrf_uneven()
{
    float tol = 1e-6;
    float y;

    float h[16] = {
        1, -1, 1, -1, 1, -1, 1, -1,
        1, -1, 1, -1, 1, -1, 1, -1};

    float x[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

    float test1 = 1;
    dotprod_rrrf_run(h,x,1,&y);
    CONTEND_DELTA(y,  test1, tol);

    float test2 = 0;
    dotprod_rrrf_run(h,x,2,&y);
    CONTEND_DELTA(y, test2, tol);

    float test3 = 1;
    dotprod_rrrf_run(h,x,3,&y);
    CONTEND_DELTA(y, test3, tol);

    float test11 = 1;
    dotprod_rrrf_run(h,x,11,&y);
    CONTEND_DELTA(y, test11, tol);

    float test13 = 1;
    dotprod_rrrf_run(h,x,13,&y);
    CONTEND_DELTA(y, test13, tol);

    float test15 = 1;
    dotprod_rrrf_run(h,x,15,&y);
    CONTEND_DELTA(y, test15, tol);

}

// 
// AUTOTEST: structured dot product
//
void autotest_dotprod_rrrf_struct()
{
    float tol = 1e-6;
    float y;

    float h[16] = {
        1, -1, 1, -1, 1, -1, 1, -1,
        1, -1, 1, -1, 1, -1, 1, -1};

    // create object
    dotprod_rrrf dp = dotprod_rrrf_create(h,16);

    float x0[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float test0 = 0;
    dotprod_rrrf_execute(dp,x0,&y);
    CONTEND_DELTA(y,  test0, tol);

    float x1[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    float test1 = 0;
    dotprod_rrrf_execute(dp,x1,&y);
    CONTEND_DELTA(y,  test1, tol);

    float x2[16] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
    float test2 = -8;
    dotprod_rrrf_execute(dp,x2,&y);
    CONTEND_DELTA(y,  test2, tol);

    float *x3 = h;
    float test3 = 16;
    dotprod_rrrf_execute(dp,x3,&y);
    CONTEND_DELTA(y,  test3, tol);

    // clean-up
    dotprod_rrrf_destroy(dp);
}

// 
// AUTOTEST: structured dot product with floating-point data
//
void autotest_dotprod_rrrf_struct_align()
{
    float h[16] = {
    -0.050565, -0.952580,  0.274320,  1.232400, 
     1.268200,  0.565770,  0.800830,  0.923970, 
     0.517060, -0.530340, -0.378550, -1.127100, 
     1.123100, -1.006000, -1.483800, -0.062007
    };

    float x[16] = {
    -0.384280, -0.812030,  0.156930,  1.919500, 
     0.564580, -0.123610, -0.138640,  0.004984, 
    -1.100200, -0.497620,  0.089977, -1.745500, 
     0.463640,  0.592100,  1.150000, -1.225400
    };

    float test = 3.66411513609863;
    float tol = 1e-3f;
    float y;

    // create dotprod object
    dotprod_rrrf dp = dotprod_rrrf_create(h,16);

    // test data misalignment conditions
    float x_buffer[20];
    float * x_hat;
    unsigned int i;
    for (i=0; i<4; i++) {
        // set pointer to array aligned with counter
        x_hat = &x_buffer[i];

        // copy input data to buffer
        memmove(x_hat, x, 16*sizeof(float));
        
        // execute dotprod
        dotprod_rrrf_execute(dp,x_hat,&y);
        CONTEND_DELTA(y,test,tol);
    }

    // destroy dotprod object
    dotprod_rrrf_destroy(dp);
}


// 
// AUTOTEST: dot product with floating-point data
//
void autotest_dotprod_rrrf_rand01()
{
    float h[16] = {
    -0.050565, -0.952580,  0.274320,  1.232400, 
     1.268200,  0.565770,  0.800830,  0.923970, 
     0.517060, -0.530340, -0.378550, -1.127100, 
     1.123100, -1.006000, -1.483800, -0.062007
    };

    float x[16] = {
    -0.384280, -0.812030,  0.156930,  1.919500, 
     0.564580, -0.123610, -0.138640,  0.004984, 
    -1.100200, -0.497620,  0.089977, -1.745500, 
     0.463640,  0.592100,  1.150000, -1.225400
    };

    float test = 3.66411513609863;
    float tol = 1e-3f;
    float y;

    dotprod_rrrf_run(h,x,16,&y);
    CONTEND_DELTA(y,test,tol);
}

// 
// AUTOTEST: dot product with floating-point data
//
void autotest_dotprod_rrrf_rand02()
{
    float h[16] = {
     2.595300,  1.243600, -0.818550, -1.439800, 
     0.055795, -1.476000,  0.445900,  0.325460, 
    -3.451200,  0.058528, -0.246990,  0.476290, 
    -0.598780, -0.885250,  0.464660, -0.610140
    };

    float x[16] = {
    -0.917010, -1.278200, -0.533190,  2.309200, 
     0.592980,  0.964820,  0.183220, -0.082864, 
     0.057171, -1.186500, -0.738260,  0.356960, 
    -0.144000, -1.435200, -0.893420,  1.657800
    };

    float test     = -8.17832326680587;
    float test_rev =  4.56839328512000;
    float tol = 1e-3f;
    float y;

    dotprod_rrrf_run(h,x,16,&y);
    CONTEND_DELTA(y,test,tol);

    // create object
    dotprod_rrrf q = dotprod_rrrf_create(h,16);
    dotprod_rrrf_execute(q,x,&y);
    CONTEND_DELTA(y,test,tol);

    // test running in reverse
    q = dotprod_rrrf_recreate_rev(q,h,16);
    dotprod_rrrf_execute(q,x,&y);
    CONTEND_DELTA(y,test_rev,tol);

    // create original again
    q = dotprod_rrrf_recreate(q,h,16);
    dotprod_rrrf_execute(q,x,&y);
    CONTEND_DELTA(y,test,tol);

    // clean it up
    dotprod_rrrf_destroy(q);
}

// 
// AUTOTEST: structured dot product, odd lengths
//
void autotest_dotprod_rrrf_struct_lengths()
{
    float tol = 2e-6;
    float y;

    float x[35] = {
         0.03117498,  -1.54311769,  -0.58759073,  -0.73882202, 
         0.86592259,  -0.26669417,  -0.70153724,  -1.24555787, 
        -1.09272288,  -1.41984975,  -1.40299260,   0.95861481, 
        -0.67361246,   2.05305710,   1.26576873,  -0.77474848, 
        -0.93143252,  -1.05724660,   0.21455006,   1.07554168, 
        -0.46703810,   0.68878404,  -1.11900266,  -0.52016966, 
         0.61400744,  -0.46506142,  -0.16801031,   0.48237303, 
         0.51286055,  -0.57239385,  -0.64462740,  -0.75596668, 
         1.95612355,  -0.47917908,   0.52384983, };
    float h[35] = {
        -0.12380948,   0.88417134,   2.27373797,  -2.61506417, 
         0.35022002,   0.07481393,   0.52984228,  -0.65542307, 
        -2.14893606,   0.62466395,   0.07330391,  -1.28014856, 
         0.16347776,   0.21238151,   0.05462232,  -0.60290942, 
        -1.27658956,   3.05114996,   1.34789601,  -1.22098592, 
         1.70899633,  -0.41002037,   3.08009931,  -1.39895771, 
        -0.50875066,   0.25817865,   1.08668549,   0.05494174, 
        -1.05337166,   1.26772604,   1.00369204,  -0.55129338, 
         1.01828299,   0.76014664,  -0.15605569, };

    float v32 = -7.99577847f;
    float v33 = -6.00389114f;
    float v34 = -6.36813751f;
    float v35 = -6.44988725f;

    // 
    dotprod_rrrf dp;

    // n = 32
    dp = dotprod_rrrf_create(h,32);
    dotprod_rrrf_execute(dp, x, &y);
    CONTEND_DELTA(y, v32, tol);
    dotprod_rrrf_destroy(dp);
    if (liquid_autotest_verbose)
        printf("  dotprod-rrrf-32 : %12.8f (expected %12.8f)\n", y, v32);

    // n = 33
    dp = dotprod_rrrf_create(h,33);
    dotprod_rrrf_execute(dp, x, &y);
    CONTEND_DELTA(y, v33, tol);
    dotprod_rrrf_destroy(dp);
    if (liquid_autotest_verbose)
        printf("  dotprod-rrrf-33 : %12.8f (expected %12.8f)\n", y, v33);

    // n = 34
    dp = dotprod_rrrf_create(h,34);
    dotprod_rrrf_execute(dp, x, &y);
    CONTEND_DELTA(y, v34, tol);
    dotprod_rrrf_destroy(dp);
    if (liquid_autotest_verbose)
        printf("  dotprod-rrrf-34 : %12.8f (expected %12.8f)\n", y, v34);

    // n = 35
    dp = dotprod_rrrf_create(h,35);
    dotprod_rrrf_execute(dp, x, &y);
    CONTEND_DELTA(y, v35, tol);
    dotprod_rrrf_destroy(dp);
    if (liquid_autotest_verbose)
        printf("  dotprod-rrrf-35 : %12.8f (expected %12.8f)\n", y, v35);
}

// 
// AUTOTEST: compare structured result to ordinal computation
//

// helper function (compare structured object to ordinal computation)
void runtest_dotprod_rrrf(unsigned int _n)
{
    float tol = 1e-4;
    float h[_n];
    float x[_n];

    // generate random coefficients
    unsigned int i;
    for (i=0; i<_n; i++) {
        h[i] = randnf();
        x[i] = randnf();
    }
    
    // compute expected value (ordinal computation)
    float y_test=0;
    for (i=0; i<_n; i++)
        y_test += h[i] * x[i];

    // create and run dot product object
    float y_struct;
    dotprod_rrrf dp;
    dp = dotprod_rrrf_create(h,_n);
    dotprod_rrrf_execute(dp, x, &y_struct);
    dotprod_rrrf_destroy(dp);

    // run unstructured
    float y_run, y_run4;
    dotprod_rrrf_run (h,x,_n,&y_run );
    dotprod_rrrf_run4(h,x,_n,&y_run4);

    // print results
    if (liquid_autotest_verbose) {
        printf("  dotprod-rrrf-%-4u(struct) : %12.8f (expected %12.8f)\n", _n, y_struct, y_test);
        printf("  dotprod-rrrf-%-4u(run   ) : %12.8f (expected %12.8f)\n", _n, y_run,    y_test);
        printf("  dotprod-rrrf-%-4u(run4  ) : %12.8f (expected %12.8f)\n", _n, y_run4,   y_test);
    }

    // validate result (structured object)
    CONTEND_DELTA(y_struct, y_test, tol);

    // validate result (unstructured, run)
    CONTEND_DELTA(y_run, y_test, tol);

    // validate result (unstructured, run4)
    CONTEND_DELTA(y_run4, y_test, tol);
}

// compare structured object to ordinal computation
void autotest_dotprod_rrrf_struct_vs_ordinal()
{
    // run many, many tests
    unsigned int i;
    for (i=1; i<=512; i++)
        runtest_dotprod_rrrf(i);
}

