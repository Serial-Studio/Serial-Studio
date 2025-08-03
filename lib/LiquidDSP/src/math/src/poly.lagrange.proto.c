/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// Lagrange polynomial methods
//

#include <stdio.h>
#include <string.h>

// Lagrange polynomial exact fit (order _n-1)
int POLY(_fit_lagrange)(T *          _x,
                        T *          _y,
                        unsigned int _n,
                        T *          _p)
{
    unsigned int k=_n-1;    // polynomial order

    // clear polynomial coefficients array
    unsigned int i;
    for (i=0; i<_n; i++)
        _p[i] = 0.;

    // compute roots, gain
    T roots[k];     // polynomial roots
    T c[_n];        // expanded polynomial
    T g;            // gain
    unsigned int j;
    unsigned int n;
    for (i=0; i<_n; i++) {
        n=0;
        g=1.0f;
        for (j=0; j<_n; j++) {
            if (j!=i) {
                roots[n++] =  _x[j];
                g *= (_x[i] - _x[j]);
            }
        }
        g = _y[i] / g;

        // expand roots
        POLY(_expandroots)(roots, k, c);

        for (j=0; j<_n; j++) {
            _p[j] += g * c[j];
        }

#if 0
        // debug/print
        printf("****** %3u : **********************\n", i);
        printf("  g : %12.8f\n", crealf(g));
        printf("  roots:\n");
        for (j=0; j<k; j++)
            printf("  r[%3u] = %12.8f\n", j, crealf(roots[j]));
        printf("  expanded roots:\n");
        for (j=0; j<_n; j++)
            printf("  c[%3u] = %16.6f > %16.6f\n", j, crealf(c[j]), crealf(g*c[j]));
#endif
    }
    return LIQUID_OK;
}

// Lagrange polynomial interpolation
T POLY(_interp_lagrange)(T * _x,
                         T * _y,
                         unsigned int _n,
                         T   _x0)
{
    T y0 = 0.0;     // set output to zero
    T g;            // accumulator
    unsigned int i, j;
    for (i=0; i<_n; i++) {
        g=1.0f;
        for (j=0; j<_n; j++) {
            if (j!=i) {
                g *= (_x0 - _x[j])/(_x[i] - _x[j]);
            }
        }
        y0 += _y[i] * g;
    }

    return y0;
}
// Lagrange polynomial fit (barycentric form)
int POLY(_fit_lagrange_barycentric)(T *         _x,
                                   unsigned int _n,
                                   T *          _w)
{
    // compute barycentric weights (slow method)
    unsigned int j, k;
    for (j=0; j<_n; j++) {
        _w[j] = 1.;
        for (k=0; k<_n; k++) {
            if (j==k)   continue;
            else        _w[j] *= (_x[j] - _x[k]);
        }

        if (_w[j] == 0.0f) _w[j] += 1.0e-9f;
        _w[j] = 1. / _w[j];
    }

    // normalize by _w[0], add minuscule margin to avoid division by zero
    T w0 = _w[0] + 1.0e-9f;
    for (j=0; j<_n; j++)
        _w[j] /= w0;
    return LIQUID_OK;
}

// Lagrange polynomial interpolation (barycentric form)
T POLY(_val_lagrange_barycentric)(T *          _x,
                                  T *          _y,
                                  T *          _w,
                                  T            _x0,
                                  unsigned int _n)
{
    T t0 = 0.;  // numerator sum
    T t1 = 0.;  // denominator sum
    T g;        // _x0 - _x[j]

    // exact fit tolerance
    float tol = 1e-6f;

    unsigned int j;
    for (j=0; j<_n; j++) {
        g = _x0 - _x[j];

        // test for "exact" fit
        if (T_ABS(g) < tol)
            return _y[j];

        t0 += _w[j] * _y[j] / g;
        t1 += _w[j] / g;
    }
    return t0 / t1;
}

