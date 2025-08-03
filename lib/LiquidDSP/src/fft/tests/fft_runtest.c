/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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

#include "autotest/autotest.h"
#include "liquid.h"

// autotest helper function
//  _x      :   fft input array
//  _test   :   expected fft output
//  _n      :   fft size
void fft_test(float complex * _x,
              float complex * _test,
              unsigned int    _n)
{
    int _method = 0;
    float tol=2e-4f;

    unsigned int i;

    float complex y[_n], z[_n];

    // compute FFT
    fftplan pf = fft_create_plan(_n, _x, y, LIQUID_FFT_FORWARD, _method);
    fft_execute(pf);

    // compute IFFT
    fftplan pr = fft_create_plan(_n, y, z, LIQUID_FFT_BACKWARD, _method);
    fft_execute(pr);

    // normalize inverse
    for (i=0; i<_n; i++)
        z[i] /= (float) _n;

    // validate results
    float fft_error, ifft_error;
    for (i=0; i<_n; i++) {
        fft_error = cabsf( y[i] - _test[i] );
        ifft_error = cabsf( _x[i] - z[i] );
        CONTEND_DELTA( fft_error, 0, tol);
        CONTEND_DELTA( ifft_error, 0, tol);
    }

    // destroy plans
    fft_destroy_plan(pf);
    fft_destroy_plan(pr);
}

