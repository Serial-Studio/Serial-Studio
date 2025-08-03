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

//
// polynomial methods for finding roots (double precision)
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "liquid.internal.h"

// debug polynomial root-finding methods?
#define LIQUID_POLY_FINDROOTS_DEBUG     0

// finds the complex roots of the polynomial using the Durand-Kerner method
//  _p      :   polynomial array, ascending powers [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _roots  :   resulting complex roots [size: _k-1 x 1]
int liquid_poly_findroots_durandkerner(double *         _p,
                                       unsigned int     _k,
                                       double complex * _roots)
{
    if (_k < 2)
        return liquid_error(LIQUID_EICONFIG,"liquid_poly_findroots_durandkerner(), order must be greater than 0");
    if (_p[_k-1] != 1)
        return liquid_error(LIQUID_EICONFIG,"liquid_poly_findroots_durandkerner(), _p[_k-1] must be equal to 1");

    unsigned int i;
    unsigned int num_roots = _k-1;
    double r0[num_roots];
    double r1[num_roots];

    // find initial magnitude
    float g     = 0.0f;
    float gmax  = 0.0f;
    for (i=0; i<_k; i++) {
        g = fabs(_p[i]);
        if (i==0 || g > gmax)
            gmax = g;
    }

    // initialize roots
    double t0 = 0.9f * (1 + gmax) * cexpf(_Complex_I*1.1526f);
    double t  = 1.0f;
    for (i=0; i<num_roots; i++) {
        r0[i] = t;
        t *= t0;
    }

    unsigned int max_num_iterations = 50;
    int continue_iterating = 1;
    unsigned int j, k;
    double f;
    double fp;
    //for (i=0; i<num_iterations; i++) {
    i = 0;
    while (continue_iterating) {
#if LIQUID_POLY_FINDROOTS_DEBUG
        printf("liquid_poly_findroots(), i=%3u :\n", i);
        for (j=0; j<num_roots; j++)
            printf("  r[%3u] = %12.8f + j*%12.8f\n", j, creal(r0[j]), cimag(r0[j]));
#endif
        for (j=0; j<num_roots; j++) {
            f = poly_val(_p,_k,r0[j]);
            fp = 1;
            for (k=0; k<num_roots; k++) {
                if (k==j) continue;
                fp *= r0[j] - r0[k];
            }
            r1[j] = r0[j] - f / fp;
        }

        // stop iterating if roots have settled
        float delta=0.0f;
        double e;
        for (j=0; j<num_roots; j++) {
            e = r0[j] - r1[j];
            delta += creal(e*conjf(e));
        }
        delta /= num_roots * gmax;
#if LIQUID_POLY_FINDROOTS_DEBUG
        printf("delta[%3u] = %12.4e\n", i, delta);
#endif

        if (delta < 1e-6f || i == max_num_iterations)
            continue_iterating = 0;

        memmove(r0, r1, num_roots*sizeof(double));
        i++;
    }

    for (i=0; i<_k; i++)
        _roots[i] = r1[i];
    return LIQUID_OK;
}

