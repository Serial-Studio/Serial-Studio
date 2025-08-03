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
// fft_dft.c : definitions for regular, slow DFTs
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.internal.h"

// create FFT plan for regular DFT
//  _nfft   :   FFT size
//  _x      :   input array [size: _nfft x 1]
//  _y      :   output array [size: _nfft x 1]
//  _dir    :   fft direction: {LIQUID_FFT_FORWARD, LIQUID_FFT_BACKWARD}
//  _method :   fft method
FFT(plan) FFT(_create_plan_dft)(unsigned int _nfft,
                                TC *         _x,
                                TC *         _y,
                                int          _dir,
                                int          _flags)
{
    // allocate plan and initialize all internal arrays to NULL
    FFT(plan) q = (FFT(plan)) malloc(sizeof(struct FFT(plan_s)));

    q->nfft      = _nfft;
    q->x         = _x;
    q->y         = _y;
    q->flags     = _flags;
    q->type      = (_dir == LIQUID_FFT_FORWARD) ? LIQUID_FFT_FORWARD : LIQUID_FFT_BACKWARD;
    q->direction = (_dir == LIQUID_FFT_FORWARD) ? LIQUID_FFT_FORWARD : LIQUID_FFT_BACKWARD;
    q->method    = LIQUID_FFT_METHOD_DFT;
        
    q->data.dft.twiddle = NULL;
    q->data.dft.dotprod = NULL;

    // check size, use specific codelet for small DFTs
    if      (q->nfft == 2) q->execute = FFT(_execute_dft_2);
    else if (q->nfft == 3) q->execute = FFT(_execute_dft_3);
    else if (q->nfft == 4) q->execute = FFT(_execute_dft_4);
    else if (q->nfft == 5) q->execute = FFT(_execute_dft_5);
    else if (q->nfft == 6) q->execute = FFT(_execute_dft_6);
    else if (q->nfft == 7) q->execute = FFT(_execute_dft_7);
    else if (q->nfft == 8) q->execute = FFT(_execute_dft_8);
    else if (q->nfft ==16) q->execute = FFT(_execute_dft_16);
    else {
        q->execute = FFT(_execute_dft);

        // initialize twiddle factors
        q->data.dft.twiddle = (TC *) malloc(q->nfft * sizeof(TC));

        // create dotprod objects
        q->data.dft.dotprod = (DOTPROD()*) malloc(q->nfft * sizeof(DOTPROD()));
        
        // create dotprod objects
        // twiddles: exp(-j*2*pi*W/n), W=
        //  0   0   0   0   0...
        //  0   1   2   3   4...
        //  0   2   4   6   8...
        //  0   3   6   9   12...
        //  ...
        // Note that first row/column is zero, no multiplication necessary.
        // Create dotprod for first row anyway because it's still faster...
        unsigned int i;
        unsigned int k;
        T d = (q->direction == LIQUID_FFT_FORWARD) ? -1.0 : 1.0;
        for (i=0; i<q->nfft; i++) {
            // initialize twiddle factors
            // NOTE: no need to compute first twiddle because exp(-j*2*pi*0) = 1
            for (k=1; k<q->nfft; k++)
                q->data.dft.twiddle[k-1] = cexpf(_Complex_I*d*2*M_PI*(T)(k*i) / (T)(q->nfft));

            // create dotprod object
            q->data.dft.dotprod[i] = DOTPROD(_create)(q->data.dft.twiddle, q->nfft-1);
        }
    }

    return q;
}

