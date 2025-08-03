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

#define J _Complex_I

//
// AUTOTEST: Hilbert transform, 2:1 decimator
//
void autotest_firhilbf_decim()
{
    float x[32] = {
         1.0000,  0.7071,  0.0000, -0.7071, -1.0000, -0.7071, -0.0000,  0.7071,
         1.0000,  0.7071,  0.0000, -0.7071, -1.0000, -0.7071, -0.0000,  0.7071,
         1.0000,  0.7071,  0.0000, -0.7071, -1.0000, -0.7071, -0.0000,  0.7071,
         1.0000,  0.7071, -0.0000, -0.7071, -1.0000, -0.7071, -0.0000,  0.7071
    };

    float complex test[16] = {
         0.0000+J*-0.0055, -0.0000+J* 0.0231,  0.0000+J*-0.0605, -0.0000+J* 0.1459,
         0.0000+J*-0.5604, -0.7071+J*-0.7669, -0.7071+J* 0.7294,  0.7071+J* 0.7008,
         0.7071+J*-0.7064, -0.7071+J*-0.7064, -0.7071+J* 0.7064,  0.7071+J* 0.7064,
         0.7071+J*-0.7064, -0.7071+J*-0.7064, -0.7071+J* 0.7064,  0.7071+J* 0.7064
    };

    float complex y[16];
    unsigned int m=5;   // h_len = 4*m+1 = 21
    firhilbf ht = firhilbf_create(m,60.0f);
    float tol=0.005f;

    // run decimator
    unsigned int i;
    for (i=0; i<16; i++)
        firhilbf_decim_execute(ht, &x[2*i], &y[i]);

    if (liquid_autotest_verbose) {
        printf("hilbert transform decimator output:\n");
        for (i=0; i<16; i++)
            printf("  y(%3u) = %8.5f + j*%8.5f;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }

    // run validation
    for (i=0; i<16; i++) {
        CONTEND_DELTA(crealf(y[i]), crealf(test[i]), tol);
        CONTEND_DELTA(cimagf(y[i]), cimagf(test[i]), tol);
    }

    // clean up filter object
    firhilbf_destroy(ht);
}

//
// AUTOTEST: Hilbert transform, 1:2 interpolator
//
void autotest_firhilbf_interp()
{
    float complex x[16] = {
         1.0000+J* 0.0000, -0.0000+J*-1.0000, -1.0000+J* 0.0000,  0.0000+J* 1.0000,
         1.0000+J*-0.0000, -0.0000+J*-1.0000, -1.0000+J* 0.0000,  0.0000+J* 1.0000,
         1.0000+J*-0.0000, -0.0000+J*-1.0000, -1.0000+J* 0.0000,  0.0000+J* 1.0000,
         1.0000+J*-0.0000,  0.0000+J*-1.0000, -1.0000+J* 0.0000,  0.0000+J* 1.0000
    };

    float test[32] = {
         0.0000, -0.0055, -0.0000, -0.0231, -0.0000, -0.0605, -0.0000, -0.1459,
        -0.0000, -0.5604, -0.0000,  0.7669,  1.0000,  0.7294,  0.0000, -0.7008,
        -1.0000, -0.7064, -0.0000,  0.7064,  1.0000,  0.7064,  0.0000, -0.7064,
        -1.0000, -0.7064, -0.0000,  0.7064,  1.0000,  0.7064,  0.0000, -0.7064
    };


    float y[32];
    unsigned int m=5;   // h_len = 4*m+1 = 21
    firhilbf ht = firhilbf_create(m,60.0f);
    float tol=0.005f;

    // run interpolator
    unsigned int i;
    for (i=0; i<16; i++)
        firhilbf_interp_execute(ht, x[i], &y[2*i]);

    if (liquid_autotest_verbose) {
        printf("hilbert transform interpolator output:\n");
        for (i=0; i<32; i++)
            printf("  y(%3u) = %8.5f;\n", i+1, y[i]);
    }

    // run validation
    for (i=0; i<32; i++) {
        CONTEND_DELTA(y[i], test[i], tol);
    }

    // clean up filter object
    firhilbf_destroy(ht);
}

// test end-to-end power specral density
void autotest_firhilbf_psd()
{
    float        tol  = 1;  // error tolerance [dB]
    float        bw = 0.4f; // pulse bandwidth
    float        As = 60.0f;// transform stop-band suppression
    unsigned int p    = 40; // pulse semi-length
    unsigned int m    = 25; // Transform delay

    // create transform
    firhilbf q = firhilbf_create(m,As);
    firhilbf_print(q);

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
    firhilbf_interp_execute_block(q, buf_0, num_samples, buf_1);

    // clear object
    firhilbf_reset(q);

    // run decimation
    firhilbf_decim_execute_block(q, buf_1, num_samples, buf_2);

    // verify input spectrum
    autotest_psd_s regions_orig[] = {
      {.fmin=-0.5,    .fmax=-0.5*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin=-0.3*bw, .fmax=+0.3*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=+0.5*bw, .fmax=+0.5,    .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signal(buf_0, num_samples, regions_orig, 3,
        liquid_autotest_verbose ? "autotest/logs/firhilbf_orig.m" : NULL);

    // verify interpolated spectrum
    autotest_psd_s regions_interp[] = {
      {.fmin=-0.5,          .fmax=-0.25-0.25*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin=-0.25-0.15*bw, .fmax=-0.25+0.15*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin=-0.25+0.25*bw, .fmax=+0.25-0.25*bw, .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
      {.fmin= 0.25-0.15*bw, .fmax= 0.25+0.15*bw, .pmin=-1, .pmax=+1,      .test_lo=1, .test_hi=1},
      {.fmin= 0.25+0.25*bw, .fmax= 0.5,          .pmin= 0, .pmax=-As+tol, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signalf(buf_1, 2*num_samples, regions_interp, 5,
        liquid_autotest_verbose ? "autotest/logs/firhilbf_interp.m" : NULL);

    // verify decimated spectrum (using same regions as original)
    liquid_autotest_validate_psd_signal(buf_2, num_samples, regions_orig, 3,
        liquid_autotest_verbose ? "autotest/logs/firhilbf_decim.m" : NULL);

    // destroy filter object and free memory
    firhilbf_destroy(q);
}

void autotest_firhilbf_invalid_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firhilbf config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check that object returns NULL for invalid configurations
    CONTEND_ISNULL(firhilbf_create( 0, 60.0f)); // m too small
    CONTEND_ISNULL(firhilbf_create( 1, 60.0f)); // m too small

    // create proper object but test invalid internal configurations
    //firhilbf q = firhilbf_create(12,60.0f);
    //firhilbf_destroy(q);
}

void autotest_firhilbf_copy_interp()
{
    firhilbf q0 = firhilbf_create(12,120.0f);

    // run interpolator on random data
    unsigned int i;
    float y0[2], y1[2];
    for (i=0; i<80; i++) {
        float complex x = randnf() + _Complex_I*randnf();
        firhilbf_interp_execute(q0, x, y0);
    }

    // copy object
    firhilbf q1 = firhilbf_copy(q0);

    for (i=0; i<80; i++) {
        float complex x = randnf() + _Complex_I*randnf();
        firhilbf_interp_execute(q0, x, y0);
        firhilbf_interp_execute(q1, x, y1);
        if (liquid_autotest_verbose) {
            printf("%3u : %12.8f +j%12.8f > {%12.8f, %12.8f}, {%12.8f, %12.8f}\n",
                    i, crealf(x), cimagf(x), y0[0], y0[1], y1[0], y1[1]);
        }
        CONTEND_EQUALITY(y0[0], y1[0]);
        CONTEND_EQUALITY(y0[1], y1[1]);
    }

    // destroy objects
    firhilbf_destroy(q0);
    firhilbf_destroy(q1);
}

void autotest_firhilbf_copy_decim()
{
    firhilbf q0 = firhilbf_create(12,120.0f);

    // run interpolator on random data
    unsigned int i;
    float x[2];
    float complex y0, y1;
    for (i=0; i<80; i++) {
        x[0] = randnf();
        x[1] = randnf();
        firhilbf_decim_execute(q0, x, &y0);
    }

    // copy object
    firhilbf q1 = firhilbf_copy(q0);

    for (i=0; i<80; i++) {
        x[0] = randnf();
        x[1] = randnf();
        firhilbf_decim_execute(q0, x, &y0);
        firhilbf_decim_execute(q1, x, &y1);
        if (liquid_autotest_verbose) {
            printf("%3u : {%12.8f %12.8f} > %12.8f +j%12.8f, %12.8f +j%12.8f\n",
                    i, x[0], x[1], crealf(y0), cimagf(y0), crealf(y1), cimagf(y1));
        }
        CONTEND_EQUALITY(y0, y1);
    }

    // destroy objects
    firhilbf_destroy(q0);
    firhilbf_destroy(q1);
}

