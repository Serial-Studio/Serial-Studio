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
// Elliptic filter design
//
//  [Orfanidis:2006] Sophocles J. Orfanidis, "Lecture Notes on
//      Elliptic Filter Design." 2006
//  [Orfanidis:2005] Sophocles J. Orfanidis, source code available
//      on www.ece.rutgers.edu/~orfanidi/hpeq

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.internal.h"

#define LIQUID_DEBUG_ELLIP_PRINT   0

// landenf()
//
// Compute the Landen transformation of _k over _n iterations,
//  k[n] = (k[n-1] / (1+k'[n-1]))^2
// where
//  k'[n-1] = sqrt(1-k[n-1]^2)
//
//  _k      :   elliptic modulus
//  _n      :   number of iterations
//  _v      :   sequence of decreasing moduli [size: _n]
int landenf(float        _k,
            unsigned int _n,
            float *      _v)
{
    unsigned int i;

    float k = _k;       // k[n]
    float kp;           // k'[n]
    for (i=0; i<_n; i++) {
        kp = sqrtf(1 - k*k);
        k  = (1 - kp)/(1 + kp);
        _v[i] = k;
    }
    return LIQUID_OK;
}

// ellipkf()
//
// compute elliptic integral K(k) for _n recursions
//
//  _k      :   elliptic modulus
//  _n      :   number of iterations
//  _K      :   complete elliptic integral (modulus k)
//  _Kp     :   complete elliptic integral (modulus k')
int ellipkf(float         _k,
             unsigned int _n,
             float *      _K,
             float *      _Kp)
{
    // define range for k due to machine precision
    float kmin = 4e-4f;
    float kmax = sqrtf(1-kmin*kmin);
    
    float K;
    float Kp;

    float kp = sqrtf(1-_k*_k);

    //printf("ellipkf: k = %16.8e, kp = %16.8e\n", _k, kp);

    // Floating point resolution limits the range of the
    // computation of K on input _k [Orfanidis:2005]
    if (_k > kmax) {
        // input exceeds maximum extreme for this precision; use
        // approximation
        float L = -logf(0.25f*kp);
        K = L + 0.25f*(L-1)*kp*kp;
    } else {
        float v[_n];
        landenf(_k,_n,v);
        K = M_PI * 0.5f;
        unsigned int i;
        for (i=0; i<_n; i++)
            K *= (1 + v[i]);
    }

    if (_k < kmin) {
        // input exceeds minimum extreme for this precision; use
        // approximation
        float L = -logf(_k*0.25f);
        Kp = L + 0.25f*(L-1)*_k*_k;
    } else {
        float vp[_n];
        landenf(kp,_n,vp);
        Kp = M_PI * 0.5f;
        unsigned int i;
        for (i=0; i<_n; i++)
            Kp *= (1 + vp[i]);
    }

    // set return values
    *_K  = K;
    *_Kp = Kp;
    return LIQUID_OK;
}

// ellipdegf()
//
// Compute elliptic degree using _n iterations
//
//  _N      :   analog filter order
//  _k1     :   elliptic modulus for stop-band, ep/ep1
//  _n      :   number of Landen iterations
float ellipdegf(float        _N,
                float        _k1,
                unsigned int _n)
{
    // compute K1, K1p from _k1
    float K1, K1p;
    ellipkf(_k1,_n,&K1,&K1p);

    // compute q1 from K1, K1p
    float q1 = expf(-M_PI*K1p/K1);

    // compute q from q1
    float q = powf(q1,1.0f/_N);

    // expand numerator, denominator
    unsigned int m;
    float b = 0.0f;
    for (m=0; m<_n; m++)
        b += powf(q,(float)(m*(m+1)));
    float a = 0.0f;
    for (m=1; m<_n; m++)
        a += powf(q,(float)(m*m));

    float g = b / (1.0f + 2.0f*a);
    float k = 4.0f*sqrtf(q)*g*g;

#if LIQUID_DEBUG_ELLIP_PRINT
    printf("ellipdegf(N=%f, k1=%f, n=%u):\n", _N, _k1, _n);
    printf("  K1        :   %12.4e;\n", K1);
    printf("  K1p       :   %12.4e;\n", K1p);
    printf("  q1        :   %12.4e;\n", q1);
    printf("  q         :   %12.4e;\n", q);
#endif

    return k;
}