// destroy FFT plan
int FFT(_destroy_plan_dft)(FFT(plan) _q)
{
    // free twiddle factors
    if (_q->data.dft.twiddle != NULL)
        free(_q->data.dft.twiddle);

    // free dotprod objects
    if (_q->data.dft.dotprod != NULL) {
        unsigned int i;
        for (i=0; i<_q->nfft; i++)
            DOTPROD(_destroy)(_q->data.dft.dotprod[i]);

        // free dotprod array
        free(_q->data.dft.dotprod);
    }

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// execute DFT (slow but functionally correct)
int FFT(_execute_dft)(FFT(plan) _q)
{
    unsigned int i;
    unsigned int nfft = _q->nfft;

#if 0
    // DC value is sum of input
    _q->y[0] = _q->x[0];
    for (i=1; i<nfft; i++)
        _q->y[0] += _q->x[i];
    
    // compute remaining DFT values
    unsigned int k;
    for (i=1; i<nfft; i++) {
        _q->y[i] = _q->x[0];
        for (k=1; k<nfft; k++) {
            _q->y[i] += _q->x[k] * _q->data.dft.twiddle[(i*k)%_q->nfft];
        }
    }
#else
    // use vector dot products
    // NOTE: no need to compute first multiplication because exp(-j*2*pi*0) = 1
    for (i=0; i<nfft; i++) {
        DOTPROD(_execute)(_q->data.dft.dotprod[i], &_q->x[1], &_q->y[i]);
        _q->y[i] += _q->x[0];
    }
#endif
    return LIQUID_OK;
}

// 
// codelets for small DFTs
//

// 
int FFT(_execute_dft_2)(FFT(plan) _q)
{
    _q->y[0] = _q->x[0] + _q->x[1];
    _q->y[1] = _q->x[0] - _q->x[1];
    return LIQUID_OK;
}

//
int FFT(_execute_dft_3)(FFT(plan) _q)
{
#if 0
    // NOTE: not as fast as other method, but perhaps useful for
    // fixed-point algorithm
    //  x = a + jb
    //  y = c + jd
    // We want to compute both x*y and x*conj(y) with as few
    // multiplications as possible. If we define
    //  k1 = a*(c+d);
    //  k2 = d*(a+b);
    //  k3 = c*(b-a);
    //  k4 = b*(c+d);
    // then
    //  x*y       = (k1-k2) + j(k1+k3)
    //  x*conj(y) = (k4-k3) + j(k4-k2)
    T a,  b; // c=real(g)=-0.5, d=imag(g)=-sqrt(3)/2
    T k1, k2, k3, k4;

    // compute both _q->x[1]*g and _q->x[1]*conj(g) with only 4 real multiplications
    a = crealf(_q->x[1]);
    b = cimagf(_q->x[1]);
    //k1 = a*(-0.5f + -0.866025403784439f);
    k1 = -1.36602540378444f*a;
    k2 = -0.866025403784439f*(    a + b);
    k3 =               -0.5f*(    b - a);
    //k4 =                   b*(-0.5f + -0.866025403784439f);
    k4 = -1.36602540378444f*b;

    TC ta1 = (k1-k2) + _Complex_I*(k1+k3);   // 
    TC tb1 = (k4-k3) + _Complex_I*(k4-k2);   // 
    
    // compute both _q->x[2]*g and _q->x[2]*conj(g) with only 4 real multiplications
    a = crealf(_q->x[2]);
    b = cimagf(_q->x[2]);
    //k1 = a*(-0.5f + -0.866025403784439f);
    k1 = -1.36602540378444f*a;
    k2 = -0.866025403784439f*(    a + b);
    k3 =               -0.5f*(    b - a);
    //k4 =                   b*(-0.5f + -0.866025403784439f);
    k4 = -1.36602540378444f*b;

    TC ta2 = (k1-k2) + _Complex_I*(k1+k3);   // 
    TC tb2 = (k4-k3) + _Complex_I*(k4-k2);   // 
    
    // set return values
    _q->y[0] = _q->x[0] + _q->x[1] + _q->x[2];
    if (_q->direction == LIQUID_FFT_FORWARD) {
        _q->y[1] = _q->x[0] + ta1 + tb2;
        _q->y[2] = _q->x[0] + tb1 + ta2;
    } else {
        _q->y[1] = _q->x[0] + tb1 + ta2;
        _q->y[2] = _q->x[0] + ta1 + tb2;
    }
#else
    TC g  = -0.5f - _Complex_I*0.866025403784439; // sqrt(3)/2

    _q->y[0] = _q->x[0] + _q->x[1]          + _q->x[2];
    TC ta    = _q->x[0] + _q->x[1]*g        + _q->x[2]*conjf(g);
    TC tb    = _q->x[0] + _q->x[1]*conjf(g) + _q->x[2]*g;

    // set return values
    if (_q->direction == LIQUID_FFT_FORWARD) {
        _q->y[1] = ta;
        _q->y[2] = tb;
    } else {
        _q->y[1] = tb;
        _q->y[2] = ta;
    }
#endif
    return LIQUID_OK;
}

//
int FFT(_execute_dft_4)(FFT(plan) _q)
{
    TC yp;
    TC * x = _q->x;
    TC * y = _q->y;

    // index reversal
    y[0] = x[0];
    y[1] = x[2];
    y[2] = x[1];
    y[3] = x[3];

    // k0 = 0, k1=1
    yp = y[1];
    y[1] = y[0] - yp;
    y[0] = y[0] + yp;

    // k0 = 2, k1=3
    yp = y[3];
    y[3] = y[2] - yp;
    y[2] = y[2] + yp;

    // k0 = 0, k1=2
    yp = y[2];
    y[2] = y[0] - yp;
    y[0] = y[0] + yp;

    // k0 = 1, k1=3
    yp = cimagf(y[3]) - _Complex_I*crealf(y[3]);
    if (_q->direction == LIQUID_FFT_BACKWARD)
        yp = -yp;
    y[3] = y[1] - yp;
    y[1] = y[1] + yp;
    return LIQUID_OK;
}

//
int FFT(_execute_dft_5)(FFT(plan) _q)
{
    TC * x = _q->x;
    TC * y = _q->y;

    // DC value is sum of inputs
    y[0] = x[0] + x[1] + x[2] + x[3] + x[4];

    // exp(-j*2*pi*1/5)
    TC g0 =  0.309016994374947 - 0.951056516295154*_Complex_I;

    // exp(-j*2*pi*2/5)
    TC g1 = -0.809016994374947 - 0.587785252292473*_Complex_I;

    if (_q->direction == LIQUID_FFT_BACKWARD) {
        g0 = conjf(g0);
        g1 = conjf(g1);
    }
    TC g0_conj = conjf(g0);
    TC g1_conj = conjf(g1);

    y[1] = x[0] + x[1]*g0      + x[2]*g1      + x[3]*g1_conj + x[4]*g0_conj;
    y[2] = x[0] + x[1]*g1      + x[2]*g0_conj + x[3]*g0      + x[4]*g1_conj;
    y[3] = x[0] + x[1]*g1_conj + x[2]*g0      + x[3]*g0_conj + x[4]*g1;
    y[4] = x[0] + x[1]*g0_conj + x[2]*g1_conj + x[3]*g1      + x[4]*g0;
    return LIQUID_OK;
}

//
int FFT(_execute_dft_6)(FFT(plan) _q)
{
    TC * x = _q->x;
    TC * y = _q->y;

    // DC value is sum of inputs
    y[0] = x[0] + x[1] + x[2] + x[3] + x[4] + x[5];

    // exp(-j*2*pi*1/6) = 1/2 - j*sqrt(3)/2
    TC g = 0.5 - 0.866025403784439*_Complex_I;

    TC g1, g2, g4, g5;

    if (_q->direction == LIQUID_FFT_FORWARD) {
        g1 =        g;  // exp(-j*2*pi*1/6)
        g2 = -conjf(g); // exp(-j*2*pi*2/6)
        g4 =       -g;  // exp(-j*2*pi*4/6)
        g5 =  conjf(g); // exp(-j*2*pi*5/6)
    } else {
        g1 =  conjf(g); // exp( j*2*pi*1/6)
        g2 =       -g;  // exp( j*2*pi*2/6)
        g4 = -conjf(g); // exp( j*2*pi*4/6)
        g5 =        g;  // exp( j*2*pi*5/6)
    }

    y[1] = x[0] + x[1]*g1 + x[2]*g2 - x[3] + x[4]*g4 + x[5]*g5;
    y[2] = x[0] + x[1]*g2 + x[2]*g4 + x[3] + x[4]*g2 + x[5]*g4;
    y[3] = x[0] - x[1]    + x[2]    - x[3] + x[4]    - x[5];
    y[4] = x[0] + x[1]*g4 + x[2]*g2 + x[3] + x[4]*g4 + x[5]*g2;
    y[5] = x[0] + x[1]*g5 + x[2]*g4 - x[3] + x[4]*g2 + x[5]*g1;
    return LIQUID_OK;
}

//
int FFT(_execute_dft_7)(FFT(plan) _q)
{
    TC * x = _q->x;
    TC * y = _q->y;

    // DC value is sum of inputs
    y[0] = x[0] + x[1] + x[2] + x[3] + x[4] + x[5] + x[6];

    // initialize twiddle factors
    TC g1 =  0.623489801858734 - 0.781831482468030 * _Complex_I; // exp(-j*2*pi*1/7)
    TC g2 = -0.222520933956314 - 0.974927912181824 * _Complex_I; // exp(-j*2*pi*2/7)
    TC g3 = -0.900968867902419 - 0.433883739117558 * _Complex_I; // exp(-j*2*pi*3/7)

    if (_q->direction == LIQUID_FFT_FORWARD) {
    } else {
        g1 = conjf(g1); // exp(+j*2*pi*1/7)
        g2 = conjf(g2); // exp(+j*2*pi*2/7)
        g3 = conjf(g3); // exp(+j*2*pi*3/7)
    }

    TC g4 = conjf(g3);
    TC g5 = conjf(g2);
    TC g6 = conjf(g1);

    y[1] = x[0] + x[1]*g1 + x[2]*g2 + x[3]*g3 + x[4]*g4 + x[5]*g5 + x[6]*g6;
    y[2] = x[0] + x[1]*g2 + x[2]*g4 + x[3]*g6 + x[4]*g1 + x[5]*g3 + x[6]*g5;
    y[3] = x[0] + x[1]*g3 + x[2]*g6 + x[3]*g2 + x[4]*g5 + x[5]*g1 + x[6]*g4;
    y[4] = x[0] + x[1]*g4 + x[2]*g1 + x[3]*g5 + x[4]*g2 + x[5]*g6 + x[6]*g3;
    y[5] = x[0] + x[1]*g5 + x[2]*g3 + x[3]*g1 + x[4]*g6 + x[5]*g4 + x[6]*g2;
    y[6] = x[0] + x[1]*g6 + x[2]*g5 + x[3]*g4 + x[4]*g3 + x[5]*g2 + x[6]*g1;
    return LIQUID_OK;
}

//
int FFT(_execute_dft_8)(FFT(plan) _q)
{
    TC yp;
    TC * x = _q->x;
    TC * y = _q->y;

    // fft or ifft?
    int fft = _q->direction == LIQUID_FFT_FORWARD ? 1 : 0;

    // index reversal
    y[0] = x[0];
    y[1] = x[4];
    y[2] = x[2];
    y[3] = x[6];
    y[4] = x[1];
    y[5] = x[5];
    y[6] = x[3];
    y[7] = x[7];

    // i=0
    yp = y[1];  y[1] = y[0]-yp;     y[0] += yp;
    yp = y[3];  y[3] = y[2]-yp;     y[2] += yp;
    yp = y[5];  y[5] = y[4]-yp;     y[4] += yp;
    yp = y[7];  y[7] = y[6]-yp;     y[6] += yp;


    // i=1
    yp = y[2];  y[2] = y[0]-yp;     y[0] += yp;
    yp = y[6];  y[6] = y[4]-yp;     y[4] += yp;

    if (fft) yp =  cimagf(y[3]) - crealf(y[3])*_Complex_I;
    else     yp = -cimagf(y[3]) + crealf(y[3])*_Complex_I;
    y[3] = y[1]-yp;
    y[1] += yp;

    if (fft) yp =  cimagf(y[7]) - crealf(y[7])*_Complex_I;
    else     yp = -cimagf(y[7]) + crealf(y[7])*_Complex_I;
    y[7] = y[5]-yp;
    y[5] += yp;


    // i=2
    yp = y[4];  y[4] = y[0]-yp;     y[0] += yp;

    if (fft) yp = y[5]*(M_SQRT1_2 - M_SQRT1_2*_Complex_I);
    else     yp = y[5]*(M_SQRT1_2 + M_SQRT1_2*_Complex_I);
    y[5] = y[1]-yp;
    y[1] += yp;

    if (fft) yp =  cimagf(y[6]) - crealf(y[6])*_Complex_I;
    else     yp = -cimagf(y[6]) + crealf(y[6])*_Complex_I;
    y[6] = y[2]-yp;
    y[2] += yp;

    if (fft) yp = y[7]*(-M_SQRT1_2 - M_SQRT1_2*_Complex_I);
    else     yp = y[7]*(-M_SQRT1_2 + M_SQRT1_2*_Complex_I);
    y[7] = y[3]-yp;
    y[3] += yp;
    return LIQUID_OK;
}

//
int FFT(_execute_dft_16)(FFT(plan) _q)
{
    TC yp;
    TC * x = _q->x;
    TC * y = _q->y;

    // fft or ifft?
    int fft = _q->direction == LIQUID_FFT_FORWARD ? 1 : 0;

    // index reversal
    y[ 0] = x[ 0];
    y[ 1] = x[ 8];
    y[ 2] = x[ 4];
    y[ 3] = x[12];
    y[ 4] = x[ 2];
    y[ 5] = x[10];
    y[ 6] = x[ 6];
    y[ 7] = x[14];
    y[ 8] = x[ 1];
    y[ 9] = x[ 9];
    y[10] = x[ 5];
    y[11] = x[13];
    y[12] = x[ 3];
    y[13] = x[11];
    y[14] = x[ 7];
    y[15] = x[15];

    // i=0
    yp =  y[ 1];    y[ 1]  = y[ 0] - yp;    y[ 0] += yp;
    yp =  y[ 3];    y[ 3]  = y[ 2] - yp;    y[ 2] += yp;
    yp =  y[ 5];    y[ 5]  = y[ 4] - yp;    y[ 4] += yp;
    yp =  y[ 7];    y[ 7]  = y[ 6] - yp;    y[ 6] += yp;
    yp =  y[ 9];    y[ 9]  = y[ 8] - yp;    y[ 8] += yp;
    yp =  y[11];    y[11]  = y[10] - yp;    y[10] += yp;
    yp =  y[13];    y[13]  = y[12] - yp;    y[12] += yp;
    yp =  y[15];    y[15]  = y[14] - yp;    y[14] += yp;

    // i=1
    yp =  y[ 2];    y[ 2]  = y[ 0] - yp;    y[ 0] += yp;
    yp =  y[ 6];    y[ 6]  = y[ 4] - yp;    y[ 4] += yp;
    yp =  y[10];    y[10]  = y[ 8] - yp;    y[ 8] += yp;
    yp =  y[14];    y[14]  = y[12] - yp;    y[12] += yp;
    if (fft) {
        yp = -y[ 3]*_Complex_I;    y[ 3]  = y[ 1] - yp;    y[ 1] += yp;
        yp = -y[ 7]*_Complex_I;    y[ 7]  = y[ 5] - yp;    y[ 5] += yp;
        yp = -y[11]*_Complex_I;    y[11]  = y[ 9] - yp;    y[ 9] += yp;
        yp = -y[15]*_Complex_I;    y[15]  = y[13] - yp;    y[13] += yp;
    } else {
        yp  =  y[ 3]*_Complex_I;    y[ 3]  = y[ 1] - yp;   y[ 1] += yp;
        yp  =  y[ 7]*_Complex_I;    y[ 7]  = y[ 5] - yp;   y[ 5] += yp;
        yp  =  y[11]*_Complex_I;    y[11]  = y[ 9] - yp;   y[ 9] += yp;
        yp  =  y[15]*_Complex_I;    y[15]  = y[13] - yp;   y[13] += yp;
    }

    // i=2
    yp =  y[ 4];    y[ 4]  = y[ 0] - yp;    y[ 0] += yp;
    yp =  y[12];    y[12]  = y[ 8] - yp;    y[ 8] += yp;
    if (fft) {
        yp =  y[ 5]*(  0.70710677 + _Complex_I* -0.70710677);  y[ 5]  = y[ 1] - yp;  y[ 1] += yp;
        yp =  y[13]*(  0.70710677 + _Complex_I* -0.70710677);  y[13]  = y[ 9] - yp;  y[ 9] += yp;
        yp = -y[ 6]*_Complex_I;                                y[ 6]  = y[ 2] - yp;  y[ 2] += yp;
        yp = -y[14]*_Complex_I;                                y[14]  = y[10] - yp;  y[10] += yp;
        yp =  y[ 7]*( -0.70710677 + _Complex_I* -0.70710677);  y[ 7]  = y[ 3] - yp;  y[ 3] += yp;
        yp =  y[15]*( -0.70710677 + _Complex_I* -0.70710677);  y[15]  = y[11] - yp;  y[11] += yp;
    } else {
        yp =  y[ 5]*(  0.70710677 - _Complex_I* -0.70710677);  y[ 5]  = y[ 1] - yp;  y[ 1] += yp;
        yp =  y[13]*(  0.70710677 - _Complex_I* -0.70710677);  y[13]  = y[ 9] - yp;  y[ 9] += yp;
        yp  =  y[ 6]*_Complex_I;                               y[ 6]  = y[ 2] - yp;  y[ 2] += yp;
        yp  =  y[14]*_Complex_I;                               y[14]  = y[10] - yp;  y[10] += yp;
        yp =  y[ 7]*( -0.70710677 - _Complex_I* -0.70710677);  y[ 7]  = y[ 3] - yp;  y[ 3] += yp;
        yp =  y[15]*( -0.70710677 - _Complex_I* -0.70710677);  y[15]  = y[11] - yp;  y[11] += yp;
    }

    // i=3
    yp =  y[ 8];    y[ 8]  = y[ 0] - yp;    y[ 0] += yp;
    if (fft) {
        yp =  y[ 9]*(  0.92387950 + _Complex_I* -0.38268346);  y[ 9]  = y[ 1] - yp;  y[ 1] += yp;
        yp =  y[10]*(  0.70710677 + _Complex_I* -0.70710677);  y[10]  = y[ 2] - yp;  y[ 2] += yp;
        yp =  y[11]*(  0.38268343 + _Complex_I* -0.92387950);  y[11]  = y[ 3] - yp;  y[ 3] += yp;
        yp = -y[12]*_Complex_I;                                y[12]  = y[ 4] - yp;  y[ 4] += yp;
        yp =  y[13]*( -0.38268340 + _Complex_I* -0.92387956);  y[13]  = y[ 5] - yp;  y[ 5] += yp;
        yp =  y[14]*( -0.70710677 + _Complex_I* -0.70710677);  y[14]  = y[ 6] - yp;  y[ 6] += yp;
        yp =  y[15]*( -0.92387950 + _Complex_I* -0.38268349);  y[15]  = y[ 7] - yp;  y[ 7] += yp;
    } else {
        yp =  y[ 9]*(  0.92387950 - _Complex_I* -0.38268346);  y[ 9]  = y[ 1] - yp;  y[ 1] += yp;
        yp =  y[10]*(  0.70710677 - _Complex_I* -0.70710677);  y[10]  = y[ 2] - yp;  y[ 2] += yp;
        yp =  y[11]*(  0.38268343 - _Complex_I* -0.92387950);  y[11]  = y[ 3] - yp;  y[ 3] += yp;
        yp =  y[12]*_Complex_I;                                y[12]  = y[ 4] - yp;  y[ 4] += yp;
        yp =  y[13]*( -0.38268340 - _Complex_I* -0.92387956);  y[13]  = y[ 5] - yp;  y[ 5] += yp;
        yp =  y[14]*( -0.70710677 - _Complex_I* -0.70710677);  y[14]  = y[ 6] - yp;  y[ 6] += yp;
        yp =  y[15]*( -0.92387950 - _Complex_I* -0.38268349);  y[15]  = y[ 7] - yp;  y[ 7] += yp;
    }
    return LIQUID_OK;
}

