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
// 2nd-order iir (infinite impulse response) phase-locked loop filter design
//
// References:
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid.internal.h"

// design 2nd-order IIR filter (active lag)
//          1 + t2 * s
//  F(s) = ------------
//          1 + t1 * s
//
//  _w      :   filter bandwidth
//  _zeta   :   damping factor (1/sqrt(2) suggested)
//  _K      :   loop gain (1000 suggested)
//  _b      :   output feed-forward coefficients [size: 3 x 1]
//  _a      :   output feed-back coefficients [size: 3 x 1]
void iirdes_pll_active_lag(float _w,
                           float _zeta,
                           float _K,
                           float * _b,
                           float * _a)
{
    // validate input
    if (_w <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"iirdes_pll_active_lag(), bandwidth must be greater than 0");
        return;
    } else if (_zeta <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"iirdes_pll_active_lag(), damping factor must be greater than 0");
        return;
    } else if (_K <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"iirdes_pll_active_lag(), gain must be greater than 0");
        return;
    }

    float wn = _w;                  // natural frequency
    float t1 = _K/(wn*wn);          // 
    float t2 = 2*_zeta/wn - 1/_K;   //

    _b[0] = 2*_K*(1.+t2/2.0f);
    _b[1] = 2*_K*2.;
    _b[2] = 2*_K*(1.-t2/2.0f);

    _a[0] =  1 + t1/2.0f;
    _a[1] = -t1;
    _a[2] = -1 + t1/2.0f;
}

// design 2nd-order IIR filter (active PI)
//          1 + t2 * s
//  F(s) = ------------
//           t1 * s
//
//  _w      :   filter bandwidth
//  _zeta   :   damping factor (1/sqrt(2) suggested)
//  _K      :   loop gain (1000 suggested)
//  _b      :   output feed-forward coefficients [size: 3 x 1]
//  _a      :   output feed-back coefficients [size: 3 x 1]
void iirdes_pll_active_PI(float _w,
                          float _zeta,
                          float _K,
                          float * _b,
                          float * _a)
{
    // validate input
    if (_w <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"iirdes_pll_active_PI(), bandwidth must be greater than 0");
        return;
    } else if (_zeta <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"iirdes_pll_active_PI(), damping factor must be greater than 0");
        return;
    } else if (_K <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"iirdes_pll_active_PI(), gain must be greater than 0");
        return;
    }

    // loop filter (active lag)
    float wn = _w;          // natural frequency
    float t1 = _K/(wn*wn);  //
    float t2 = 2*_zeta/wn;  //

    _b[0] = 2*_K*(1.+t2/2.0f);
    _b[1] = 2*_K*2.;
    _b[2] = 2*_K*(1.-t2/2.0f);

    _a[0] =  t1/2.0f;
    _a[1] = -t1;
    _a[2] =  t1/2.0f;
}