// ellip_cdf()
//
// complex elliptic cd() function (Jacobian elliptic cosine)
//
//  _u      :   vector in the complex u-plane
//  _k      :   elliptic modulus (0 <= _k < 1)
//  _n      :   number of Landen iterations (typically 5-6)
float complex ellip_cdf(float complex _u,
                        float         _k,
                        unsigned int  _n)
{
    float complex wn = ccosf(_u*M_PI*0.5f);
    float v[_n];
    landenf(_k,_n,v);
    unsigned int i;
    for (i=_n; i>0; i--) {
        wn = (1 + v[i-1])*wn / (1 + v[i-1]*wn*wn);
    }
    return wn;
}

// ellip_snf()
//
// complex elliptic sn() function (Jacobian elliptic sine)
//
//  _u      :   vector in the complex u-plane
//  _k      :   elliptic modulus (0 <= _k < 1)
//  _n      :   number of Landen iterations (typically 5-6)
float complex ellip_snf(float complex _u,
                        float         _k,
                        unsigned int  _n)
{
    float complex wn = csinf(_u*M_PI*0.5f);
    float v[_n];
    landenf(_k,_n,v);
    unsigned int i;
    for (i=_n; i>0; i--) {
        wn = (1 + v[i-1])*wn / (1 + v[i-1]*wn*wn);
    }
    return wn;
}

// ellip_acdf()
//
// complex elliptic inverse cd() function (Jacobian elliptic
// arc cosine)
//
//  _w      :   vector in the complex u-plane
//  _k      :   elliptic modulus (0 <= _k < 1)
//  _n      :   number of Landen iterations (typically 5-6)
float complex ellip_acdf(float complex _w,
                         float         _k,
                         unsigned int  _n)
{
    float v[_n];
    landenf(_k,_n,v);
    float v1;

    float complex w = _w;
    unsigned int i;
    for (i=0; i<_n; i++) {
        v1 = (i==0) ? _k : v[i-1];
        w = w / (1 + liquid_csqrtf(1 - w*w*v1*v1)) * 2.0 / (1+v[i]);
        //printf("  w[%3u] = %12.8f + j*%12.8f\n", i, crealf(w), cimagf(w));
    }

    float complex u = liquid_cacosf(w) * 2.0 / M_PI;
    //printf("  u = %12.8f + j*%12.8f\n", crealf(u), cimagf(u));

#if 0
    float K, Kp;
    ellipkf(_k, _n, &K, &Kp);
    float R = Kp / K;
#endif
    return u;
}

// ellip_asnf()
//
// complex elliptic inverse sn() function (Jacobian elliptic
// arc sine)
//
//  _w      :   vector in the complex u-plane
//  _k      :   elliptic modulus (0 <= _k < 1)
//  _n      :   number of Landen iterations (typically 5-6)
float complex ellip_asnf(float complex _w,
                         float         _k,
                         unsigned int  _n)
{
    return 1.0 - ellip_acdf(_w,_k,_n);
}

