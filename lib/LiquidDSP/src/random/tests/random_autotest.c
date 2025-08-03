/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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

#define LIQUID_RANDOM_AUTOTEST_NUM_TRIALS (250000)
#define LIQUID_RANDOM_AUTOTEST_ERROR_TOL  (0.15)

// uniform
void autotest_randf()
{
    unsigned long int N = LIQUID_RANDOM_AUTOTEST_NUM_TRIALS;
    unsigned long int i;
    float x, m1=0.0f, m2=0.0f;
    float tol = LIQUID_RANDOM_AUTOTEST_ERROR_TOL;

    // uniform
    for (i=0; i<N; i++) {
        x = randf();
        m1 += x;
        m2 += x*x;
    }
    m1 /= (float) N;
    m2 = (m2 / (float)N) - m1*m1;

    CONTEND_DELTA(m1, 0.5f, tol);
    CONTEND_DELTA(m2, 1/12.0f, tol);
}

// Gauss
void autotest_randnf()
{
    unsigned long int N = LIQUID_RANDOM_AUTOTEST_NUM_TRIALS; // number of trials
    unsigned long int i;
    float x, m1=0.0f, m2=0.0f;
    float tol = LIQUID_RANDOM_AUTOTEST_ERROR_TOL;

    // uniform
    for (i=0; i<N; i++) {
        x = randnf();
        m1 += x;
        m2 += x*x;
    }
    m1 /= (float) N;
    m2 = (m2 / (float)N) - m1*m1;

    CONTEND_DELTA(m1, 0.0f, tol);
    CONTEND_DELTA(m2, 1.0f, tol);
}

// complex Gauss
void autotest_crandnf()
{
    unsigned long int N = LIQUID_RANDOM_AUTOTEST_NUM_TRIALS;
    unsigned long int i;
    float complex x;
    float m1=0.0f, m2=0.0f;
    float tol = LIQUID_RANDOM_AUTOTEST_ERROR_TOL;

    // uniform
    for (i=0; i<N; i++) {
        crandnf(&x);
        m1 += crealf(x) + cimagf(x);
        m2 += crealf(x)*crealf(x) + cimagf(x)*cimagf(x);
    }
    N *= 2; // double N for real and imag components
    m1 /= (float) N;
    m2 = (m2 / (float)N) - m1*m1;

    CONTEND_DELTA(m1, 0.0f, tol);
    CONTEND_DELTA(m2, 1.0f, tol);
}

// Weibull
void autotest_randweibf()
{
    unsigned long int N = LIQUID_RANDOM_AUTOTEST_NUM_TRIALS;
    unsigned long int i;
    float x, m1=0.0f, m2=0.0f;
    float tol = LIQUID_RANDOM_AUTOTEST_ERROR_TOL;
    float alpha=1.0f, beta=2.0f, gamma=6.0f;

    // uniform
    for (i=0; i<N; i++) {
        x = randweibf(alpha, beta, gamma);
        m1 += x;
        m2 += x*x;
    }
    m1 /= (float) N;
    m2 = (m2 / (float)N) - m1*m1;

    // compute expected moments (closed-form solution)
    float t0     = liquid_gammaf(1. + 1./alpha);
    float t1     = liquid_gammaf(1. + 2./alpha);
    float m1_exp = beta * t0 + gamma;
    float m2_exp = beta*beta*( t1 - t0*t0 );
    //printf("m1: %12.8f (expected %12.8f)\n", m1, m1_exp);
    //printf("m2: %12.8f (expected %12.8f)\n", m2, m2_exp);

    CONTEND_DELTA(m1, m1_exp, tol);
    CONTEND_DELTA(m2, m2_exp, tol);
}

// Rice-K
void autotest_randricekf()
{
    unsigned long int N = LIQUID_RANDOM_AUTOTEST_NUM_TRIALS;
    unsigned long int i;
    float x, m1=0.0f, m2=0.0f;
    float tol = LIQUID_RANDOM_AUTOTEST_ERROR_TOL;
    float K=2.0f, omega=1.0f;

    // uniform
    for (i=0; i<N; i++) {
        x = randricekf(K, omega);
        m1 += x;
        m2 += x*x;
    }
    m1 /= (float) N;
    m2 = (m2 / (float)N);

    CONTEND_DELTA(m1, 0.92749f, tol);
    CONTEND_DELTA(m2, omega, tol);
}

