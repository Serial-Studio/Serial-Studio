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
// AUTOTEST: polyf_findroots
//

//  _p      : polynomial,     [size: _order+1 x 1]
//  _r      : roots (sorted), [size: _order   x 1]
//  _ordre  : polynomial order
void polyf_findroots_testbench(float *         _p,
                               float complex * _r,
                               unsigned int    _order,
                               float           _tol)
{
    float complex roots[_order];
    polyf_findroots(_p,_order+1,roots);

    unsigned int i;
    if (liquid_autotest_verbose) {
        printf("poly:\n");
        for (i=0; i<=_order; i++)
            printf("  p[%3u] = %12.8f\n", i, _p[i]);

        printf("roots:\n");
        for (i=0; i<_order; i++) {
            float e = cabsf(roots[i] - _r[i]);
            printf("  r[%3u] = %12.8f + %12.8fj (%12.8f + %12.8fj) %12.4e%s\n",
                    i,
                    crealf(roots[i]), cimagf(roots[i]),
                    crealf(   _r[i]), cimagf(   _r[i]),
                    e, e < _tol ? "" : " *");
        }
    }

    // check to see if roots match within relative tolerance
    for (i=0; i<_order; i++)
        CONTEND_DELTA(roots[i], _r[i], _tol);
}

void autotest_polyf_findroots_real()
{
    // basic roots, no complex values
    float         p[6] = {6,11,-33,-33,11,6};
    float complex r[5] = {-3, -1, -1./3., 0.5, 2.};
    polyf_findroots_testbench(p, r, 5, 1e-6f);
}

void autotest_polyf_findroots_complex()
{

    // complex roots
    float         p[3] = {3,2,1};
    float complex r[2] = {-1 + _Complex_I*M_SQRT2,
                          -1 - _Complex_I*M_SQRT2};
    polyf_findroots_testbench(p, r, 2, 1e-6f);
}

void autotest_polyf_findroots_mix()
{

    // complex roots
    float         p[7] = {-1,6,5,4,3,2,1};
    float complex r[6] = {
		-1.544928106217380,
		-0.8438580445415772 + 1.251293921227189*_Complex_I,
		-0.8438580445415772 - 1.251293921227189*_Complex_I,
		 0.1464465720078399,
		 0.5430988116463471 + 1.282747429218130*_Complex_I,
		 0.5430988116463471 - 1.282747429218130*_Complex_I};
    polyf_findroots_testbench(p, r, 6, 1e-6f);
}

void autotest_polyf_findroots_mix2()
{
    // complex roots, longer polynomial
    float         p[11] = {
        -2.1218292415142059326171875000e-02,
         1.6006522178649902343750000000e+00,
        -1.2054302543401718139648437500e-01,
        -8.4453743696212768554687500000e-01,
        -1.1174567937850952148437500000e+00,
         8.2108253240585327148437500000e-01,
         2.2316795587539672851562500000e-01,
         1.4220994710922241210937500000e+00,
        -8.4215706586837768554687500000e-01,
         1.3681684434413909912109375000e-01,
         1.0689756833016872406005859375e-02,};

    float complex r[10] = {
       -17.67808709752869,
        -0.7645511425850682 + 0.4932343666704793*_Complex_I,
        -0.7645511425850682 - 0.4932343666704793*_Complex_I,
        -0.2764509491715267 + 1.058805768356938 *_Complex_I,
        -0.2764509491715267 - 1.058805768356938 *_Complex_I,
         0.01327054605125156,
         0.9170364272114475 + 0.3838341863217226*_Complex_I,
         0.9170364272114475 - 0.3838341863217226*_Complex_I,
         2.556937242081334  + 1.448576080447611 *_Complex_I,
         2.556937242081334  - 1.448576080447611 *_Complex_I};

    polyf_findroots_testbench(p, r, 10, 4e-6f);
}

// 
// AUTOTEST: polycf_findroots (random roots)
//
void xautotest_polycf_findroots_rand()
{
    unsigned int n=5;
    float tol=1e-4f;

    float complex p[n];
    float complex roots[n-1];

    float complex p_hat[n];

    unsigned int i;
    for (i=0; i<n; i++)
        p[i] = i == n-1 ? 1 : 3.0f * randnf();

    polycf_findroots(p,n,roots);

    float complex roots_hat[n-1];
    // convert form...
    for (i=0; i<n-1; i++)
        roots_hat[i] = -roots[i];

    polycf_expandroots(roots_hat,n-1,p_hat);

    if (liquid_autotest_verbose) {
        printf("poly:\n");
        for (i=0; i<n; i++)
            printf("  p[%3u] = %12.8f + j*%12.8f\n", i, crealf(p[i]), cimagf(p[i]));

        printf("roots:\n");
        for (i=0; i<n-1; i++)
            printf("  r[%3u] = %12.8f + j*%12.8f\n", i, crealf(roots[i]), cimagf(roots[i]));

        printf("poly (expanded roots):\n");
        for (i=0; i<n; i++)
            printf("  p[%3u] = %12.8f + j*%12.8f\n", i, crealf(p_hat[i]), cimagf(p_hat[i]));
    }

    for (i=0; i<n; i++) {
        CONTEND_DELTA(crealf(p[i]), crealf(p_hat[i]), tol);
        CONTEND_DELTA(cimagf(p[i]), cimagf(p_hat[i]), tol);
    }
}

