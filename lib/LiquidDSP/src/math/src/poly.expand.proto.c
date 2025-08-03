/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
// polynomial expansion methods
//

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

// expands the polynomial:
//  P_n(x) = (1+x)^n
// as
//  P_n(x) = p[0] + p[1]*x + p[2]*x^2 + ... + p[n]x^n
// NOTE: _p has order n=m+k (array is length n+1)
int POLY(_expandbinomial)(unsigned int _n,
                          T *          _c)
{
    // no roots; return zero
    if (_n == 0) {
        _c[0] = 0.;
        return LIQUID_OK;
    }

    int i, j;
    // initialize coefficients array to [1,0,0,....0]
    for (i=0; i<=_n; i++)
        _c[i] = (i==0) ? 1 : 0;

    // iterative polynomial multiplication
    for (i=0; i<_n; i++) {
        for (j=i+1; j>0; j--)
            _c[j] = _c[j] + _c[j-1];
    }
    // assert(_c[0]==1.0f);
    return LIQUID_OK;
}

// expands the polynomial:
//  P_n(x) = (1+x)^m * (1-x)^k
// as
//  P_n(x) = p[0] + p[1]*x + p[2]*x^2 + ... + p[n]x^n
// NOTE: _p has order n=m+k (array is length n+1)
int POLY(_expandbinomial_pm)(unsigned int _m,
                             unsigned int _k,
                             T *          _c)
{
    unsigned int n = _m + _k;

    // no roots; return zero
    if (n == 0) {
        _c[0] = 0.;
        return LIQUID_OK;
    }

    int i, j;
    // initialize coefficients array to [1,0,0,....0]
    for (i=0; i<=n; i++)
        _c[i] = (i==0) ? 1 : 0;

    // iterative polynomial multiplication (1+x)
    for (i=0; i<_m; i++) {
        for (j=i+1; j>0; j--)
            _c[j] = _c[j] + _c[j-1];
    }

    // iterative polynomial multiplication (1-x)
    for (i=_m; i<n; i++) {
        for (j=i+1; j>0; j--)
            _c[j] = _c[j] - _c[j-1];
    }
    // assert(_c[0]==1.0f);
    return LIQUID_OK;
}

#if 0
// expands the polynomial:
//  (1+x*a[0])*(1+x*a[1]) * ... * (1+x*a[n-1])
// as
//  c[0] + c[1]*x + c[2]*x^2 + ... + c[n]*x^n
int POLY(_expandbinomial)(T *          _a,
                          unsigned int _n,
                          T *          _c)
{
    // no roots; return zero
    if (_n == 0) {
        _c[0] = 0.;
        return LIQUID_OK;
    }

    int i, j;
    // initialize coefficients array to [1,0,0,....0]
    for (i=0; i<=_n; i++)
        _c[i] = (i==0) ? 1 : 0;

    // iterative polynomial multiplication
    for (i=0; i<_n; i++) {
        for (j=i+1; j>0; j--)
            _c[j] = _a[i]*_c[j] + _c[j-1];

        _c[j] *= _a[i];
    }

    // flip values
    unsigned int r = (_n+1) % 2;
    unsigned int L = (_n+1-r)/2;
    T tmp;
    for (i=0; i<L; i++) {
        tmp = _c[i];
        _c[i] = _c[_n-i];
        _c[_n-i] = tmp;
    }

    // assert(_c[0]==1.0f);
    return LIQUID_OK;
}
#endif

// Perform root expansion on the polynomial
//  "P_n(x) = (x-r[0]) * (x-r[1]) * ... * (x-r[n-1])"
// as
//  "P_n(x) = p[0] + p[1]*x + ... + p[n]*x^n"
// where r[0],r[1],...,r[n-1] are the roots of P_n(x)
// NOTE: _p has order _n (array is length _n+1)
//  _r      : roots of polynomial [size: _n x 1]
//  _n      : number of roots in polynomial
//  _p      : polynomial coefficients [size: _n+1 x 1]
int POLY(_expandroots)(T *          _r,
                       unsigned int _n,
                       T *          _p)
{
    // no roots; return zero
    if (_n == 0) {
        _p[0] = 0.;
        return LIQUID_OK;
    }

    int i, j;
    // initialize coefficients array to [1,0,0,....0]
    for (i=0; i<=_n; i++)
        _p[i] = (i==0) ? 1 : 0;

    // iterative polynomial multiplication
    for (i=0; i<_n; i++) {
        for (j=i+1; j>0; j--)
            _p[j] = -_r[i]*_p[j] + _p[j-1];

        _p[j] *= -_r[i];
    }

    // assert(c[_n]==1.0f)
    return LIQUID_OK;
}

// Perform root expansion on the polynomial
//  "P_n(x) = (x*b[0]-a[0]) * (x*b[1]-a[1]) * ... * (x*b[n-1]-a[n-1])"
// as
//  "P_n(x) = p[0] + p[1]*x + ... + p[n]*x^n"
// NOTE: _p has order _n (array is length _n+1)
//  _a      : subtractant of polynomial rotos [size: _n x 1]
//  _b      : multiplicant of polynomial roots [size: _n x 1]
//  _n      : number of roots in polynomial
//  _p      : polynomial coefficients [size: _n+1 x 1]
int POLY(_expandroots2)(T *          _a,
                        T *          _b,
                        unsigned int _n,
                        T *          _p)
{
    unsigned int i;

    // factor _b[i] from each root : (x*b - a) = (x - a/b)*b
    T g = 1.0;
    T r[_n];
    for (i=0; i<_n; i++) {
        g *= -_b[i];
        r[i] = _a[i] / _b[i];
    }

    // expand new root set
    POLY(_expandroots)(r, _n, _p);

    // multiply by gain
    for (i=0; i<_n+1; i++)
        _p[i] *= g;
    return LIQUID_OK;
}

// expands the multiplication of two polynomials
//
//  (a[0] + a[1]*x + a[2]*x^2 + ...) * (b[0] + b[1]*x + b[]*x^2 + ...)
// as
//  c[0] + c[1]*x + c[2]*x^2 + ... + c[n]*x^n
//
// where order(c)  = order(a)  + order(b) + 1
//    :: length(c) = length(a) + length(b) - 1
//
//  _a          :   1st polynomial coefficients (length is _order_a+1)
//  _order_a    :   1st polynomial order
//  _b          :   2nd polynomial coefficients (length is _order_b+1)
//  _order_b    :   2nd polynomial order
//  _c          :   output polynomial coefficients (length is _order_a + _order_b + 1)
int POLY(_mul)(T *          _a,
               unsigned int _order_a,
               T *          _b,
               unsigned int _order_b,
               T *          _c)
{
    unsigned int na = _order_a + 1;
    unsigned int nb = _order_b + 1;
    unsigned int nc = na + nb - 1;
    unsigned int i;
    for (i=0; i<nc; i++)
        _c[i] = 0.0f;

    unsigned int j;
    for (i=0; i<na; i++) {
        for (j=0; j<nb; j++) {
            _c[i+j] += _a[i]*_b[j];
        }
    }
    return LIQUID_OK;
}