// finds the complex roots of the polynomial using Bairstow's method
//  _p      :   polynomial array, ascending powers [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _roots  :   resulting complex roots [size: _k-1 x 1]
int liquid_poly_findroots_bairstow(double *         _p,
                                   unsigned int     _k,
                                   double complex * _roots)
{
    double p0[_k];       // buffer 0
    double p1[_k];       // buffer 1
    double * p   = NULL; // input polynomial
    double * pr  = NULL; // output (reduced) polynomial

    unsigned int i;
    unsigned int k=0;   // output roots counter
    memmove(p0, _p, _k*sizeof(double));

    double u, v;

    unsigned int n = _k;        // input counter (decrementer)
    unsigned int r = _k % 2;    // polynomial length odd? (order even?)
    unsigned int L = (_k-r)/2;  // semi-length
    for (i=0; i<L-1+r; i++) {
        // set polynomial and reduced polynomial buffer pointers
        p  = (i % 2) == 0 ? p0 : p1;
        pr = (i % 2) == 0 ? p1 : p0;

        // initial estimates for u, v
        if (p[n-1] == 0) {
            liquid_error(LIQUID_EIVAL,"poly_findroots_bairstow(), irreducible polynomial");
            p[n-1] = 1e-12;
        }
        u = p[n-2] / p[n-1];
        v = p[n-3] / p[n-1];

        // compute factor using Bairstow's recursion
        if (n > 3)
            liquid_poly_findroots_bairstow_persistent(p,n,pr,&u,&v);

        // compute complex roots of x^2 + u*x + v
        double complex r0 = 0.5f*(-u + csqrtf(u*u - 4.0*v));
        double complex r1 = 0.5f*(-u - csqrtf(u*u - 4.0*v));
        //printf("roots: r0=%12.8f + j*%12.8f, r1=%12.8f + j*%12.8f\n\n",
        //        creal(r0), cimag(r0), creal(r1), cimag(r1));

        // append result to output
        _roots[k++] = r0;
        _roots[k++] = r1;

#if LIQUID_POLY_FINDROOTS_DEBUG
        // print debugging info
        unsigned int j;
        printf("initial polynomial:\n");
        for (j=0; j<n; j++)
            printf("  p[%3u]  = %12.8f + j*%12.8f\n", j, creal(p[j]), cimag(p[j]));

        printf("polynomial factor: x^2 + u*x + v\n");
        printf("  u : %12.8f + j*%12.8f\n", creal(u), cimag(u));
        printf("  v : %12.8f + j*%12.8f\n", creal(v), cimag(v));

        printf("roots:\n");
        printf("  r0 : %12.8f + j*%12.8f\n", creal(r0), cimag(r0));
        printf("  r1 : %12.8f + j*%12.8f\n", creal(r1), cimag(r1));

        printf("reduced polynomial:\n");
        for (j=0; j<n-2; j++)
            printf("  pr[%3u] = %12.8f + j*%12.8f\n", j, creal(pr[j]), cimag(pr[j]));
#endif

        // decrement new (reduced) polynomial size by 2
        n -= 2;
    }

    if (r==0) {
#if LIQUID_POLY_FINDROOTS_DEBUG
        assert(n==2);
#endif
        _roots[k++] = -pr[0]/pr[1];
    }
    return LIQUID_OK;
}

