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

#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

// check direct digital synthesis, both interpolator and decimator
void testbench_dds_cccf(unsigned int _num_stages,   // number of half-band stages
                        float        _fc,           // filter cut-off
                        float        _as)           // stop-band suppression
{
    float        tol  = 1;  // error tolerance [dB]
    float        bw = 0.1f; // original pulse bandwidth
    unsigned int m    = 40; // pulse semi-length
    unsigned int r=1<<_num_stages;   // resampling rate (output/input)

    // create resampler
    dds_cccf q = dds_cccf_create(_num_stages,_fc,bw,_as);
    dds_cccf_set_scale(q, 1.0f/r);
    if (liquid_autotest_verbose)
        dds_cccf_print(q);

    unsigned int delay_interp = dds_cccf_get_delay_interp(q);
    float        delay_decim  = dds_cccf_get_delay_decim (q);
    unsigned int h_len = 2*r*m+1; // pulse length
    unsigned int num_samples = h_len + delay_interp + (unsigned int) delay_decim + 8;

    unsigned int i;
    float complex * buf_0 = (float complex*) malloc(num_samples  *sizeof(float complex)); // input
    float complex * buf_1 = (float complex*) malloc(num_samples*r*sizeof(float complex)); // interpolated
    float complex * buf_2 = (float complex*) malloc(num_samples  *sizeof(float complex)); // decimated

    // generate the baseband signal (filter pulse)
    float h[h_len];
    float w = 0.36f * bw; // pulse bandwidth
    liquid_firdes_kaiser(h_len,w,_as,0.0f,h);
    for (i=0; i<num_samples; i++)
        buf_0[i] = i < h_len ? 2*w*h[i] : 0.0f;

    // run interpolation (up-conversion) stage
    for (i=0; i<num_samples; i++)
        dds_cccf_interp_execute(q, buf_0[i], &buf_1[r*i]);

    // clear DDS object
    dds_cccf_reset(q);
    dds_cccf_set_scale(q, r);

    // run decimation (down-conversion) stage
    for (i=0; i<num_samples; i++)
        dds_cccf_decim_execute(q, &buf_1[r*i], &buf_2[i]);

    // verify input spectrum
    autotest_psd_s regions_orig[] = {
      {.fmin=-0.5,    .fmax=-0.6*bw, .pmin= 0, .pmax=-_as+tol, .test_lo=0, .test_hi=1},
      {.fmin=-0.3*bw, .fmax=+0.3*bw, .pmin=-1, .pmax=+1,       .test_lo=1, .test_hi=1},
      {.fmin=+0.6*bw, .fmax=+0.5,    .pmin= 0, .pmax=-_as+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signal(buf_0, num_samples, regions_orig, 3,
        liquid_autotest_verbose ? "autotest/logs/dds_cccf_orig.m" : NULL);

    // verify interpolated spectrum
    float f1 = _fc-0.6*bw/r, f2 = _fc-0.3*bw/r, f3 = _fc+0.3*bw/r, f4 = _fc+0.6*bw/r;
    autotest_psd_s regions_interp[] = {
      {.fmin=-0.5, .fmax=f1,   .pmin= 0, .pmax=-_as+tol, .test_lo=0, .test_hi=1},
      {.fmin= f2,  .fmax=f3,   .pmin=-1, .pmax=+1,       .test_lo=1, .test_hi=1},
      {.fmin= f4,  .fmax=+0.5, .pmin= 0, .pmax=-_as+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signal(buf_1, r*num_samples, regions_interp, 3,
        liquid_autotest_verbose ? "autotest/logs/dds_cccf_interp.m" : NULL);

    // verify decimated spectrum (using same regions as original)
    liquid_autotest_validate_psd_signal(buf_2, num_samples, regions_orig, 3,
        liquid_autotest_verbose ? "autotest/logs/dds_cccf_decim.m" : NULL);

    // destroy filter object and free memory
    dds_cccf_destroy(q);
    free(buf_0);
    free(buf_1);
    free(buf_2);
}

// test different configurations
void autotest_dds_cccf_0(){ testbench_dds_cccf( 1, +0.0f, 60.0f); }
void autotest_dds_cccf_1(){ testbench_dds_cccf( 2, +0.0f, 60.0f); }
void autotest_dds_cccf_2(){ testbench_dds_cccf( 3, +0.0f, 60.0f); }

// FIXME: adjust filter lengths appropriately
//void xautotest_dds_cccf_4(){ testbench_dds_cccf( 2, +0.1f,      60.0f); }
//void xautotest_dds_cccf_5(){ testbench_dds_cccf( 2, -0.213823f, 60.0f); }
//void xautotest_dds_cccf_6(){ testbench_dds_cccf( 2, -0.318234f, 80.0f); }

void autotest_dds_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping dds config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check that object returns NULL for invalid configurations
    CONTEND_ISNULL(dds_cccf_create( 50,  0.0f,  0.1f, 60.0f)); // num stages out of range
    CONTEND_ISNULL(dds_cccf_create(  2,  0.7f,  0.1f, 60.0f)); // fc out of range
    CONTEND_ISNULL(dds_cccf_create(  2, -0.7f,  0.1f, 60.0f)); // fc out of range
    CONTEND_ISNULL(dds_cccf_create(  2,  0.2f,  1.4f, 60.0f)); // bw out of range
    CONTEND_ISNULL(dds_cccf_create(  2,  0.2f, -1.4f, 60.0f)); // bw out of range
    CONTEND_ISNULL(dds_cccf_create(  2,  0.2f,  0.1f, -1.0f)); // as out of range

    // create proper object and test configurations
    dds_cccf q = dds_cccf_create( 2, 0.0f, 0.2f, 60.0f);
    dds_cccf_print(q);

    // test setting/getting properties
    dds_cccf_set_scale(q, 2.0f - _Complex_I*3.0f);
    float complex scale = 0.0f;
    dds_cccf_get_scale(q, &scale);
    CONTEND_EQUALITY(scale, 2.0f - _Complex_I*3.0f);

    // destroy object
    dds_cccf_destroy(q);
}

// copy object
void autotest_dds_copy()
{
    unsigned int num_stages = 3;    // number of half-band stages
    unsigned int r=1<<num_stages;   // resampling rate (input/output)

    // create resampler
    dds_cccf q0 = dds_cccf_create(num_stages,0.1234f,0.4321f,60.0f);
    dds_cccf_set_scale(q0, 0.72280f);

    // create generator with default parameters
    symstreamrcf gen = symstreamrcf_create();

    // generate samples and push through resampler
    float complex buf[r]; // input buffer
    float complex y0, y1; // output samples
    unsigned int i;
    for (i=0; i<10; i++) {
        // generate block of samples
        symstreamrcf_write_samples(gen, buf, r);

        // resample
        dds_cccf_decim_execute(q0, buf, &y0);
    }

    // copy object
    dds_cccf q1 = dds_cccf_copy(q0);

    // run samples through both resamplers in parallel
    for (i=0; i<60; i++) {
        // generate block of samples
        symstreamrcf_write_samples(gen, buf, r);

        // resample
        dds_cccf_decim_execute(q0, buf, &y0);
        dds_cccf_decim_execute(q1, buf, &y1);

        // compare output
        CONTEND_EQUALITY(y0, y1);
    }

    // destroy objects
    dds_cccf_destroy(q0);
    dds_cccf_destroy(q1);
    symstreamrcf_destroy(gen);
}

