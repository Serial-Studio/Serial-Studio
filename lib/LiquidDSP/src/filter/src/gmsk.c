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
// Finite impulse response GMSK transmit/receive filter design
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// Design GMSK transmit filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_gmsktx(unsigned int _k,
                         unsigned int _m,
                         float        _beta,
                         float        _dt,
                         float *      _h)
{
    // validate input
    if ( _k < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_gmsktx(): k must be greater than 0");
    if ( _m < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_gmsktx(): m must be greater than 0");
    if ( (_beta < 0.0f) || (_beta > 1.0f) )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_gmsktx(): beta must be in [0,1]");

    // derived values
    unsigned int h_len = 2*_k*_m+1;

    // compute filter coefficients
    unsigned int i;
    float t;
    float c0 = 1.0f / sqrtf(logf(2.0f));
    for (i=0; i<h_len; i++) {
        t = (float)i/(float)(_k)-(float)(_m) + _dt;

        _h[i] = liquid_Qf(2*M_PI*_beta*(t-0.5f)*c0) -
                liquid_Qf(2*M_PI*_beta*(t+0.5f)*c0);
    }

    // normalize filter coefficients such that the filter's
    // integral is pi/2
    float e = 0.0f;
    for (i=0; i<h_len; i++)
        e += _h[i];
    for (i=0; i<h_len; i++)
        _h[i] *= M_PI / (2.0f * e);
    for (i=0; i<h_len; i++)
        _h[i] *= (float)_k;
    return LIQUID_OK;
}

// Design GMSK receive filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_gmskrx(unsigned int _k,
                         unsigned int _m,
                         float        _beta,
                         float        _dt,
                         float *      _h)
{
    // validate input
    if ( _k < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_gmskrx(): k must be greater than 0");
    if ( _m < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_gmskrx(): m must be greater than 0");
    if ( (_beta < 0.0f) || (_beta > 1.0f) )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_gmskrx(): beta must be in [0,1]");

    unsigned int k = _k;
    unsigned int m = _m;
    float BT = _beta;

    // internal options
    float beta = BT;                // prototype filter cut-off
    float delta = 1e-3f;            // filter design correction factor
    liquid_firfilt_type prototype = LIQUID_FIRFILT_KAISER;    // Nyquist prototype

    unsigned int i;

    // derived values
    unsigned int h_len = 2*k*m+1;   // filter length

    // arrays
    float ht[h_len];         // transmit filter coefficients
    float hr[h_len];         // receive filter coefficients

    // design transmit filter
    liquid_firdes_gmsktx(k,m,BT,0.0f,ht);

    //
    // start of filter design procedure
    //

    // 'internal' arrays
    float h_primef[h_len];          // temporary buffer for real 'prototype' coefficients
    float g_primef[h_len];          // temporary buffer for real 'gain' coefficient

    float complex h_tx[h_len];      // impulse response of transmit filter
    float complex h_prime[h_len];   // impulse response of 'prototype' filter
    float complex g_prime[h_len];   // impulse response of 'gain' filter
    float complex h_hat[h_len];     // impulse response of receive filter
    
    float complex H_tx[h_len];      // frequency response of transmit filter
    float complex H_prime[h_len];   // frequency response of 'prototype' filter
    float complex G_prime[h_len];   // frequency response of 'gain' filter
    float complex H_hat[h_len];     // frequency response of receive filter

    // create 'prototype' matched filter
    liquid_firdes_prototype(prototype,k,m,beta,0.0f,h_primef);

    // create 'gain' filter to improve stop-band rejection
    float fc = (0.7f + 0.1*beta) / (float)k;
    float as = 60.0f;
    liquid_firdes_kaiser(h_len, fc, as, 0.0f, g_primef);

    // copy to fft input buffer, shifting appropriately
    for (i=0; i<h_len; i++) {
        h_prime[i] = h_primef[ (i+k*m)%h_len ];
        g_prime[i] = g_primef[ (i+k*m)%h_len ];
        h_tx[i]    = ht[       (i+k*m)%h_len ];
    }

    // run ffts
    fft_run(h_len, h_prime, H_prime, LIQUID_FFT_FORWARD, 0);
    fft_run(h_len, g_prime, G_prime, LIQUID_FFT_FORWARD, 0);
    fft_run(h_len, h_tx,    H_tx,    LIQUID_FFT_FORWARD, 0);

    // find minimum of responses
    float H_tx_min = 0.0f;
    float H_prime_min = 0.0f;
    float G_prime_min = 0.0f;
    for (i=0; i<h_len; i++) {
        if (i==0 || crealf(H_tx[i])    < H_tx_min)    H_tx_min    = crealf(H_tx[i]);
        if (i==0 || crealf(H_prime[i]) < H_prime_min) H_prime_min = crealf(H_prime[i]);
        if (i==0 || crealf(G_prime[i]) < G_prime_min) G_prime_min = crealf(G_prime[i]);
    }

    // compute 'prototype' response, removing minima, and add correction factor
    for (i=0; i<h_len; i++) {
        // compute response necessary to yield prototype response (not exact, but close)
        H_hat[i] = crealf(H_prime[i] - H_prime_min + delta) / crealf(H_tx[i] - H_tx_min + delta);

        // include additional term to add stop-band suppression
        H_hat[i] *= crealf(G_prime[i] - G_prime_min) / crealf(G_prime[0]);
    }

    // compute ifft and copy response
    fft_run(h_len, H_hat, h_hat, LIQUID_FFT_BACKWARD, 0);
    for (i=0; i<h_len; i++)
        hr[i] = crealf( h_hat[(i+k*m+1)%h_len] ) / (float)(k*h_len);

    // copy result, scaling by (samples/symbol)^2
    for (i=0; i<h_len; i++)
        _h[i] = hr[i]*_k*_k;

    return LIQUID_OK;
}

