/*
 * Copyright (c) 2007 - 2019 Joseph Gaeddert
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

#include <math.h>
#include "autotest/autotest.h"
#include "liquid.h"

// Help function to keep code base small
//  _n      : input sample size
//  _p      : order
//  _fc     : filter cut-off frequency
//  _tol    : RMS error tolerance (dB)
void lpc_test_harness(unsigned int _n,
                      unsigned int _p,
                      float        _fc,
                      float        _tol)
{
    // create low-pass filter object
    iirfilt_rrrf lowpass = iirfilt_rrrf_create_lowpass(7,_fc);

    // allocate memory for data arrays
    float y[_n];         // input signal (filtered noise)
    float a_hat[_p+1];   // lpc output
    float g_hat[_p+1];   // lpc output

    // generate input signal (filtered noise)
    unsigned int i;
    msequence ms = msequence_create_default(15);
    for (i=0; i<_n; i++) {
        // rough, but simple uniform random variable
        float v = (float)msequence_generate_symbol(ms,10)/1023.0f - 0.5f;

        // filter result
        iirfilt_rrrf_execute(lowpass, v, &y[i]);
    }

    // run linear prediction algorithm
    liquid_lpc(y,_n,_p,a_hat,g_hat);

    // create linear prediction filter
    float a_lpc[_p+1];
    float b_lpc[_p+1];
    for (i=0; i<_p+1; i++) {
        a_lpc[i] = (i==0) ? 1.0f : 0.0f;
        b_lpc[i] = (i==0) ? 0.0f : -a_hat[i];
    }
    iirfilt_rrrf lpc = iirfilt_rrrf_create(b_lpc,_p+1, a_lpc,_p+1);

    // compute prediction error over random sequence
    // NOTE: no need to reset objects here
    float rmse = 0.0f;
    unsigned int n_error = 5000;
    for (i=0; i<n_error; i++) {
        // generate input
        float v = (float)msequence_generate_symbol(ms,10)/1023.0f - 0.5f;

        // run filters
        float s0, s1;
        iirfilt_rrrf_execute(lowpass, v,  &s0); // original
        iirfilt_rrrf_execute(lpc,     s0, &s1); // predictive

        // compute error
        rmse += (s0-s1)*(s0-s1);

        //if (liquid_autotest_verbose) printf("%12.4e %12.4e %12.4e\n", v, s0, s1);
    }
    rmse = 10*log10f( rmse / (float)n_error );
    if (liquid_autotest_verbose) {
        printf("original lowpass filter:\n");
        iirfilt_rrrf_print(lowpass);
        printf("linear predictive filter:\n");
        iirfilt_rrrf_print(lpc);
        printf("lpc(n=%u,p=%u,fc=%.3f), rmse: %.2f (tol: %.2f) dB\n", _n, _p, _fc, rmse, _tol);
    }
    CONTEND_LESS_THAN(rmse, _tol);

    // destroy objects
    iirfilt_rrrf_destroy(lowpass);
    iirfilt_rrrf_destroy(lpc);
    msequence_destroy(ms);
}

// AUTOTESTS: test linear prediction algorithm with various lengths, orders, and filters
// NOTE: in most cases the RMSE is less than -50 dB, so we have plenty of margin here
void autotest_lpc_p4()  { lpc_test_harness( 200,  4, 0.020, -40.0f); }
void autotest_lpc_p6()  { lpc_test_harness( 400,  6, 0.028, -40.0f); }
void autotest_lpc_p8()  { lpc_test_harness( 600,  8, 0.035, -40.0f); }
void autotest_lpc_p10() { lpc_test_harness( 800, 10, 0.050, -40.0f); }
void autotest_lpc_p16() { lpc_test_harness(1600, 16, 0.055, -40.0f); }
void autotest_lpc_p32() { lpc_test_harness(3200, 24, 0.065, -40.0f); }

