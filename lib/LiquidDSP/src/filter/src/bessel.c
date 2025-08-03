/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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

//
// Bessel filter design
//
// References:
//  [Bianchi:2007] G. Bianchi and R. Sorrentino, "Electronic Filter Simulation
//      and Design." New York: McGraw-Hill, 2007.
//  [Orchard:1965] H. J. Orchard, "The Roots of the Maximally Flat-Delay
//      Polynomials." IEEE Transactions on Circuit Theory, September, 1965.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.internal.h"

#define LIQUID_DEBUG_BESSEL_PRINT   0

// forward declarations

int fpoly_bessel(unsigned int _n, float * _p);

int fpoly_bessel_roots(unsigned int _n, float complex * _roots);

int fpoly_bessel_roots_orchard(unsigned int _n, float complex * _roots);

int fpoly_bessel_roots_orchard_recursion(unsigned int _n,
                                          float _x,
                                          float _y,
                                          float * _x_hat,
                                          float * _y_hat);

// ****************************************

// Compute analog zeros, poles, gain of low-pass Bessel
// filter, grouping complex conjugates together. If
// the filter order is odd, the single real pole is at
// the end of the array.  There are no zeros for the
// analog Bessel filter.  The gain is unity.
//  _n      :   filter order
//  _z      :   output analog zeros [length:  0]
//  _p      :   output analog poles [length: _n]
//  _k      :   output analog gain
int bessel_azpkf(unsigned int    _n,
                 float complex * _za,
                 float complex * _pa,
                 float complex * _ka)
{
    // roots are computed with order _n+1 so we must use a longer array to
    // prevent out-of-bounds write on the provided _pa array
    float complex _tmp_pa[_n+1];

    // compute poles (roots to Bessel polynomial)
    if (fpoly_bessel_roots(_n+1,_tmp_pa) != LIQUID_OK)
        return liquid_error(LIQUID_EICONFIG,"bessel_azpkf(), invalid configuration");
    for (int i = 0; i < _n; i++)
        _pa[i] = _tmp_pa[i];

    // analog Bessel filter prototype has no zeros

    // The analog Bessel filter's 3-dB cut-off frequency is a
    // non-linear function of its order.  This frequency can
    // be approximated from [Bianchi:2007] (1.67), pp. 33.
    // Re-normalize poles by (approximated) 3-dB frequency.
    float w3dB = sqrtf((2*_n-1)*logf(2.0f));
    unsigned int i;
    for (i=0; i<_n; i++)
        _pa[i] /= w3dB;

    // set gain
    *_ka = 1.0f;
    for (i=0; i<_n; i++)
        *_ka *= _pa[i];

    return LIQUID_OK;
}

int fpoly_bessel(unsigned int _n, float * _p)
{
    unsigned int k;
    unsigned int N = _n-1;
    for (k=0; k<_n; k++) {
#if 0
        // use internal log(gamma(z))
        float t0 = liquid_lngammaf((float)(2*N-k)+1);
        float t1 = liquid_lngammaf((float)(N-k)  +1);
        float t2 = liquid_lngammaf((float)(k)     +1);
#else
        // use standard math log(gamma(z))
        float t0 = lgammaf((float)(2*N-k)+1);
        float t1 = lgammaf((float)(N-k)  +1);
        float t2 = lgammaf((float)(k)    +1);
#endif

        // M_LN2 = log(2) = 0.693147180559945
        float t3 = M_LN2 * (float)(N-k);    // log(2^(N-k)) = log(2)*log(N-k)

        _p[k] = roundf(expf(t0 - t1 - t2 - t3));

#if 0
        printf("  p[%3u,%3u] = %12.4e\n", k, _n, _p[k]);
        printf("    t0 : %12.4e\n", t0);
        printf("    t1 : %12.4e\n", t1);
        printf("    t2 : %12.4e\n", t2);
        printf("    t3 : %12.4e\n", t3);
#endif
    }
    return LIQUID_OK;
}

int fpoly_bessel_roots(unsigned int    _n,
                       float complex * _roots)
{
    return fpoly_bessel_roots_orchard(_n, _roots);
}

