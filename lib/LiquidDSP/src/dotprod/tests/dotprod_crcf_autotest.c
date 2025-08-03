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

#include "autotest/autotest.h"
#include "liquid.internal.h"

// 
// AUTOTEST: dot product with floating-point data
//
void autotest_dotprod_crcf_rand01()
{
    float h[16] = {
      5.5375e-02,  -6.5857e-01,  -1.7657e+00,   7.7444e-01, 
      8.0730e-01,  -5.1340e-01,  -9.3437e-02,  -5.6301e-01, 
     -6.6480e-01,  -2.1673e+00,   9.0269e-01,   3.5284e+00, 
     -9.7835e-01,  -6.9512e-01,  -1.2958e+00,   1.1628e+00
    };

    float complex x[16] = {
      1.3164e+00+  5.4161e-01*_Complex_I,   1.8295e-01+ -9.0284e-02*_Complex_I, 
      1.3487e+00+ -1.8148e+00*_Complex_I,  -7.4696e-01+ -4.1792e-01*_Complex_I, 
     -9.0551e-01+ -4.4294e-01*_Complex_I,   6.0591e-01+ -1.5383e+00*_Complex_I, 
     -7.5393e-01+ -3.5691e-01*_Complex_I,  -4.5733e-01+  1.1926e-01*_Complex_I, 
     -1.4744e-01+ -4.7676e-02*_Complex_I,  -1.2422e+00+ -2.0213e+00*_Complex_I, 
      3.3208e-02+ -1.3756e+00*_Complex_I,  -4.8573e-01+  1.0977e+00*_Complex_I, 
      1.5053e+00+  2.1141e-01*_Complex_I,  -8.4062e-01+ -1.0211e+00*_Complex_I, 
     -1.3932e+00+ -4.8491e-01*_Complex_I,  -1.4234e+00+  2.0333e-01*_Complex_I
    };

    float complex y;
    float complex test     = -3.35346556487224 + 11.78023318618137*_Complex_I;
    float complex test_rev =  3.655541203500000 + 4.26531912591000*_Complex_I;
    float tol = 1e-3f;

    dotprod_crcf_run(h,x,16,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    dotprod_crcf_run4(h,x,16,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    // test object
    dotprod_crcf q = dotprod_crcf_create(h,16);
    dotprod_crcf_execute(q,x,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    // test running in reverse
    q = dotprod_crcf_recreate_rev(q,h,16);
    dotprod_crcf_execute(q,x,&y);
    CONTEND_DELTA( crealf(y), crealf(test_rev), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test_rev), tol);

    // create original again
    q = dotprod_crcf_recreate(q,h,16);
    dotprod_crcf_execute(q,x,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    // clean it up
    dotprod_crcf_destroy(q);
}

// 
// AUTOTEST: dot product with floating-point data
//
void autotest_dotprod_crcf_rand02()
{
    float h[16] = {
      4.7622e-01,   7.1453e-01,  -7.1370e-01,  -1.6457e-01, 
     -1.1573e-01,   6.4114e-01,  -1.0688e+00,  -1.6761e+00, 
     -1.0376e+00,  -1.0991e+00,  -2.4161e-01,   4.6065e-01, 
     -1.0403e+00,  -1.1424e-01,  -1.2371e+00,  -7.9723e-01
    };

    float complex x[16] = {
     -8.3558e-01+  3.0504e-01*_Complex_I,  -6.3004e-01+  2.4680e-01*_Complex_I, 
      9.6908e-01+  1.2978e+00*_Complex_I,  -2.0587e+00+  9.5385e-01*_Complex_I, 
      2.5692e-01+ -1.7314e+00*_Complex_I,  -1.2237e+00+ -6.2139e-02*_Complex_I, 
      5.0300e-02+ -9.2092e-01*_Complex_I,  -1.8816e-01+  7.0746e-02*_Complex_I, 
     -2.4177e+00+  8.3177e-01*_Complex_I,   1.6871e-01+ -8.5129e-02*_Complex_I, 
      6.5203e-01+  2.0739e-02*_Complex_I,  -1.2331e-01+ -9.7920e-01*_Complex_I, 
      8.2352e-01+  9.1093e-01*_Complex_I,   1.5161e+00+ -9.1865e-01*_Complex_I, 
     -2.0892e+00+  2.7759e-02*_Complex_I,  -2.5188e-01+  2.5568e-01*_Complex_I
    };

    float complex y;
    float complex test = 2.11053363855085 - 2.04167493441477*_Complex_I;
    float tol = 1e-3f;

    dotprod_crcf_run(h,x,16,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    dotprod_crcf_run4(h,x,16,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    // test object
    dotprod_crcf q = dotprod_crcf_create(h,16);
    dotprod_crcf_execute(q,x,&y);
    if (liquid_autotest_verbose) {
        printf("  dotprod : %12.8f + j%12.8f (expected: %12.8f + j%12.8f)\n",
                crealf(y), cimagf(y), crealf(test), cimagf(test));
    }
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);
    dotprod_crcf_destroy(q);
}

// 
// AUTOTEST: compare structured result to ordinal computation
//

// helper function (compare structured object to ordinal computation)
void runtest_dotprod_crcf(unsigned int _n)
{
    float tol = 1e-4;
    float h[_n];
    float complex x[_n];

    // generate random coefficients
    unsigned int i;
    for (i=0; i<_n; i++) {
        h[i] = randnf();
        x[i] = randnf() + randnf() * _Complex_I;
    }
    
    // compute expected value (ordinal computation)
    float complex y_test=0;
    for (i=0; i<_n; i++)
        y_test += h[i] * x[i];

    // create and run dot product object
    float complex y_struct;
    dotprod_crcf dp;
    dp = dotprod_crcf_create(h,_n);
    dotprod_crcf_execute(dp, x, &y_struct);
    dotprod_crcf_destroy(dp);

    // run unstructured
    float complex y_run, y_run4;
    dotprod_crcf_run (h,x,_n,&y_run );
    dotprod_crcf_run4(h,x,_n,&y_run4);

    // print results
    if (liquid_autotest_verbose) {
        printf("  dotprod-crcf-%-4u(struct) : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                _n, crealf(y_struct), cimagf(y_struct), crealf(y_test), cimagf(y_test));
        printf("  dotprod-crcf-%-4u(run   ) : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                _n, crealf(y_run   ), cimagf(y_run   ), crealf(y_test), cimagf(y_test));
        printf("  dotprod-crcf-%-4u(run4  ) : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                _n, crealf(y_run4  ), cimagf(y_run4  ), crealf(y_test), cimagf(y_test));
    }

    // validate result (structured object)
    CONTEND_DELTA(crealf(y_struct), crealf(y_test), tol);
    CONTEND_DELTA(cimagf(y_struct), cimagf(y_test), tol);

    // validate result (unstructured, run)
    CONTEND_DELTA(crealf(y_run   ), crealf(y_test), tol);
    CONTEND_DELTA(cimagf(y_run   ), cimagf(y_test), tol);

    // validate result (unstructured, run4)
    CONTEND_DELTA(crealf(y_run4  ), crealf(y_test), tol);
    CONTEND_DELTA(cimagf(y_run4  ), cimagf(y_test), tol);
}

// compare structured object to ordinal computation
void autotest_dotprod_crcf_struct_vs_ordinal()
{
    // run many, many tests
    unsigned int i;
    for (i=1; i<=512; i++)
        runtest_dotprod_crcf(i);
}

