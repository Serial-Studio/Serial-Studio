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

#include "autotest/autotest.h"
#include "liquid.h"

// test rational-rate resampler
void test_iirinterp_crcf(const char * _method,
                         unsigned int _interp,
                         unsigned int _order)
{
    // options
    unsigned int n    = 800000; // number of output samples to analyze
    float        bw   = 0.2f;   // target output bandwidth
    unsigned int nfft = 800;    // number of bins in transform
    float        As   = 60.0f;  // error tolerance [dB]
    float        tol  = 0.5f;   // error tolerance [dB]

    // create resampler with rate _interp/_decim
    iirinterp_crcf interp = iirinterp_crcf_create_default(_interp, _order);

    // create and configure objects
    spgramcf     q   = spgramcf_create(nfft, LIQUID_WINDOW_HANN, nfft/2, nfft/4);
    symstreamrcf gen = symstreamrcf_create_linear(LIQUID_FIRFILT_KAISER,bw*_interp,25,0.2f,LIQUID_MODEM_QPSK);
    symstreamrcf_set_gain(gen, sqrtf(bw));

    // generate samples and push through spgram object
    unsigned int  block_size = 10;
    float complex buf_0[block_size];         // input buffer
    float complex buf_1[block_size*_interp]; // output buffer
    while (spgramcf_get_num_samples_total(q) < n) {
        // generate block of samples
        symstreamrcf_write_samples(gen, buf_0, block_size);

        // interpolate
        iirinterp_crcf_execute_block(interp, buf_0, block_size, buf_1);

        // run samples through the spgram object
        spgramcf_write(q, buf_1, block_size*_interp);
    }

    // verify result
    float psd[nfft];
    spgramcf_get_psd(q, psd);
    autotest_psd_s regions[] = {
        {.fmin=-0.5f,    .fmax=-0.6f*bw, .pmin=0,     .pmax=-As+tol, .test_lo=0, .test_hi=1},
        {.fmin=-0.4f*bw, .fmax=+0.4f*bw, .pmin=0-tol, .pmax=  0+tol, .test_lo=1, .test_hi=1},
        {.fmin=+0.6f*bw, .fmax=+0.5f,    .pmin=0,     .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/iirinterp_crcf_M%u_O%u.m", _interp, _order);
    liquid_autotest_validate_spectrum(psd, nfft, regions, 3,
        liquid_autotest_verbose ? filename : NULL);

    // destroy objects
    iirinterp_crcf_destroy(interp);
    spgramcf_destroy(q);
    symstreamrcf_destroy(gen);
}

// baseline tests using create_kaiser() method
void autotest_iirinterp_crcf_M2_O9() { test_iirinterp_crcf("baseline", 2, 9); }
void autotest_iirinterp_crcf_M3_O9() { test_iirinterp_crcf("baseline", 3, 9); }
void autotest_iirinterp_crcf_M4_O9() { test_iirinterp_crcf("baseline", 4, 9); }

// test copy method
void autotest_iirinterp_copy()
{
    // create base object
    iirinterp_crcf q0 = iirinterp_crcf_create_default(3, 7);
    //iirinterp_crcf_set_scale(q0, 0.12345f);

    // run samples through filter
    unsigned int i;
    float complex buf_0[3], buf_1[3];
    for (i=0; i<20; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        iirinterp_crcf_execute(q0, v, buf_0);
    }

    // copy object
    iirinterp_crcf q1 = iirinterp_crcf_copy(q0);

    // run samples through both filters in parallel
    for (i=0; i<60; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        iirinterp_crcf_execute(q0, v, buf_0);
        iirinterp_crcf_execute(q1, v, buf_1);

        CONTEND_SAME_DATA( buf_0, buf_1, 3*sizeof(float complex) );
    }

    // destroy objects
    iirinterp_crcf_destroy(q0);
    iirinterp_crcf_destroy(q1);
}

