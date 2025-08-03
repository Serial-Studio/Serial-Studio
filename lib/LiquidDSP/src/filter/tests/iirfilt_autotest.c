/*
 * Copyright (c) 2007 - 2025 Joseph Gaeddert
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
#include "liquid.internal.h"

void autotest_iirfilt_integrator()
{
    // options
    unsigned int num_ones    = 10;
    unsigned int num_samples = 40;

    // allocate memory for data arrays
    float buf_0[num_samples]; // filter input
    float buf_1[num_samples]; // filter output

    // generate input signal
    unsigned int i;
    for (i=0; i<num_samples; i++)
        buf_0[i] = i < num_ones ? 1 : 0;

    // create integrator and run on sample data
    iirfilt_rrrf q = iirfilt_rrrf_create_integrator();
    iirfilt_rrrf_execute_block(q, buf_0, num_samples, buf_1);
    iirfilt_rrrf_destroy(q);

    //for (i=0; i<num_samples; i++)
    //    printf("[%3u] %20.17f\n", i, buf_1[i]);

    // check that last value matches expected
    CONTEND_DELTA(buf_1[num_samples-1], num_ones, 0.01f);
}

void autotest_iirfilt_differentiator()
{
    // options
    unsigned int num_samples = 400;

    // allocate memory for data arrays
    float buf_0[num_samples]; // filter input
    float buf_1[num_samples]; // filter output

    // generate input signal
    unsigned int i;
    for (i=0; i<num_samples; i++)
        buf_0[i] = i;

    // create differentiator and run on sample data
    iirfilt_rrrf q = iirfilt_rrrf_create_differentiator();
    iirfilt_rrrf_execute_block(q, buf_0, num_samples, buf_1);
    iirfilt_rrrf_destroy(q);

    //for (i=0; i<num_samples; i++)
    //    printf("[%3u] %20.17f\n", i, buf_1[i]);

    // check that derivative is equal to 1
    CONTEND_DELTA(buf_1[num_samples-1], 1.0f, 0.01f);
}

void autotest_iirfilt_dcblock()
{
    // options
    unsigned int n    = 400000; // number of output samples to analyze
    float        alpha= 0.2f;   // forgetting factor
    unsigned int nfft = 1200;   // number of bins in transform
    float        tol  = 1.0f;   // error tolerance [dB]

    // create base object
    iirfilt_crcf filter = iirfilt_crcf_create_dc_blocker(alpha);

    // create and configure objects
    spgramcf q = spgramcf_create(nfft, LIQUID_WINDOW_HANN, nfft/2, nfft/4);

    // start running input through filter
    float complex x, y;
    unsigned int i;
    for (i=0; i<n; i++) {
        // generate noise input
        x = (randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // apply filter
        iirfilt_crcf_execute(filter, x, &y);

        // run samples through the spgram object
        spgramcf_push(q, y);
    }

    // verify result
    float psd[nfft];
    spgramcf_get_psd(q, psd);
    autotest_psd_s regions[] = {
        {.fmin=-0.500f, .fmax=-0.200f, .pmin=-tol, .pmax=+tol, .test_lo=1, .test_hi=1},
        {.fmin=-0.002f, .fmax=+0.002f, .pmin=-tol, .pmax=-20,  .test_lo=0, .test_hi=1},
        {.fmin=+0.200f, .fmax=+0.500f, .pmin=-tol, .pmax=+tol, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_spectrum(psd, nfft, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/iirfilt_dcblock.m" : NULL);

    // destroy objects
    spgramcf_destroy(q);
    iirfilt_crcf_destroy(filter);
}

void testbench_iirfilt_copy(liquid_iirdes_format _format)
{
    // create base object
    iirfilt_crcf q0 = iirfilt_crcf_create_prototype(
        LIQUID_IIRDES_ELLIP, LIQUID_IIRDES_LOWPASS, _format,
        9,0.2f,0.0f,0.1f,60.0f);

    // start running input through filter
    unsigned int i, num_samples = 80;
    float complex y0, y1;
    for (i=0; i<num_samples; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        iirfilt_crcf_execute(q0, v, &y0);
    }

    // copy filter
    iirfilt_crcf q1 = iirfilt_crcf_copy(q0);

    // continue running through both filters
    for (i=0; i<num_samples; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        iirfilt_crcf_execute(q0, v, &y0);
        iirfilt_crcf_execute(q1, v, &y1);

        // compare result
        CONTEND_EQUALITY(y0, y1);
    }

    // destroy filter objects
    iirfilt_crcf_destroy(q0);
    iirfilt_crcf_destroy(q1);
}

void autotest_iirfilt_copy_tf () { testbench_iirfilt_copy(LIQUID_IIRDES_TF ); }
void autotest_iirfilt_copy_sos() { testbench_iirfilt_copy(LIQUID_IIRDES_SOS); }

// test errors and invalid configuration
void autotest_iirfilt_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping iirfilt config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // test copying/creating invalid objects
    CONTEND_ISNULL( iirfilt_crcf_copy(NULL) );
    CONTEND_ISNULL( iirfilt_crcf_create(NULL, 0, NULL, 5) ); // nb is 0
    CONTEND_ISNULL( iirfilt_crcf_create(NULL, 5, NULL, 0) ); // nb is 0
    CONTEND_ISNULL( iirfilt_crcf_create_sos(NULL, NULL, 0) ); // nsos is 0
    CONTEND_ISNULL( iirfilt_crcf_create_prototype(LIQUID_IIRDES_BUTTER,LIQUID_IIRDES_LOWPASS,LIQUID_IIRDES_SOS,0,0,0,0,0) );

    // create valid object
    iirfilt_crcf filter = iirfilt_crcf_create_lowpass(7, 0.1f);
    CONTEND_EQUALITY( LIQUID_OK, iirfilt_crcf_print(filter) );

    // check properties
    CONTEND_EQUALITY( LIQUID_OK, iirfilt_crcf_set_scale(filter, 7.22f) );
    float scale;
    CONTEND_EQUALITY( LIQUID_OK, iirfilt_crcf_get_scale(filter, &scale) );
    CONTEND_EQUALITY( scale, 7.22f );
    CONTEND_EQUALITY( 7+1, iirfilt_crcf_get_length(filter) );

    // destroy object
    iirfilt_crcf_destroy(filter);
}

