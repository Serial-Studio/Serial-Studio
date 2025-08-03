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
// Design root raised-cosine filter
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "liquid.internal.h"

// Design root-Nyquist raised-cosine filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_rrcos(unsigned int _k,
                        unsigned int _m,
                        float _beta,
                        float _dt,
                        float * _h)
{
    if ( _k < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_rrcos(): k must be greater than 0");
    if ( _m < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_rrcos(): m must be greater than 0");
    if ( (_beta < 0.0f) || (_beta > 1.0f) )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_rrcos(): beta must be in [0,1]");

    unsigned int n;
    float z, t1, t2, t3, t4, T=1.0f;

    float nf, kf, mf;

    unsigned int h_len = 2*_k*_m + 1;

    // Calculate filter coefficients
    for (n=0; n<h_len; n++) {
        nf = (float) n;
        kf = (float) _k;
        mf = (float) _m;

        z = (nf+_dt)/kf-mf;
        t1 = cosf((1+_beta)*M_PI*z);
        t2 = sinf((1-_beta)*M_PI*z);

        // Check for special condition where z equals zero
        if ( fabsf(z) < 1e-5 ) {
            _h[n] = 1 - _beta + 4*_beta/M_PI;
        } else {
            t3 = 1/((4*_beta*z));

            float g = 1-16*_beta*_beta*z*z;
            g *= g;

            // Check for special condition where 16*_beta^2*z^2 equals 1
            if ( g < 1e-5 ) {
                float g1, g2, g3, g4;
                g1 = 1 + 2.0f/M_PI;
                g2 = sinf(0.25f*M_PI/_beta);
                g3 = 1 - 2.0f/M_PI;
                g4 = cosf(0.25f*M_PI/_beta);
                _h[n] = _beta/sqrtf(2.0f)*(g1*g2 + g3*g4);
            } else {
                t4 = 4*_beta/(M_PI*sqrtf(T)*(1-(16*_beta*_beta*z*z)));
                _h[n] = t4*( t1 + (t2*t3) );
            }
        }
    }
    return LIQUID_OK;
}