// iterate over Bairstow's method, finding quadratic factor x^2 + u*x + v
//  _p      :   polynomial array, ascending powers [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _p1     :   reduced polynomial (output) [size: _k-2 x 1]
//  _u      :   input: initial estimate for u; output: resulting u
//  _v      :   input: initial estimate for v; output: resulting v
int liquid_poly_findroots_bairstow_recursion(double *     _p,
                                             unsigned int _k,
                                             double *     _p1,
                                             double *     _u,
                                             double *     _v)
{
    // validate length
    if (_k < 3)
        return liquid_error(LIQUID_EICONFIG,"liquid_poly_findroots_bairstow_recursion(), invalid polynomial length: %u", _k);

    float        tol                = 1e-12;    // tolerance before stopping
    unsigned int num_iterations_max =    20;    // maximum iteration count

    // initial estimates for u, v
    double u = *_u;
    double v = *_v;
    //printf("bairstow recursion, u=%12.4e + j*%12.4e, v=%12.4e + j*%12.4e\n",
    //        creal(u), cimag(u), creal(v), cimag(v));
    
    unsigned int n = _k-1;
    double c,d,g,h;
    double q0, q1, q;
    double du, dv;

    // reduced polynomials
    double b[_k];
    double f[_k];
    b[n] = b[n-1] = 0;
    f[n] = f[n-1] = 0;

    int i;
    unsigned int num_iterations = 0;  // current iteration count
    int          rc             = 0;  //
    while (1) {
        // check iteration count
        if (num_iterations == num_iterations_max) {
            // failed to converge
            rc = 1;
            break;
        }
        num_iterations++;

        // update reduced polynomial coefficients
        for (i=n-2; i>=0; i--) {
            b[i] = _p[i+2] - u*b[i+1] - v*b[i+2];
            f[i] =  b[i+2] - u*f[i+1] - v*f[i+2];
        }
        c = _p[1] - u*b[0] - v*b[1];
        g =  b[1] - u*f[0] - v*f[1];
        d = _p[0] - v*b[0];
        h =  b[0] - v*f[0];

        // compute scaling factor
        q0 = v*g*g;
        q1 = h*(h-u*g);
        double metric = fabs(q0+q1);
        if ( metric < tol ) {
            q = 0;
            u *= 0.5;
            v *= 0.5;
            continue;
        } else {
            q  = 1/(v*g*g + h*(h-u*g));
        }
        // compute u, v steps
        du = - q*(-h*c   + g*d);
        dv = - q*(-g*v*c + (g*u-h)*d);

        double step = fabs(du)+fabs(dv);

#if 0
        printf(" %3u : q0=%12.4e, q1=%12.4e, q=%12.4e, metric=%12.4e, u=%8.3f, v=%8.3f, step=%12.4e\n",
                num_iterations, fabs(q0), fabs(q1), fabs(q), metric,
                creal(u), creal(v),
                step);
#endif

#if LIQUID_POLY_FINDROOTS_DEBUG
        // print debugging info
        printf("bairstow [%u] :\n", num_iterations);
        printf("  u     : %12.4e + j*%12.4e\n", creal(u), cimag(u));
        printf("  v     : %12.4e + j*%12.4e\n", creal(v), cimag(v));
        printf("  b     : \n");
        for (i=0; i<n-2; i++)
            printf("      %12.4e + j*%12.4e\n", creal(b[i]), cimag(b[i]));
        printf("  fb    : \n");
        for (i=0; i<n-2; i++)
            printf("      %12.4e + j*%12.4e\n", creal(f[i]), cimag(f[i]));
        printf("  c     : %12.4e + j*%12.4e\n", creal(c), cimag(c));
        printf("  g     : %12.4e + j*%12.4e\n", creal(g), cimag(g));
        printf("  d     : %12.4e + j*%12.4e\n", creal(d), cimag(d));
        printf("  h     : %12.4e + j*%12.4e\n", creal(h), cimag(h));
        printf("  q     : %12.4e + j*%12.4e\n", creal(q), cimag(q));
        printf("  du    : %12.4e + j*%12.4e\n", creal(du), cimag(du));
        printf("  dv    : %12.4e + j*%12.4e\n", creal(dv), cimag(dv));

        printf("  step : %12.4e + j*%12.4e\n", creal(du+dv), cimag(du+dv));
#endif
        // adjust u, v by step size
        u += du;
        v += dv;

        // exit conditions
        if (step < tol)
            break;
    }

    // set resulting reduced polynomial
    for (i=0; i<_k-2; i++)
        _p1[i] = b[i];

    // set output pairs
    *_u = u;
    *_v = v;
    if (rc) {
        printf("liquid_poly_findroots_bairstow_recursion(), failed to converge\n");
        return LIQUID_EINT;
    }

    return LIQUID_OK;
}

// run multiple iterations of Bairstow's method with different starting
// conditions looking for convergence
int liquid_poly_findroots_bairstow_persistent(double *     _p,
                                              unsigned int _k,
                                              double *     _p1,
                                              double *     _u,
                                              double *     _v)
{
    unsigned int i, num_iterations_max = 10;
    for (i=0; i<num_iterations_max; i++) {
        //printf("#\n# persistence %u\n#\n", i);
        if (liquid_poly_findroots_bairstow_recursion(_p, _k, _p1, _u, _v)==0) {
            // success
            return LIQUID_OK;
        } else if (i < num_iterations_max-1) {
            // didn't converge; adjust starting point using consistent and
            // reproduceable starting point
            *_u = cosf((float)i * 1.1f) * expf((float)i * 0.2f);
            *_v = sinf((float)i * 1.1f) * expf((float)i * 0.2f);
        }
    }

    // could not converge
    printf("liquid_poly_findroots_bairstow_persistence(), failed to converge, u=%12.8f, v=%12.8f\n",
            creal(*_u), cimag(*_v));
    return LIQUID_EINT;
}

// compare roots for sorting
int liquid_poly_sort_roots_compare(const void * _a,
                                   const void * _b)
{
    double ar = (double) creal( *((double complex*)_a) );
    double br = (double) creal( *((double complex*)_b) );

    double ai = (double) cimag( *((double complex*)_a) );
    double bi = (double) cimag( *((double complex*)_b) );

    return ar == br ? (ai > bi ? -1 : 1) : (ar > br ? 1 : -1);
}