// exponential
void autotest_randexpf()
{
    unsigned long int N = LIQUID_RANDOM_AUTOTEST_NUM_TRIALS;
    unsigned long int i;
    float x, m1=0.0f, m2=0.0f;
    float tol = LIQUID_RANDOM_AUTOTEST_ERROR_TOL;
    float lambda = 2.3f;

    // uniform
    for (i=0; i<N; i++) {
        x = randexpf(lambda);
        m1 += x;
        m2 += x*x;
    }
    m1 /= (float) N;
    m2 = (m2 / (float)N) - m1*m1;

    // compute expected moments (closed-form solution)
    float m1_exp = 1. / lambda;
    float m2_exp = 1. / (lambda * lambda);
    //printf("m1: %12.8f (expected %12.8f)\n", m1, m1_exp);
    //printf("m2: %12.8f (expected %12.8f)\n", m2, m2_exp);

    CONTEND_DELTA(m1, m1_exp, tol);
    CONTEND_DELTA(m2, m2_exp, tol);
}

void autotest_random_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping random config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // exponential: lambda out of range
    CONTEND_EQUALITY( randexpf    (       -1.0f), 0.0f );
    CONTEND_EQUALITY( randexpf_pdf( 0.0f, -1.0f), 0.0f );
    CONTEND_EQUALITY( randexpf_cdf( 0.0f, -1.0f), 0.0f );
    // exponential: pdf, cdf with valid input, but negative variable
    CONTEND_EQUALITY( randexpf_pdf(-2.0f,  2.3f), 0.0f );
    CONTEND_EQUALITY( randexpf_cdf(-2.0f,  2.3f), 0.0f );

    // gamma: parameters out of range (alpha)
    CONTEND_EQUALITY( randgammaf    (       -1.0f,  1.0f), 0.0f );
    CONTEND_EQUALITY( randgammaf_pdf( 0.0f, -1.0f,  1.0f), 0.0f );
    CONTEND_EQUALITY( randgammaf_cdf( 0.0f, -1.0f,  1.0f), 0.0f );
    // gamma: parameters out of range (beta)
    CONTEND_EQUALITY( randgammaf    (        1.0f, -1.0f), 0.0f );
    CONTEND_EQUALITY( randgammaf_pdf( 0.0f,  1.0f, -1.0f), 0.0f );
    CONTEND_EQUALITY( randgammaf_cdf( 0.0f,  1.0f, -1.0f), 0.0f );
    // gamma: delta function parameter out of range
    CONTEND_EQUALITY( randgammaf_delta(-1.0f), 0.0f );
    // gamma: pdf, cdf with valid input, but negative variable
    CONTEND_EQUALITY( randgammaf_pdf(-2.0f, 1.2f, 2.3f), 0.0f );
    CONTEND_EQUALITY( randgammaf_cdf(-2.0f, 1.2f, 2.3f), 0.0f );

    // nakagami-m: parameters out of range (m)
    CONTEND_EQUALITY( randnakmf    (       0.2f,  1.0f), 0.0f );
    CONTEND_EQUALITY( randnakmf_pdf( 0.0f, 0.2f,  1.0f), 0.0f );
    CONTEND_EQUALITY( randnakmf_cdf( 0.0f, 0.2f,  1.0f), 0.0f );
    // nakagami-m: parameters out of range (omega)
    CONTEND_EQUALITY( randnakmf    (       1.0f, -1.0f), 0.0f );
    CONTEND_EQUALITY( randnakmf_pdf( 0.0f, 1.0f, -1.0f), 0.0f );
    CONTEND_EQUALITY( randnakmf_cdf( 0.0f, 1.0f, -1.0f), 0.0f );
    // nakagami-m: pdf, cdf with valid input, but negative variable
    CONTEND_EQUALITY( randnakmf_pdf(-2.0f, 1.2f, 2.3f), 0.0f );
    CONTEND_EQUALITY( randnakmf_cdf(-2.0f, 1.2f, 2.3f), 0.0f );
}