// Estimate the roots of the _n^th-order Bessel polynomial using
// Orchard's recursion.  The initial estimates for the roots of
// L_{k} are extrapolated from those in L_{k-2} and L_{k-1}.
// The resulting root is near enough the true root such that
// Orchard's recursion will find it.
int fpoly_bessel_roots_orchard(unsigned int    _n,
                               float complex * _roots)
{
    // initialize arrays
    float complex r0[_n];       // roots of L_{k-2}
    float complex r1[_n];       // roots of L_{k-1}
    float complex r_hat[_n];    // roots of L_{k}

    unsigned int i, j;
    unsigned int p, L;
    for (i=1; i<_n; i++) {
        p = i % 2;  // order is odd?
        L = (i+p)/2;
        //printf("\n***** order %3u, p=%3u, L=%3u\n", i, p, L);

        if (i == 1) {
            r1[0]    = -1;
            r_hat[0] = -1;
        } else if (i == 2) {
            r1[0]    = -1;
            r_hat[0] = -1.5f + _Complex_I*0.5f*sqrtf(3.0f);
        } else {

            // use previous 2 sets of roots to estimate this set
            if (p) {
                // odd order : one real root on negative imaginary axis
                r_hat[0] = 2*crealf(r1[0]) - crealf(r0[0]);
            } else {
                // even order
                r_hat[0] = 2*r1[0] - conjf(r0[0]);
            }

            // linear extrapolation of roots of L_{k-2} and L_{k-1} for
            // new root estimate in L_{k}
            for (j=1; j<L; j++)
                r_hat[j] = 2*r1[j-p] - r0[j-1];

            for (j=0; j<L; j++) {
                float x = crealf(r_hat[j]);
                float y = cimagf(r_hat[j]);
                float x_hat, y_hat;
                fpoly_bessel_roots_orchard_recursion(i,x,y,&x_hat,&y_hat);
                r_hat[j] = x_hat + _Complex_I*y_hat;
            }
        }

        // copy roots:  roots(L_{k+1}) -> roots(L_{k+2))
        //              roots(L_{k})   -> roots(L_{k+1))
        memmove(r0, r1,    (L-p)*sizeof(float complex));
        memmove(r1, r_hat,     L*sizeof(float complex));
    }

    // copy results to output
    p = _n % 2;
    L = (_n-p)/2;
    for (i=0; i<L; i++) {
        unsigned int p = L-i-1;
        _roots[2*i+0] =       r_hat[p];
        _roots[2*i+1] = conjf(r_hat[p]);
    }

    // if order is odd, copy single real root last
    if (p)
        _roots[_n-1] = r_hat[0];

    return LIQUID_OK;
}

// from [Orchard:1965]
int fpoly_bessel_roots_orchard_recursion(unsigned int _n,
                                         float        _x,
                                         float        _y,
                                         float *      _x_hat,
                                         float *      _y_hat)
{
    if (_n < 2)
        return liquid_error(LIQUID_EICONFIG,"fpoly_bessel_roots_orchard_recursion(), n < 2");

    // create internal variables (use long double precision to help
    // algorithm converge, particularly for large _n)
    long double u0, u1, u2=0, u2p=0;
    long double v0, v1, v2=0, v2p=0;
    long double x = _x;
    long double y = _y;
    //long double eps = 1e-6f;

    unsigned int k,i;
    unsigned int num_iterations = 50;
    for (k=0; k<num_iterations; k++) {
        //printf("%3u :   %16.8e + j*%16.8e\n", k, x, y);
        u0 = 1.0;
        u1 = 1.0 + x;

        v0 = 0.0f;
        v1 = y;

        // compute u_r, v_r
        for (i=2; i<=_n; i++) {
            u2 = (2*i-1)*u1 + (x*x - y*y)*u0 - 2*x*y*v0;
            v2 = (2*i-1)*v1 + (x*x - y*y)*v0 + 2*x*y*u0;

            // if not on last iteration, update u0, v0, u1, v1
            if (i < _n) {
                u0 = u1; v0 = v1;
                u1 = u2; v1 = v2;
            }
        }

        // compute derivatives
        u2p = u2 - x*u1 + y*v1;
        v2p = v2 - x*v1 - y*u1;

        // update roots
        long double g = u2p*u2p + v2p*v2p;
        if (g == 0.) break;

        // For larger order _n, the step values dx and dy will be the
        // evaluation of the ratio of two large numbers which can prevent
        // the algorithm from converging for finite machine precision.
        long double dx = -(u2p*u2 + v2p*v2)/g;
        long double dy = -(u2p*v2 - v2p*u2)/g;
        x += dx;
        y += dy;
    }

    *_x_hat = x;
    *_y_hat = y;
    return LIQUID_OK;
}