// ellip_azpkf()
//
// Compute analog zeros, poles, gain of low-pass elliptic
// filter, grouping complex conjugates together. If
// the filter order is odd, the single real pole is at the
// end of the array.
//  _n      :   filter order
//  _ep     :   epsilon_p, related to pass-band ripple
//  _es     :   epsilon_s, related to stop-band ripple
//  _za     :   output analog zeros [length: floor(_n/2)]
//  _pa     :   output analog poles [length: _n]
//  _ka     :   output analog gain
int ellip_azpkf(unsigned int    _n,
                float           _ep,
                float           _es,
                float complex * _za,
                float complex * _pa,
                float complex * _ka)
{
    // filter specifications
    float fp = 1.0f / (2.0f * M_PI);    // pass-band cutoff
    float fs = 1.1*fp;                  // stop-band cutoff
    //float Gp = 1/sqrtf(1 + _ep*_ep);    // pass-band gain
    //float Gs = 1/sqrtf(1 + _es*_es);    // stop-band gain

    // number of iterations for elliptic integral
    // approximations
    unsigned int n=7;

    float Wp = 2*M_PI*fp;
    float Ws = 2*M_PI*fs;

    // ripples passband, stopband
    float ep = _ep;
    float es = _es;
#if LIQUID_DEBUG_ELLIP_PRINT
    printf("ep, es      : %12.8f, %12.8f\n", _ep, _es);
#endif

    float k  = Wp/Ws;           // 0.8889f;
    float k1 = ep/es;           // 0.0165f;
#if LIQUID_DEBUG_ELLIP_PRINT
    printf("k           : %12.4e\n", k);
    printf("k1          : %12.4e\n", k1);
#endif

    float K,  Kp;
    float K1, K1p;
    ellipkf(k, n, &K,  &Kp);    // K  = 2.23533416, Kp  = 1.66463780
    ellipkf(k1,n, &K1, &K1p);   // K1 = 1.57090271, K1p = 5.49361753
#if LIQUID_DEBUG_ELLIP_PRINT
    printf("K,  Kp      : %12.8f, %12.8f\n", K,  Kp);
    printf("K1, K1p     : %12.8f, %12.8f\n", K1, K1p);
#endif

    float N      = (float)_n;       // ceilf(Nexact) = 5
#if LIQUID_DEBUG_ELLIP_PRINT
    float Nexact = (K1p/K1)/(Kp/K); // 4.69604063
    printf("N (exact)   : %12.8f\n", Nexact);
    printf("N           : %12.8f\n", N);
#endif

    k = ellipdegf(N,k1,n);      // 0.91427171
#if LIQUID_DEBUG_ELLIP_PRINT
    printf("k           : %12.4e\n", k);
#endif

#if LIQUID_DEBUG_ELLIP_PRINT
    float fs_new = fp/k;        // 4.37506723
    printf("fs_new      : %12.8f\n", fs_new);
#endif

    unsigned int L = (unsigned int)(floorf(N/2.0f)); // 2
    unsigned int r = ((unsigned int)N) % 2;
    float u[L];
    unsigned int i;
    for (i=0; i<L; i++) {
        float t = (float)i + 1.0f;
        u[i] = (2.0f*t - 1.0f)/N;
#if LIQUID_DEBUG_ELLIP_PRINT
        printf("u[%3u]      : %12.8f\n", i, u[i]);
#endif
    }
    float complex zeta[L];
    for (i=0; i<L; i++) {
        zeta[i] = ellip_cdf(u[i],k,n);
#if LIQUID_DEBUG_ELLIP_PRINT
        printf("zeta[%3u]   : %12.8f + j*%12.8f\n", i, crealf(zeta[i]), cimagf(zeta[i]));
#endif
    }

    // compute filter zeros
    float complex za[L];
    for (i=0; i<L; i++) {
        za[i] = _Complex_I * Wp / (k*zeta[i]);
#if LIQUID_DEBUG_ELLIP_PRINT
        printf("za[%3u]     : %12.8f + j*%12.8f\n", i, crealf(za[i]), cimagf(za[i]));
#endif
    }

    float complex v0 = -_Complex_I*ellip_asnf(_Complex_I/ep, k1, n)/N;
#if LIQUID_DEBUG_ELLIP_PRINT
    printf("v0          : %12.8f + j*%12.8f\n", crealf(v0), cimagf(v0));
#endif

    float complex pa[L];
    for (i=0; i<L; i++) {
        pa[i] = Wp*_Complex_I*ellip_cdf(u[i]-_Complex_I*v0, k, n);
#if LIQUID_DEBUG_ELLIP_PRINT
        printf("pa[%3u]     : %12.8f + j*%12.8f\n", i, crealf(pa[i]), cimagf(pa[i]));
#endif
    }
    float complex pa0 = Wp * _Complex_I*ellip_snf(_Complex_I*v0, k, n);
#if LIQUID_DEBUG_ELLIP_PRINT
    printf("pa0         : %12.8f + j*%12.8f\n", crealf(pa0), cimagf(pa0));
#endif

    // compute poles
    unsigned int t=0;
    for (i=0; i<L; i++) {
        _pa[t++] =       pa[i];
        _pa[t++] = conjf(pa[i]);
    }
    if (r) _pa[t++] = pa0;
    if (t != _n)
        return liquid_error(LIQUID_EINT,"ellip_azpkf(), invalid derived order (poles)");

    t=0;
    for (i=0; i<L; i++) {
        _za[t++] =       za[i];
        _za[t++] = conjf(za[i]);
    }
    if (t != 2*L)
        return liquid_error(LIQUID_EINT,"ellip_azpkf(), invalid derived order (zeros)");

    // compute gain
    *_ka = r ? 1.0f : 1.0f / sqrtf(1.0f + _ep*_ep);
    for (i=0; i<_n; i++)
        *_ka *= _pa[i];
    for (i=0; i<2*L; i++)
        *_ka /= _za[i];
    return LIQUID_OK;
}


