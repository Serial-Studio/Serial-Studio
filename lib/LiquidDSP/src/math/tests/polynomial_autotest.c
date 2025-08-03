/*
 * Copyright (c) 2007 - 2019 Joseph Gaeddert
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
#include "liquid.h"

// 
// AUTOTEST: poly_fit 3rd order polynomial, critically sampled
//
void autotest_polyf_fit_q3n3()
{
    unsigned int Q=2;   // polynomial order
    unsigned int n=3;   // input vector size

    float x[3] = {-1.0f, 0.0f, 1.0f};
    float y[3] = { 1.0f, 0.0f, 1.0f};
    float p[3];
    float p_test[3] = {0.0f, 0.0f, 1.0f};
    float tol = 1e-3f;

    unsigned int k=Q+1;
    polyf_fit(x,y,n, p,k);

    if (liquid_autotest_verbose) {
        unsigned int i;
        for (i=0; i<3; i++)
            printf("%3u : %12.8f > %12.8f\n", i, x[i], y[i]);
        for (i=0; i<3; i++)
            printf("p[%3u] = %12.8f\n", i, p[i]);
    }
    
    CONTEND_DELTA(p[0], p_test[0], tol);
    CONTEND_DELTA(p[1], p_test[1], tol);
    CONTEND_DELTA(p[2], p_test[2], tol);
    //CONTEND_DELTA(p[3], p_test[3], tol);
}

void autotest_polyf_lagrange_issue165()
{
    // Inputs taken from issue#165
    //unsigned int Q=2;   // polynomial order
    unsigned int n=3;   // input vector size
    float x[3] = {-1.0f, 0.0f, 1.0f};
    float y[3] = {7.059105f, 24.998369f, 14.365907f};
    float p[3];
    float tol = 1e-3f;

    polyf_fit_lagrange(x,y,n,p);
    unsigned int j;
    for (j=0; j<n; j++)
        printf("%3u : %12.8f > %12.8f\n", j, x[j], y[j]);
    for (j=0; j<n; j++)
        printf("p[%3u] = %12.8f\n", j, p[j]);

    float y_out[3];
    for (j=0; j<n; j++) {
        y_out[j] = polyf_val(p, n, x[j]);
        printf("y_out[%3u] = %12.8f exp=%12.8f\n", j, y_out[j], y[j]);
        CONTEND_DELTA(y[j], y_out[j], tol);
    }

    polyf_fit(x,y,n,p,n);
    for (j=0; j<n; j++)
        printf("p_least_sq[%3u] = %12.8f\n", j, p[j]);

    float y_least_sq[3];
    for (j=0; j<n; j++) {
        y_least_sq[j] = polyf_val(p, n, x[j]);
        printf("y_least_sq[%3u] = %12.8f exp=%12.8f\n", j, y_least_sq[j], y[j]);
        CONTEND_DELTA(y_least_sq[j], y_out[j], tol);
    }
}


// Taken from wiki page for lagrange polynomial
// for y=x^3
void autotest_polyf_lagrange()
{

    unsigned int n=3; // input vector size

    float x[3] = {1.0f, 2.0f, 3.0f};
    float y[3] = {1.0f, 8.0f, 27.0f};
    float p[3];
    float tol = 1e-3;
    polyf_fit_lagrange(x,y,n,p);
    unsigned int j;
    for (j=0; j<n; j++)
        printf("p[%3u] = %12.8f\n", j, p[j]);
    float y_out[3];
    for (j=0; j<n; j++) {
        y_out[j] = polyf_val(p, n, x[j]);
        printf("y_out[%3u] = %12.8f exp=%12.8f\n", j, y_out[j], y[j]);
        CONTEND_DELTA(y[j], y_out[j], tol);
    }
}
#if 0
// 
// AUTOTEST: poly_expandbinomial
//
void xautotest_polyf_expandbinomial_4()
{
    float a[4] = { 3, 2, -5, 1 };
    float c[5];
    float c_test[5] = { 1, 1, -19, -49, -30 };
    float tol = 1e-3f;

    polyf_expandbinomial(a,4,c);

    if (liquid_autotest_verbose) {
        unsigned int i;
        printf("c[5] = ");
        for (i=0; i<5; i++)
            printf("%8.2f", c[i]);
        printf("\n");
    }
    
    CONTEND_DELTA(c[0], c_test[0], tol);
    CONTEND_DELTA(c[1], c_test[1], tol);
    CONTEND_DELTA(c[2], c_test[2], tol);
    CONTEND_DELTA(c[3], c_test[3], tol);
    CONTEND_DELTA(c[4], c_test[4], tol);
}
#endif

// 
// AUTOTEST: poly_expandroots
//
void autotest_polyf_expandroots_4()
{
    float roots[5] = { -2, -1, -4, 5, 3 };
    float c[6];
    float c_test[6] = { 120, 146, 1, -27, -1, 1 };
    float tol = 1e-3f;

    polyf_expandroots(roots,5,c);

    if (liquid_autotest_verbose) {
        unsigned int i;
        printf("c[6] = ");
        for (i=0; i<6; i++)
            printf("%8.2f", c[i]);
        printf("\n");
    }
    
    CONTEND_DELTA(c[0], c_test[0], tol);
    CONTEND_DELTA(c[1], c_test[1], tol);
    CONTEND_DELTA(c[2], c_test[2], tol);
    CONTEND_DELTA(c[3], c_test[3], tol);
    CONTEND_DELTA(c[4], c_test[4], tol);
    CONTEND_DELTA(c[5], c_test[5], tol);
}


// 
// AUTOTEST: poly_expandroots
//
void autotest_polyf_expandroots_11()
{
    float roots[11] = { -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11 };
    float c[12];
    float c_test[12] = {39916800,
                        120543840,
                        150917976,
                        105258076,
                        45995730,
                        13339535,
                        2637558,
                        357423,
                        32670,
                        1925,
                        66,
                        1};
    float tol = 1e-6f;

    polyf_expandroots(roots,11,c);

    unsigned int i;
    for (i=0; i<12; i++) {
        if (liquid_autotest_verbose)
            printf("  c[%3u] : %16.8e (expected %16.8e)\n", i, c[i], c_test[i]);

        CONTEND_DELTA(c[i], c_test[i], fabsf(tol*c_test[i]));
    }
}

// 
// AUTOTEST: polycf_expandroots
//
void autotest_polycf_expandroots_4()
{
    // expand complex roots on conjugate pair
    float theta = 1.7f;
    float complex a[2] = { -cexpf(_Complex_I*theta), -cexpf(-_Complex_I*theta) };
    float complex c[3];
    float complex c_test[3] = { 1, 2*cosf(theta), 1 };
    float tol = 1e-3f;

    polycf_expandroots(a,2,c);

    if (liquid_autotest_verbose) {
        unsigned int i;
        for (i=0; i<3; i++)
            printf("c[%3u] = %12.8f + j*%12.8f\n", i, crealf(c[i]), cimagf(c[i]));
    }
    
    CONTEND_DELTA(crealf(c[0]), crealf(c_test[0]), tol);
    CONTEND_DELTA(cimagf(c[0]), cimagf(c_test[0]), tol);

    CONTEND_DELTA(crealf(c[1]), crealf(c_test[1]), tol);
    CONTEND_DELTA(cimagf(c[1]), cimagf(c_test[1]), tol);

    CONTEND_DELTA(crealf(c[2]), crealf(c_test[2]), tol);
    CONTEND_DELTA(cimagf(c[2]), cimagf(c_test[2]), tol);

}

// 
// AUTOTEST: poly_expandroots2
//
// expand (2*x-5)*(3*x+2)*(-1*x+3)
//
void autotest_polyf_expandroots2_3()
{
    unsigned int n=3;
    float a[3] = {  2,  3, -1 };
    float b[3] = {  5, -2, -3 };
    float c[4];
    float c_test[4] = { -6, 29, -23, -30 };
    float tol = 1e-3f;

    polyf_expandroots2(a,b,n,c);

    if (liquid_autotest_verbose) {
        unsigned int i;
        printf("c[%u] = ", n+1);
        for (i=0; i<n+1; i++)
            printf("%8.2f", c[i]);
        printf("\n");
    }
    
    CONTEND_DELTA(c[0], c_test[0], tol);
    CONTEND_DELTA(c[1], c_test[1], tol);
    CONTEND_DELTA(c[2], c_test[2], tol);
    CONTEND_DELTA(c[3], c_test[3], tol);
}


// 
// AUTOTEST: polyf_mul
//
void autotest_polyf_mul_2_3()
{
    float a[3] = {  2, -4,  3 };
    float b[4] = { -9,  3, -2,  5};
    float c[6];
    float c_test[6] = { -18, 42, -43, 27, -26, 15 };
    float tol = 1e-3f;

    polyf_mul(a,2,b,3,c);

    if (liquid_autotest_verbose) {
        unsigned int i;
        printf("c[6] = ");
        for (i=0; i<6; i++)
            printf("%8.2f", c[i]);
        printf("\n");
    }
    
    CONTEND_DELTA(c[0], c_test[0], tol);
    CONTEND_DELTA(c[1], c_test[1], tol);
    CONTEND_DELTA(c[2], c_test[2], tol);
    CONTEND_DELTA(c[3], c_test[3], tol);
    CONTEND_DELTA(c[4], c_test[4], tol);
    CONTEND_DELTA(c[5], c_test[5], tol);
}

// 
// AUTOTEST: poly_expandbinomial
//
void autotest_poly_expandbinomial_n6()
{
    unsigned int n=6;
    float c[7];
    float c_test[7] = {1, 6, 15, 20, 15, 6, 1};

    polyf_expandbinomial(n,c);

    if (liquid_autotest_verbose) {
        unsigned int i;
        printf("c[%2u] = ", n+1);
        for (i=0; i<=n; i++)
            printf("%6.1f", c[i]);
        printf("\n");
    }
    
    CONTEND_SAME_DATA(c,c_test,sizeof(c));
}


// 
// AUTOTEST: poly_binomial_expand_pm
//
void autotest_poly_binomial_expand_pm_m6_k1()
{
    unsigned int m=5;
    unsigned int k=1;
    unsigned int n = m+k;
    float c[7];
    float c_test[7] = {1,  4,  5,  0, -5, -4, -1};

    polyf_expandbinomial_pm(m,k,c);

    unsigned int i;
    if (liquid_autotest_verbose) {
        printf("c[%u] = ", m+1);
        for (i=0; i<=n; i++)
            printf("%6.1f", c[i]);
        printf("\n");
    }

    for (i=0; i<=n; i++)
        CONTEND_DELTA(c[i], c_test[i], 1e-3f);
}

// 
// AUTOTEST: poly_expandbinomial_pm
//
void autotest_poly_expandbinomial_pm_m5_k2()
{
    unsigned int m=5;
    unsigned int k=2;
    unsigned int n = m+k;
    float c[8];
    float c_test[8] = {  1.0f,  3.0f,  1.0f, -5.0f,
                        -5.0f,  1.0f,  3.0f,  1.0f};

    polyf_expandbinomial_pm(m,k,c);

    unsigned int i;
    if (liquid_autotest_verbose) {
        printf("c[%u] = ", n+1);
        for (i=0; i<=n; i++)
            printf("%6.2f", c[i]);
        printf("\n");
    }

    for (i=0; i<=n; i++)
        CONTEND_DELTA(c[i], c_test[i], 1e-3f);
}


