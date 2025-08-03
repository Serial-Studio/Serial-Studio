/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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

// symbol timing synchronizer tests

#include <string.h>
#include "autotest/autotest.h"
#include "liquid.h"

//
void symsync_crcf_test(const char * _method,
                       unsigned int _k,
                       unsigned int _m,
                       float        _beta,
                       float        _tau,
                       float        _rate)
{
    // options
    float        tol        =  0.2f;    // error tolerance
    unsigned int k          =  _k;      // samples/symbol (input)
    unsigned int m          =  _m;      // filter delay (symbols)
    float        beta       =  _beta;   // filter excess bandwidth factor
    unsigned int num_filters= 32;       // number of filters in the bank

    unsigned int num_symbols_init=200;  // number of initial symbols
    unsigned int num_symbols_test=100;  // number of testing symbols

    // transmit filter type
    liquid_firfilt_type ftype_tx = strcmp(_method,"rnyquist")==0 ?
        LIQUID_FIRFILT_ARKAISER : LIQUID_FIRFILT_KAISER;

    float bt    =  0.02f;               // loop filter bandwidth
    float tau   =  _tau;                // fractional symbol offset
    float rate  =  _rate;               // resampled rate

    // derived values
    unsigned int num_symbols = num_symbols_init + num_symbols_test;
    unsigned int num_samples = k*num_symbols;
    unsigned int num_samples_resamp = (unsigned int) ceilf(num_samples*rate*1.1f) + 4;
    
    // compute delay
    while (tau < 0) tau += 1.0f;    // ensure positive tau
    float g = k*tau;                // number of samples offset
    int ds=floorf(g);               // additional symbol delay
    float dt = (g - (float)ds);     // fractional sample offset
    if (dt > 0.5f) {                // force dt to be in [0.5,0.5]
        dt -= 1.0f;
        ds++;
    }

    unsigned int i;

    // allocate arrays
    float complex s[num_symbols];       // data symbols
    float complex x[num_samples];       // interpolated samples
    float complex y[num_samples_resamp];// resampled data (resamp_crcf)
    float complex z[num_symbols + 64];  // synchronized symbols

    // generate pseudo-random QPSK symbols
    // NOTE: by using an m-sequence generator this sequence will be identical
    //       each time this test is run
    msequence ms = msequence_create_default(10);
    for (i=0; i<num_symbols; i++) {
        int si = msequence_generate_symbol(ms, 1);
        int sq = msequence_generate_symbol(ms, 1);
        s[i] = (si ? -1.0f : 1.0f) * M_SQRT1_2 +
               (sq ? -1.0f : 1.0f) * M_SQRT1_2 * _Complex_I;
    }
    msequence_destroy(ms);

    // 
    // create and run interpolator
    //

    // design interpolating filter
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype_tx,k,m,beta,dt);

    // interpolate block of samples
    firinterp_crcf_execute_block(interp, s, num_symbols, x);

    // destroy interpolator
    firinterp_crcf_destroy(interp);

    // 
    // run resampler
    //

    // create resampler
    unsigned int resamp_len = 10*k; // resampling filter semi-length (filter delay)
    float resamp_bw = 0.45f;        // resampling filter bandwidth
    float resamp_As = 60.0f;        // resampling filter stop-band attenuation
    unsigned int resamp_npfb = 64;  // number of filters in bank
    resamp_crcf resamp = resamp_crcf_create(rate, resamp_len, resamp_bw, resamp_As, resamp_npfb);

    // run resampler on block
    unsigned int ny;
    resamp_crcf_execute_block(resamp, x, num_samples, y, &ny);

    // destroy resampler
    resamp_crcf_destroy(resamp);

    // 
    // create and run symbol synchronizer
    //

    // create symbol synchronizer
    symsync_crcf sync = NULL;
    if (strcmp(_method,"rnyquist")==0)
        sync = symsync_crcf_create_rnyquist(ftype_tx, k, m, beta, num_filters);
    else
        sync = symsync_crcf_create_kaiser(k, m, beta, num_filters);

    // set loop filter bandwidth
    symsync_crcf_set_lf_bw(sync,bt);

    // execute on entire block of samples
    unsigned int nz;
    symsync_crcf_execute(sync, y, ny, z, &nz);

    // destroy synchronizer
    symsync_crcf_destroy(sync);

    // compute total delay through system
    // (initial filter + resampler + matched filter)
    unsigned int delay = m + 10 + m;

    if (liquid_autotest_verbose) {
        printf("symsync_crcf_test(),\n");
        printf("    k       :   %u\n",      k);
        printf("    m       :   %u\n",      m);
        printf("    beta    :   %-8.4f\n",   beta);
        printf("    tau     :   %-8.4f\n",   tau);
        printf("    rate    :   %-12.8f\n",  rate);
        printf("output symbols:\n");
    }

    // compare (and print) results
    for (i=nz-num_symbols_test; i<nz; i++) {
        // compute error
        float err = cabsf( z[i] - s[i-delay] );
        
        // assert that error is below tolerance
        CONTEND_LESS_THAN( err, tol );

        // print formatted results if desired
        if (liquid_autotest_verbose) {
            printf("  sym_out(%4u) = %8.4f + j*%8.4f; %% {%8.4f + j*%8.4f} e = %12.8f %s\n",
                    i+1,
                    crealf(z[i]      ), cimagf(z[i]      ),
                    crealf(s[i-delay]), cimagf(s[i-delay]),
                    err, err < tol ? "" : "*");
        }
    }

}

// autotest scenarios (root-Nyquist)
void autotest_symsync_crcf_scenario_0() { symsync_crcf_test("rnyquist", 2, 7, 0.35,  0.00, 1.0f    ); }
void autotest_symsync_crcf_scenario_1() { symsync_crcf_test("rnyquist", 2, 7, 0.35, -0.25, 1.0f    ); }
void autotest_symsync_crcf_scenario_2() { symsync_crcf_test("rnyquist", 2, 7, 0.35, -0.25, 1.0001f ); }
void autotest_symsync_crcf_scenario_3() { symsync_crcf_test("rnyquist", 2, 7, 0.35, -0.25, 0.9999f ); }

// autotest scenarios (Nyquist)
void autotest_symsync_crcf_scenario_4() { symsync_crcf_test("nyquist", 2, 7, 0.35,  0.00, 1.0f    ); }
void autotest_symsync_crcf_scenario_5() { symsync_crcf_test("nyquist", 2, 7, 0.35, -0.25, 1.0f    ); }
void autotest_symsync_crcf_scenario_6() { symsync_crcf_test("nyquist", 2, 7, 0.35, -0.25, 1.0001f ); }
void autotest_symsync_crcf_scenario_7() { symsync_crcf_test("nyquist", 2, 7, 0.35, -0.25, 0.9999f ); }

