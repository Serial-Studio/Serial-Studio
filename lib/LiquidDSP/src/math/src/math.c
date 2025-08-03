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
// Useful mathematical formulae
//
// References:
//  [Helstrom:1960] Helstrom, C. W., Statistical Theory of Signal
//      Detection. New York: Pergamon Press, 1960
//  [Helstrom:1992] Helstrom, C. W. "Computing the Generalized Marcum Q-
//      Function," IEEE Transactions on Information Theory, vol. 38, no. 4,
//      July, 1992.
//  [Proakis:2001] Proakis, J. Digital Communications. New York:
//      McGraw-Hill, 2001

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

//                    infty
// Q(z) = 1/sqrt(2 pi) int { exp(-u^2/2) du }
//                      z
//
// Q(z) = (1/2)*(1 - erf(z/sqrt(2)))
float liquid_Qf(float _z)
{
    return 0.5f * (1.0f - erff(_z*M_SQRT1_2));
}

// Marcum Q-function
// TODO : check this computation
// [Helstrom:1960], [Proakis:2001], [Helstrom:1992]
#define NUM_MARCUMQ_ITERATIONS 16
float liquid_MarcumQf(int _M,
                      float _alpha,
                      float _beta)
{
#if 0
    // expand as:
    //                               infty
    // Q_M(a,b) = exp(-(a^2+b^2)/2) * sum { (a/b)^k I_k(a*b) }
    //                               k=1-M
    return 0.0f
#else

    // use approximation [Helstrom:1992] (Eq. 25)
    // Q_M(a,b) ~ erfc(x),
    //   x = (b-a-M)/sigma^2,
    //   sigma = M + 2a

    // compute sigma
    float sigma = (float)(_M) + 2.0f*_alpha;

    // compute x
    float x = (_beta - _alpha - (float)_M) / (sigma*sigma);

    // return erfc(x)
    return erfcf(x);
#endif
}

// Marcum Q-function (M=1)
// TODO : check this computation
// [Helstrom:1960], [Proakis:2001]
#define NUM_MARCUMQ1_ITERATIONS 64
float liquid_MarcumQ1f(float _alpha,
                       float _beta)
{
#if 1
    // expand as:                    infty
    // Q_1(a,b) = exp(-(a^2+b^2)/2) * sum { (a/b)^k I_k(a*b) }
    //                                k=0

    float t0 = expf( -0.5f*(_alpha*_alpha + _beta*_beta) );
    float t1 = 1.0f;

    float a_div_b = _alpha / _beta;
    float a_mul_b = _alpha * _beta;

    float y = 0.0f;
    unsigned int k;
    for (k=0; k<NUM_MARCUMQ1_ITERATIONS; k++) {
        // accumulate y
        y += t1 * liquid_besselif((float)k, a_mul_b);

        // update t1
        t1 *= a_div_b;
    }

    return t0 * y;
#else
    
    // call generalized Marcum-Q function with M=1
    return liquid_MarcumQf(1, _alpha, _beta);
#endif
}

// compute sinc(x) = sin(pi*x) / (pi*x)
float sincf(float _x) {
    // _x ~ 0 approximation
    //if (fabsf(_x) < 0.01f)
    //    return expf(-lngammaf(1+_x) - lngammaf(1-_x));

    // _x ~ 0 approximation
    // from : http://mathworld.wolfram.com/SincFunction.html
    // sinc(z) = \prod_{k=1}^{\infty} { cos(\pi z / 2^k) }
    if (fabsf(_x) < 0.01f)
        return cosf(M_PI*_x/2.0f)*cosf(M_PI*_x/4.0f)*cosf(M_PI*_x/8.0f);

    return sinf(M_PI*_x)/(M_PI*_x);
}

// next power of 2 : y = ceil(log2(_x))
unsigned int liquid_nextpow2(unsigned int _x)
{
    if (_x == 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_nextpow2(), input must be greater than zero");
        return 0;
    }

    _x--;
    unsigned int n=0;
    while (_x > 0) {
        _x >>= 1;
        n++;
    }
    return n;
}

// (n choose k) = n! / ( k! (n-k)! )
float liquid_nchoosek(unsigned int _n, unsigned int _k)
{
    // 
    if (_k > _n) {
        liquid_error(LIQUID_EICONFIG,"liquid_nchoosek(), _k cannot exceed _n");
        return 0.0f;
    } else if (_k == 0 || _k == _n) {
        return 1.0f;
    }

    // take advantage of symmetry and take larger value
    if (_k < _n/2)
        _k = _n - _k;

    // use lngamma() function when _n is large
    if (_n > 12) {
        double t0 = lgamma((double)_n + 1.0f);
        double t1 = lgamma((double)_n - (double)_k + 1.0f);
        double t2 = lgamma((double)_k + 1.0f);

        return round(exp( t0 - t1 - t2 ));
    }

    // old method
    float rnum=1, rden=1;
    unsigned int i;
    for (i=_n; i>_k; i--)
        rnum *= i;
    for (i=1; i<=_n-_k; i++)
        rden *= i;
    return rnum / rden;
}

