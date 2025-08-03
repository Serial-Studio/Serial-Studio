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

// test half-band filterbank (analyzer)
void autotest_resamp2_analysis()
{
    unsigned int m=5;       // filter semi-length (actual length: 4*m+1)
    unsigned int n=37;      // number of input samples
    float as=60.0f;         // stop-band attenuation [dB]
    float f0 =  0.0739f;    // low frequency signal
    float f1 = -0.1387f;    // high frequency signal (+pi)
    float tol = 1e-3f;      // error tolerance

    unsigned int i;

    // allocate memory for data arrays
    float complex x[2*n+2*m+1]; // input signal (with delay)
    float complex y0[n];        // low-pass output
    float complex y1[n];        // high-pass output

    // generate the baseband signal
    for (i=0; i<2*n+2*m+1; i++)
        x[i] = i < 2*n ? cexpf(_Complex_I*f0*i) + cexpf(_Complex_I*(M_PI+f1)*i) : 0.0f;

    // create/print the half-band resampler, with a specified
    // stopband attenuation level
    resamp2_crcf q = resamp2_crcf_create(m,0,as);

    // run half-band decimation
    float complex y_hat[2];
    for (i=0; i<n; i++) {
        resamp2_crcf_analyzer_execute(q, &x[2*i], y_hat);
        y0[i] = y_hat[0];
        y1[i] = y_hat[1];
    }

    // clean up allocated objects
    resamp2_crcf_destroy(q);

    // validate output
    for (i=m; i<n-m; i++) {
        CONTEND_DELTA( crealf(y0[i+m]), cosf(2*f0*(i+0.5f)), tol );
        CONTEND_DELTA( cimagf(y0[i+m]), sinf(2*f0*(i+0.5f)), tol );

        CONTEND_DELTA( crealf(y1[i+m]), cosf(2*f1*(i+0.5f)), tol );
        CONTEND_DELTA( cimagf(y1[i+m]), sinf(2*f1*(i+0.5f)), tol );
    }

#if 0
    // debugging
    const char filename[] = "autotest/logs/resamp2_analysis_test.m";
    FILE * fid = fopen(filename, "w");
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    for (i=0; i<2*n; i++)
        fprintf(fid,"x(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
    for (i=0; i<n; i++) {
        fprintf(fid,"y0(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y0[i]), cimagf(y0[i]));
        fprintf(fid,"y1(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y1[i]), cimagf(y1[i]));
    }
    // save expected values
    for (i=0; i<n; i++) {
        fprintf(fid,"z0(%3u) = %12.4e + j*%12.4e;\n", i+1, cosf(i*2*f0), sinf(i*2*f0));
        fprintf(fid,"z1(%3u) = %12.4e + j*%12.4e;\n", i+1, cosf(i*2*f1), sinf(i*2*f1));
    }
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(length(y0)-1);\n");
    //fprintf(fid,"plot(t,real(z0),t-m+0.5,real(y0));\n");
    fprintf(fid,"plot(t,real(z1),t-m+0.5,real(y1));\n");
    fclose(fid);
    printf("results written to '%s'\n",filename);
#endif
}

// test half-band filterbank (synthesizer)
void autotest_resamp2_synthesis()
{
    unsigned int m=5;       // filter semi-length (actual length: 4*m+1)
    unsigned int n=37;      // number of input samples
    float as=60.0f;         // stop-band attenuation [dB]
    float f0 =  0.0739f;    // low frequency signal
    float f1 = -0.1387f;    // high frequency signal (+pi)
    float tol = 3e-3f;      // error tolerance

    unsigned int i;

    // allocate memory for data arrays
    float complex x0[n+2*m+1];  // input signal (with delay)
    float complex x1[n+2*m+1];  // input signal (with delay)
    float complex y[2*n];       // synthesized output

    // generate the baseband signals
    for (i=0; i<n+2*m+1; i++) {
        x0[i] = i < 2*n ? cexpf(_Complex_I*f0*i) : 0.0f;
        x1[i] = i < 2*n ? cexpf(_Complex_I*f1*i) : 0.0f;
    }

    // create/print the half-band resampler, with a specified
    // stopband attenuation level
    resamp2_crcf q = resamp2_crcf_create(m,0,as);

    // run synthesis
    float complex x_hat[2];
    for (i=0; i<n; i++) {
        x_hat[0] = x0[i];
        x_hat[1] = x1[i];
        resamp2_crcf_synthesizer_execute(q, x_hat, &y[2*i]);
    }

    // clean up allocated objects
    resamp2_crcf_destroy(q);

    // validate output
    for (i=m; i<n-2*m; i++) {
        CONTEND_DELTA( crealf(y[i+2*m]), cosf(0.5f*f0*i) + cosf((M_PI+0.5f*f1)*i), tol );
        CONTEND_DELTA( cimagf(y[i+2*m]), sinf(0.5f*f0*i) + sinf((M_PI+0.5f*f1)*i), tol );
    }

#if 0
    // debugging
    const char filename[] = "autotest/logs/resamp2_synthesis_test.m";
    FILE * fid = fopen(filename, "w");
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    for (i=0; i<n+2*m+1; i++) {
        fprintf(fid,"x0(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(x0[i]), cimagf(x0[i]));
        fprintf(fid,"x1(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(x1[i]), cimagf(x1[i]));
    }
    for (i=0; i<2*n; i++)
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    // save expected values
    for (i=0; i<2*n; i++) {
        fprintf(fid,"z(%3u) = %12.4e + j*%12.4e;\n", i+1, cosf(i*0.5f*f0) + cosf(i*(M_PI+0.5f*f1)),
                                                          sinf(i*0.5f*f0) + sinf(i*(M_PI+0.5f*f1)));
    }
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(length(y)-1);\n");
    fprintf(fid,"plot(t,real(z),t-2*m,real(y));\n");
    fclose(fid);
    printf("results written to '%s'\n",filename);
#endif
}

// test half-band resampler filter response
void testbench_resamp2_crcf_filter(unsigned int _m, float _as)
{
    // error tolerance [dB]
    float tol = 0.5f;

    // create the half-band resampler
    resamp2_crcf q = resamp2_crcf_create(_m,0,_as);

    // get impulse response
    unsigned int h_len = 4*_m+1;
    float complex h_0[h_len];   // low-frequency response
    float complex h_1[h_len];   // high-frequency response
    unsigned int i;
    for (i=0; i<h_len; i++)
        resamp2_crcf_filter_execute(q, i==0 ? 1 : 0, h_0+i, h_1+i);

    // clean up allocated objects
    resamp2_crcf_destroy(q);

    // compute expected transition band (extend slightly for relaxed constraints)
    float ft = estimate_req_filter_df(_as, h_len) * 1.1;

    // verify low-pass frequency response
    autotest_psd_s regions_h0[] = {
      {.fmin=-0.5,       .fmax=-0.25-ft/2, .pmin= 0, .pmax=-_as+tol, .test_lo=0, .test_hi=1},
      {.fmin=-0.25+ft/2, .fmax=+0.25-ft/2, .pmin=-1, .pmax=+1,       .test_lo=1, .test_hi=1},
      {.fmin=+0.25+ft/2, .fmax=+0.5,       .pmin= 0, .pmax=-_as+tol, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/resamp2_crcf_filter_lo_m%u_as%.0f.m", _m, _as);
    liquid_autotest_validate_psd_signal(h_0, h_len, regions_h0, 3,
        liquid_autotest_verbose ? filename : NULL);

    // verify high-pass frequency response
    autotest_psd_s regions_h1[] = {
      {.fmin=-0.5,       .fmax=-0.25-ft/2, .pmin=-1, .pmax=+1,       .test_lo=1, .test_hi=1},
      {.fmin=-0.25+ft/2, .fmax=+0.25-ft/2, .pmin= 0, .pmax=-_as+tol, .test_lo=0, .test_hi=1},
      {.fmin=+0.25+ft/2, .fmax=+0.5,       .pmin=-1, .pmax=+1,       .test_lo=1, .test_hi=1},
    };
    sprintf(filename,"autotest/logs/resamp2_crcf_filter_hi_m%u_as%.0f.m", _m, _as);
    liquid_autotest_validate_psd_signal(h_1, h_len, regions_h1, 3,
        liquid_autotest_verbose ? filename : NULL);
}

// test different configurations
void autotest_resamp2_crcf_filter_0(){ testbench_resamp2_crcf_filter( 4, 60.0f); }
void autotest_resamp2_crcf_filter_1(){ testbench_resamp2_crcf_filter( 7, 60.0f); }
void autotest_resamp2_crcf_filter_2(){ testbench_resamp2_crcf_filter(12, 60.0f); }
void autotest_resamp2_crcf_filter_3(){ testbench_resamp2_crcf_filter(15, 80.0f); }
void autotest_resamp2_crcf_filter_4(){ testbench_resamp2_crcf_filter(15,100.0f); }
void autotest_resamp2_crcf_filter_5(){ testbench_resamp2_crcf_filter(15,120.0f); }

void autotest_resamp2_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping resamp2 config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check that object returns NULL for invalid configurations
    CONTEND_ISNULL(resamp2_crcf_create( 0,  0.0f, 60.0f)); // m out of range
    CONTEND_ISNULL(resamp2_crcf_create( 1,  0.0f, 60.0f)); // m out of range
    CONTEND_ISNULL(resamp2_crcf_create( 2,  0.7f, 60.0f)); // f0 out of range
    CONTEND_ISNULL(resamp2_crcf_create( 2, -0.7f, 60.0f)); // f0 out of range
    CONTEND_ISNULL(resamp2_crcf_create( 2,  0.0f, -1.0f)); // as out of range

    // create proper object and test configurations
    resamp2_crcf q = resamp2_crcf_create( 4, 0.0f, 60.0f);
    CONTEND_EQUALITY(resamp2_crcf_get_delay(q), 2*4-1);
    resamp2_crcf_print(q);

    // redesign filter with new length
    q = resamp2_crcf_recreate(q, 8, 0.0f, 60.0f);
    CONTEND_EQUALITY(resamp2_crcf_get_delay(q), 2*8-1);

    // redesign filter with same length, but new stop-band suppression
    q = resamp2_crcf_recreate(q, 8, 0.0f, 80.0f);
    CONTEND_EQUALITY(resamp2_crcf_get_delay(q), 2*8-1);

    // test setting/getting properties
    resamp2_crcf_set_scale(q, 7.22f);
    float scale = 0.0f;
    resamp2_crcf_get_scale(q, &scale);
    CONTEND_EQUALITY(scale, 7.22f);

    // destroy object
    resamp2_crcf_destroy(q);
}

// test copy method
void autotest_resamp2_copy()
{
    // create original half-band resampler
    resamp2_crcf qa = resamp2_crcf_create(12,0,60.0f);

    // run random samples through filter
    float complex v, ya0, ya1, yb0, yb1;
    unsigned int i, num_samples = 80;
    for (i=0; i<num_samples; i++) {
        v = randnf() + _Complex_I*randnf();
        resamp2_crcf_filter_execute(qa, v, &ya0, &ya1);
    }

    // copy object
    resamp2_crcf qb = resamp2_crcf_copy(qa);

    // run random samples through both filters and compare
    for (i=0; i<num_samples; i++) {
        v = randnf() + _Complex_I*randnf();
        resamp2_crcf_filter_execute(qa, v, &ya0, &ya1);
        resamp2_crcf_filter_execute(qb, v, &yb0, &yb1);

        CONTEND_EQUALITY(ya0, yb0);
        CONTEND_EQUALITY(ya1, yb1);
    }

    // clean up allocated objects
    resamp2_crcf_destroy(qa);
    resamp2_crcf_destroy(qb);
}

