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

#include "autotest/autotest.h"
#include "liquid.h"

// test end-to-end power specral density on interp/decim methods
void autotest_iirhilbf_interp_decim()
{
    float        tol  = 1;  // error tolerance [dB]
    float        bw = 0.4f; // pulse bandwidth
    float        As = 60.0f;// transform stop-band suppression
    unsigned int p    = 40; // pulse semi-length
    unsigned int m    =  5; // Transform order

    // create transform
    //iirhilbf q = iirhilbf_create(ftype,n,Ap,As);
    iirhilbf q = iirhilbf_create_default(m);
    iirhilbf_print(q);

    unsigned int h_len       = 2*p+1; // pulse length
    unsigned int num_samples = h_len + 2*m + 8;

    unsigned int i;
    float complex buf_0[num_samples  ];
    float         buf_1[num_samples*2];
    float complex buf_2[num_samples  ];

    // generate the baseband signal (filter pulse)
    float h[h_len];
    float w = 0.36f * bw; // pulse bandwidth
    liquid_firdes_kaiser(h_len,w,80.0f,0.0f,h);
    for (i=0; i<num_samples; i++)
        buf_0[i] = i < h_len ? 2*w*h[i] : 0.0f;

    // run interpolation
    iirhilbf_interp_execute_block(q, buf_0, num_samples, buf_1);

    // clear object
    iirhilbf_reset(q);

    // run decimation
    iirhilbf_decim_execute_block(q, buf_1, num_samples, buf_2);

    // verify input spectrum
    autotest_psd_s regions_orig[] = {
      {.fmin=-0.5,    .fmax=-0.5*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin=-0.3*bw, .fmax=+0.3*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=+0.5*bw, .fmax=+0.5,    .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signal(buf_0, num_samples, regions_orig, 3,
        liquid_autotest_verbose ? "autotest/logs/iirhilbf_orig.m" : NULL);

    // verify interpolated spectrum
    autotest_psd_s regions_interp[] = {
      {.fmin=-0.5,          .fmax=-0.25-0.25*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin=-0.25-0.15*bw, .fmax=-0.25+0.15*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=-0.25+0.25*bw, .fmax=+0.25-0.25*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin= 0.25-0.15*bw, .fmax= 0.25+0.15*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin= 0.25+0.25*bw, .fmax= 0.5,          .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signalf(buf_1, 2*num_samples, regions_interp, 5,
        liquid_autotest_verbose ? "autotest/logs/iirhilbf_interp.m" : NULL);

    // verify decimated spectrum (using same regions as original)
    liquid_autotest_validate_psd_signal(buf_2, num_samples, regions_orig, 3,
        liquid_autotest_verbose ? "autotest/logs/iirhilbf_decim.m" : NULL);

    // destroy filter object and free memory
    iirhilbf_destroy(q);
}

// test end-to-end power specral density on filter methods
void autotest_iirhilbf_filter()
{
    float        tol  = 1;  // error tolerance [dB]
    float        bw = 0.2f; // pulse bandwidth
    float        f0 = 0.3f; // pulse center frequency
    float        ft =-0.3f; // frequency of tone in lower half of band
    float        As = 60.0f;// transform stop-band suppression
    unsigned int p    = 50; // pulse semi-length
    unsigned int m    =  7; // Transform order

    // create transform
    //iirhilbf q = iirhilbf_create(ftype,n,Ap,As);
    iirhilbf q = iirhilbf_create_default(m);
    iirhilbf_print(q);

    unsigned int h_len       = 2*p+1; // pulse length
    unsigned int num_samples = h_len + 2*m + 8;

    unsigned int i;
    float complex buf_0[num_samples];
    float         buf_1[num_samples];
    float complex buf_2[num_samples];

    // generate the baseband signal (filter pulse)
    float h[h_len];
    float w = 0.36f * bw; // pulse bandwidth
    liquid_firdes_kaiser(h_len,w,80.0f,0.0f,h);
    for (i=0; i<num_samples; i++)
        buf_0[i] = i < h_len ? 2*w*h[i]*cexpf(_Complex_I*2*M_PI*f0*i) : 0.0f;
    for (i=0; i<num_samples; i++)
        buf_0[i] += i < h_len ? 1e-3f*liquid_kaiser(i,num_samples,10)*cexpf(_Complex_I*2*M_PI*ft*i) : 0.0f;

    // run interpolation
    iirhilbf_c2r_execute_block(q, buf_0, num_samples, buf_1);
    // scale output
    for (i=0; i<num_samples; i++)
        buf_1[i] *= 2;

    // clear object
    iirhilbf_reset(q);

    // run decimation
    iirhilbf_r2c_execute_block(q, buf_1, num_samples, buf_2);
    // scale output
    for (i=0; i<num_samples; i++)
        buf_2[i] *= 0.5f;

    // verify input spectrum
    autotest_psd_s regions_orig[] = {
      {.fmin=-0.5,       .fmax= ft-0.03,   .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin= ft-0.01,   .fmax= ft+0.01,   .pmin=-40,.pmax= 0,      .test_lo=1, .test_hi=0},
      {.fmin= ft+0.03,   .fmax= f0-0.5*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin= f0-0.3*bw, .fmax= f0+0.3*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=+f0+0.5*bw, .fmax=+0.5,       .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signal(buf_0, num_samples, regions_orig, 5,
        liquid_autotest_verbose ? "autotest/logs/iirhilbf_filter_orig.m" : NULL);

    // verify interpolated spectrum
    autotest_psd_s regions_c2r[] = {
      {.fmin=-0.5,       .fmax=-f0-0.5*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin=-f0-0.3*bw, .fmax=-f0+0.3*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=-f0+0.5*bw, .fmax=+f0-0.5*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin= f0-0.3*bw, .fmax= f0+0.3*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=+f0+0.5*bw, .fmax=+0.5,       .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signalf(buf_1, num_samples, regions_c2r, 5,
        liquid_autotest_verbose ? "autotest/logs/iirhilbf_filter_c2r.m" : NULL);

    // verify decimated spectrum (using same regions as original)
    autotest_psd_s regions_r2c[] = {
      {.fmin=-0.5,       .fmax= f0-0.5*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin= f0-0.3*bw, .fmax= f0+0.3*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=+f0+0.5*bw, .fmax=+0.5,       .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signal(buf_2, num_samples, regions_r2c, 3,
        liquid_autotest_verbose ? "autotest/logs/iirhilbf_filter_r2c.m" : NULL);

    // destroy filter object and free memory
    iirhilbf_destroy(q);
}

void autotest_iirhilbf_invalid_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping iirhilbf config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check that object returns NULL for invalid configurations
    CONTEND_ISNULL(iirhilbf_create(LIQUID_IIRDES_BUTTER, 0, 0.1f, 60.0f)); // order out of range
    CONTEND_ISNULL(iirhilbf_create_default(0)); // order out of range

    // create proper object and test configuration methods
    iirhilbf q = iirhilbf_create(LIQUID_IIRDES_BUTTER,5,0.1f,60.0f);
    iirhilbf_print(q);
    iirhilbf_destroy(q);
}

void autotest_iirhilbf_copy_interp()
{
    // create base object
    iirhilbf q0 = iirhilbf_create(LIQUID_IIRDES_ELLIP,7,0.1f,80.0f);

    // run interpolator on random data
    unsigned int i;
    float y0[2], y1[2];
    for (i=0; i<80; i++) {
        float complex x = randnf() + _Complex_I*randnf();
        iirhilbf_interp_execute(q0, x, y0);
    }

    // copy object
    iirhilbf q1 = iirhilbf_copy(q0);

    for (i=0; i<80; i++) {
        float complex x = randnf() + _Complex_I*randnf();
        iirhilbf_interp_execute(q0, x, y0);
        iirhilbf_interp_execute(q1, x, y1);
        if (liquid_autotest_verbose) {
            printf("%3u : %12.8f +j%12.8f > {%12.8f, %12.8f}, {%12.8f, %12.8f}\n",
                    i, crealf(x), cimagf(x), y0[0], y0[1], y1[0], y1[1]);
        }
        CONTEND_EQUALITY(y0[0], y1[0]);
        CONTEND_EQUALITY(y0[1], y1[1]);
    }

    // destroy objects
    iirhilbf_destroy(q0);
    iirhilbf_destroy(q1);
}

void autotest_iirhilbf_copy_decim()
{
    // create base object
    iirhilbf q0 = iirhilbf_create(LIQUID_IIRDES_ELLIP,7,0.1f,80.0f);

    // run interpolator on random data
    unsigned int i;
    float x[2];
    float complex y0, y1;
    for (i=0; i<80; i++) {
        x[0] = randnf();
        x[1] = randnf();
        iirhilbf_decim_execute(q0, x, &y0);
    }

    // copy object and run samples through each in parallel
    iirhilbf q1 = iirhilbf_copy(q0);
    for (i=0; i<80; i++) {
        x[0] = randnf();
        x[1] = randnf();
        iirhilbf_decim_execute(q0, x, &y0);
        iirhilbf_decim_execute(q1, x, &y1);
        if (liquid_autotest_verbose) {
            printf("%3u : {%12.8f %12.8f} > %12.8f +j%12.8f, %12.8f +j%12.8f\n",
                    i, x[0], x[1], crealf(y0), cimagf(y0), crealf(y1), cimagf(y1));
        }
        CONTEND_EQUALITY(y0, y1);
    }

    // destroy filter objects
    iirhilbf_destroy(q0);
    iirhilbf_destroy(q1);
}

