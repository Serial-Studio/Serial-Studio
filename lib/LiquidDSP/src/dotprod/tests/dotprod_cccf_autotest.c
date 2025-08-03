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
// AUTOTEST: compare structured result to ordinal computation
//

// 
// AUTOTEST: dot product with floating-point data
//
void autotest_dotprod_cccf_rand16()
{
    float complex h[16] = {
      0.17702709 +   1.38978455*_Complex_I,  0.91294148 +   0.39217381*_Complex_I,
     -0.80607338 +   0.76477512*_Complex_I,  0.05099755 +  -0.87350051*_Complex_I,
      0.44513826 +  -0.49490569*_Complex_I,  0.14754967 +   2.04349962*_Complex_I,
      1.07246623 +   1.08146290*_Complex_I, -1.14028088 +   1.83380899*_Complex_I,
      0.38105361 +  -0.45591846*_Complex_I,  0.32605401 +   0.34440081*_Complex_I,
     -0.05477144 +   0.60832595*_Complex_I,  1.81667523 +  -1.12238075*_Complex_I,
     -0.87190497 +   1.10743858*_Complex_I,  1.30921403 +   1.24438643*_Complex_I,
      0.55524695 +  -1.94931519*_Complex_I, -0.87191170 +   0.91693119*_Complex_I,
    };

    float complex x[16] = {
     -2.19591953 +  -0.93229692*_Complex_I,  0.17150376 +   0.56165114*_Complex_I,
      1.58354529 +  -0.50696037*_Complex_I,  1.40929619 +   0.87868803*_Complex_I,
     -0.75505072 +  -0.30867372*_Complex_I, -0.09821367 +  -0.73949106*_Complex_I,
      0.03785571 +   0.72763665*_Complex_I, -1.20262636 +  -0.88838102*_Complex_I,
      0.23323685 +   0.12456235*_Complex_I,  0.34593736 +   0.02529594*_Complex_I,
      0.33669564 +   0.39064649*_Complex_I, -2.45003867 +  -0.54862205*_Complex_I,
     -2.64870707 +   2.33444473*_Complex_I, -0.92284477 +  -2.45121397*_Complex_I,
      0.24852918 +  -0.62409860*_Complex_I, -0.87039907 +   0.90921212*_Complex_I,
    };

    float complex y;
    float complex test     = -0.604285042605890 - 12.390925785344704 * _Complex_I;
    float complex test_rev =  3.412365881765360 + 6.1885320363931480 * _Complex_I;

    float tol = 1e-3f;

    dotprod_cccf_run(h,x,16,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    dotprod_cccf_run4(h,x,16,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    // test object
    dotprod_cccf q = dotprod_cccf_create(h,16);
    dotprod_cccf_execute(q,x,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    // test running in reverse
    q = dotprod_cccf_recreate_rev(q,h,16);
    dotprod_cccf_execute(q,x,&y);
    CONTEND_DELTA( crealf(y), crealf(test_rev), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test_rev), tol);

    // create original again
    q = dotprod_cccf_recreate(q,h,16);
    dotprod_cccf_execute(q,x,&y);
    CONTEND_DELTA( crealf(y), crealf(test), tol);
    CONTEND_DELTA( cimagf(y), cimagf(test), tol);

    // clean it up
    dotprod_cccf_destroy(q);
}

// 
// AUTOTEST: structured dot product, odd lengths
//
void autotest_dotprod_cccf_struct_lengths()
{
    float tol = 4e-6;
    float complex y;

    float complex h[35] = {
      1.11555653 +   2.30658043*_Complex_I, -0.36133676 +  -0.10917327*_Complex_I,
      0.17714505 +  -2.14631440*_Complex_I,  2.20424609 +   0.59063608*_Complex_I,
     -0.44699194 +   0.23369318*_Complex_I,  0.60613931 +   0.21868288*_Complex_I,
     -1.18746289 +  -0.52159563*_Complex_I, -0.46277775 +   0.75010157*_Complex_I,
      0.93796307 +   0.28608151*_Complex_I, -2.18699829 +   0.38029319*_Complex_I,
      0.16145611 +   0.18343353*_Complex_I, -0.62653631 +  -1.79037656*_Complex_I,
     -0.67042462 +   0.11044084*_Complex_I,  0.70333438 +   1.78729174*_Complex_I,
     -0.32923580 +   0.78514690*_Complex_I,  0.27534332 +  -0.56377431*_Complex_I,
      0.41492559 +   1.37176526*_Complex_I,  3.25368958 +   2.70495218*_Complex_I,
      1.63002035 +  -0.14193750*_Complex_I,  2.22057186 +   0.55056461*_Complex_I,
      1.40896777 +   0.80722903*_Complex_I, -0.22334033 +  -0.14227395*_Complex_I,
     -1.48631186 +   0.53610531*_Complex_I, -1.91632185 +   0.88755083*_Complex_I,
     -0.52054895 +  -0.35572001*_Complex_I, -1.56515607 +  -0.41448794*_Complex_I,
     -0.91107117 +   0.17059659*_Complex_I, -0.77007659 +   2.73381816*_Complex_I,
     -0.46645585 +   0.38994666*_Complex_I,  0.80317663 +  -0.41756968*_Complex_I,
      0.26992512 +   0.41828145*_Complex_I, -0.72456446 +   1.25002030*_Complex_I,
      1.19573306 +   0.98449546*_Complex_I,  1.42491943 +  -0.55426305*_Complex_I,
      1.08243614 +   0.35774368*_Complex_I, };

    float complex x[35] = {
     -0.82466736 +  -1.39329228*_Complex_I, -1.46176052 +  -1.96218827*_Complex_I,
     -1.28388174 +  -0.07152934*_Complex_I, -0.51910014 +  -0.37915971*_Complex_I,
     -0.65964708 +  -0.98417534*_Complex_I, -1.40213479 +  -0.82198463*_Complex_I,
      0.86051446 +   0.97926463*_Complex_I,  0.26257342 +   0.76586696*_Complex_I,
      0.72174183 +  -1.89884636*_Complex_I, -0.26018863 +   1.06920599*_Complex_I,
      0.57949117 +  -0.77431546*_Complex_I,  0.84635184 +  -0.81123009*_Complex_I,
     -1.12637629 +  -0.42027412*_Complex_I, -1.04214881 +   0.90519721*_Complex_I,
      0.54458433 +  -1.03487314*_Complex_I, -0.17847893 +   2.20358978*_Complex_I,
      0.19642532 +  -0.07449796*_Complex_I, -1.84958229 +   0.13218920*_Complex_I,
     -1.49042886 +   0.81610408*_Complex_I, -0.27466940 +  -1.48438409*_Complex_I,
      0.29239375 +   0.72443343*_Complex_I, -1.20243456 +  -2.77032750*_Complex_I,
     -0.41784260 +   0.77455254*_Complex_I,  0.37737465 +  -0.52426993*_Complex_I,
     -1.25500377 +   1.76270122*_Complex_I,  1.55976056 +  -1.18189171*_Complex_I,
     -0.05111343 +  -1.18849396*_Complex_I, -1.92966664 +   0.66504899*_Complex_I,
     -2.82387897 +   1.41128242*_Complex_I, -1.48171326 +  -0.03347470*_Complex_I,
      0.38047273 +  -1.40969799*_Complex_I,  1.71995272 +   0.00298203*_Complex_I,
      0.56040910 +  -0.12713027*_Complex_I, -0.46653022 +  -0.65450499*_Complex_I,
      0.15515755 +   1.58944030*_Complex_I, };

    float complex v32 = -11.5100903519506 - 15.3575526884014*_Complex_I;
    float complex v33 = -10.7148314918614 - 14.9578463360225*_Complex_I;
    float complex v34 = -11.7423673921916 - 15.6318827515320*_Complex_I;
    float complex v35 = -12.1430314741466 - 13.8559085000689*_Complex_I;

    // 
    dotprod_cccf dp;

    // n = 32
    dp = dotprod_cccf_create(h,32);
    dotprod_cccf_execute(dp, x, &y);
    CONTEND_DELTA(y, v32, tol);
    dotprod_cccf_destroy(dp);
    if (liquid_autotest_verbose) {
        printf("  dotprod-cccf-32 : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                crealf(y), cimagf(y), crealf(v32), cimagf(v32));
    }

    // n = 33
    dp = dotprod_cccf_create(h,33);
    dotprod_cccf_execute(dp, x, &y);
    CONTEND_DELTA(y, v33, tol);
    dotprod_cccf_destroy(dp);
    if (liquid_autotest_verbose) {
        printf("  dotprod-cccf-33 : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                crealf(y), cimagf(y), crealf(v33), cimagf(v33));
    }

    // n = 34
    dp = dotprod_cccf_create(h,34);
    dotprod_cccf_execute(dp, x, &y);
    CONTEND_DELTA(y, v34, tol);
    dotprod_cccf_destroy(dp);
    if (liquid_autotest_verbose) {
        printf("  dotprod-cccf-34 : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                crealf(y), cimagf(y), crealf(v34), cimagf(v34));
    }

    // n = 35
    dp = dotprod_cccf_create(h,35);
    dotprod_cccf_execute(dp, x, &y);
    CONTEND_DELTA(y, v35, tol);
    dotprod_cccf_destroy(dp);
    if (liquid_autotest_verbose) {
        printf("  dotprod-cccf-35 : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                crealf(y), cimagf(y), crealf(v35), cimagf(v35));
    }
}

// helper function (compare structured object to ordinal computation)
void runtest_dotprod_cccf(unsigned int _n)
{
    float tol = 1e-3;
    float complex h[_n];
    float complex x[_n];

    // generate random coefficients
    unsigned int i;
    for (i=0; i<_n; i++) {
        h[i] = randnf() + randnf() * _Complex_I;
        x[i] = randnf() + randnf() * _Complex_I;
    }
    
    // compute expected value (ordinal computation)
    float complex y_test=0;
    for (i=0; i<_n; i++)
        y_test += h[i] * x[i];

    // create and run dot product object
    float complex y_struct;
    dotprod_cccf dp;
    dp = dotprod_cccf_create(h,_n);
    dotprod_cccf_execute(dp, x, &y_struct);
    dotprod_cccf_destroy(dp);

    // run unstructured
    float complex y_run, y_run4;
    dotprod_cccf_run (h,x,_n,&y_run );
    dotprod_cccf_run4(h,x,_n,&y_run4);

    // print results
    if (liquid_autotest_verbose) {
        printf("  dotprod-cccf-%-4u(struct) : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                _n, crealf(y_struct), cimagf(y_struct), crealf(y_test), cimagf(y_test));
        printf("  dotprod-cccf-%-4u(run   ) : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
                _n, crealf(y_run   ), cimagf(y_run   ), crealf(y_test), cimagf(y_test));
        printf("  dotprod-cccf-%-4u(run4  ) : %12.8f + j%12.8f (expected %12.8f + j%12.8f)\n",
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
void autotest_dotprod_cccf_struct_vs_ordinal()
{
    // run many, many tests
    unsigned int i;
    for (i=1; i<=512; i++)
        runtest_dotprod_cccf(i);
}

