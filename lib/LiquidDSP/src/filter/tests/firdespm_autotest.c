/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// References:
//  [McClellan:1973] J. H. McClellan, T. W. Parks, L. R. Rabiner, "A
//      Computer Program for Designing Optimum FIR Linear Phase
//      Digital Filters," IEEE Transactions on Audio and
//      Electroacoustics, vol. AU-21, No. 6, December 1973.

#include "autotest/autotest.h"
#include "liquid.h"

void autotest_firdespm_bandpass_n24()
{
    // [McClellan:1973], Figure 7.

    // Initialize variables
    unsigned int n=24;
    unsigned int num_bands=2;
    float bands[4]  = {0.0f,0.08f,0.16f,0.50f};
    float des[2]    = {1.0f,0.0f};
    float weights[2]= {1.0f,1.0f};
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    float tol = 1e-4f;

    // Initialize pre-determined coefficient array
    float h0[24] = {
        0.33740917e-2f,
        0.14938299e-1f,
        0.10569360e-1f,
        0.25415067e-2f,
       -0.15929392e-1f,
       -0.34085343e-1f,
       -0.38112177e-1f,
       -0.14629169e-1f,
        0.40089541e-1f,
        0.11540713e-0f,
        0.18850752e-0f,
        0.23354606e-0f,
        // symmetry
        0.23354606e-0f,
        0.18850752e-0f,
        0.11540713e-0f,
        0.40089541e-1f,
       -0.14629169e-1f,
       -0.38112177e-1f,
       -0.34085343e-1f,
       -0.15929392e-1f,
        0.25415067e-2f,
        0.10569360e-1f,
        0.14938299e-1f,
        0.33740917e-2f
    };

    // Create filter
    float h[n];
    firdespm_run(n,num_bands,bands,des,weights,NULL,btype,h);

    // Ensure data are equal
    unsigned int i;
    for (i=0; i<n; i++)
        CONTEND_DELTA( h[i], h0[i], tol );
}


void autotest_firdespm_bandpass_n32()
{
    // [McClellan:1973], Figure 9.

    // Initialize variables
    unsigned int n=32;
    unsigned int num_bands = 3;
    float bands[6] = {  0.0f,   0.1f,
                        0.2f,   0.35f,
                        0.425f, 0.5f};
    float des[3] = {0.0f, 1.0f, 0.0f};
    float weights[3] = {10.0f, 1.0f, 10.0f};
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    float tol = 1e-4f;

    // Initialize pre-determined coefficient array
    float h0[32] = {
       -0.57534121e-2f,
        0.99027198e-3f,
        0.75733545e-2f,
       -0.65141192e-2f,
        0.13960525e-1f,
        0.22951469e-2f,
       -0.19994067e-1f,
        0.71369560e-2f,
       -0.39657363e-1f,
        0.11260114e-1f,
        0.66233643e-1f,
       -0.10497223e-1f,
        0.85136133e-1f,
       -0.12024993e+0f,
       -0.29678577e+0f,
        0.30410917e+0f,
        // symmetry
        0.30410917e+0f,
       -0.29678577e+0f,
       -0.12024993e+0f,
        0.85136133e-1f,
       -0.10497223e-1f,
        0.66233643e-1f,
        0.11260114e-1f,
       -0.39657363e-1f,
        0.71369560e-2f,
       -0.19994067e-1f,
        0.22951469e-2f,
        0.13960525e-1f,
       -0.65141192e-2f,
        0.75733545e-2f,
        0.99027198e-3f,
       -0.57534121e-2f
    };

    // Create filter
    float h[n];
    firdespm_run(n,num_bands,bands,des,weights,NULL,btype,h);

    // Ensure data are equal
    unsigned int i;
    for (i=0; i<n; i++)
        CONTEND_DELTA( h[i], h0[i], tol );
}

void autotest_firdespm_lowpass()
{
    // design filter
    unsigned int n  = 51;
    float        fc = 0.2f;
    float        As = 60.0f;
    float        mu = 0.0f;
    float        h[n];
    firdespm_lowpass(n,fc,As,mu,h);

    // verify resulting spectrum
    autotest_psd_s regions[] = {
      {.fmin=-0.5,   .fmax=-0.25,  .pmin= 0,    .pmax=-60,   .test_lo=0, .test_hi=1},
      {.fmin=-0.15,  .fmax=+0.15,  .pmin=-0.02, .pmax=+0.02, .test_lo=1, .test_hi=1},
      {.fmin= 0.25,  .fmax=+0.5,   .pmin= 0,    .pmax=-60,   .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signalf(h, n, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/firdespm_lowpass.m" : NULL);
}

// user-defined callback function defining response and weights
int callback_firdespm_autotest(double   _frequency,
                               void   * _userdata,
                               double * _desired,
                               double * _weight)
{
    *_desired = _frequency < 0.39 ? exp(20*fabs(_frequency)) : 0;
    *_weight  = _frequency < 0.39 ? exp(-10*_frequency) : 1;
    return 0;
}

void autotest_firdespm_callback()
{
    // filter design parameters
    unsigned int h_len = 81;    // inverse sinc filter length
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    unsigned int num_bands = 2;
    float        bands[4]  = {0.0, 0.35, 0.4, 0.5};

    // design filter
    float h[h_len];
    firdespm q = firdespm_create_callback(h_len,num_bands,bands,btype,
            callback_firdespm_autotest,NULL);
    firdespm_execute(q,h);
    firdespm_destroy(q);

    // verify resulting spectrum
    autotest_psd_s regions[] = {
      {.fmin=-0.50,  .fmax=-0.40,  .pmin= 0, .pmax=-20, .test_lo=0, .test_hi=1},
      {.fmin=-0.36,  .fmax=-0.30,  .pmin=52, .pmax= 62, .test_lo=1, .test_hi=1},
      {.fmin=-0.30,  .fmax=-0.20,  .pmin=34, .pmax= 53, .test_lo=1, .test_hi=1},
      {.fmin=-0.20,  .fmax=-0.10,  .pmin=15, .pmax= 36, .test_lo=1, .test_hi=1},
      {.fmin=-0.10,  .fmax=+0.10,  .pmin= 0, .pmax= 19, .test_lo=1, .test_hi=1},
      {.fmin= 0.10,  .fmax= 0.20,  .pmin=15, .pmax= 36, .test_lo=1, .test_hi=1},
      {.fmin= 0.20,  .fmax= 0.30,  .pmin=34, .pmax= 53, .test_lo=1, .test_hi=1},
      {.fmin= 0.30,  .fmax= 0.36,  .pmin=52, .pmax= 62, .test_lo=1, .test_hi=1},
      {.fmin= 0.40,  .fmax= 0.50,  .pmin= 0, .pmax=-20, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_psd_signalf(h, h_len, regions, 9,
        liquid_autotest_verbose ? "autotest/logs/firdespm_callback.m" : NULL);
}

// test halfband filter design by specifying filter semi-length and transition bandwidth
void testbench_firdespm_halfband_ft(unsigned int _m,
                                    float        _ft)
{
    unsigned int h_len = 4*_m + 1;
    float h[h_len];
    liquid_firdespm_halfband_ft(_m, _ft, h);

    // estimate stop band suppression
    float As = estimate_req_filter_As(_ft, h_len);

    // verify resulting spectrum
    float f0 = 0.25f - 0.5f*_ft;
    float f1 = 0.25f + 0.5f*_ft;
    autotest_psd_s regions[] = {
      {.fmin=-0.5, .fmax= -f1, .pmin= 0,   .pmax=-As,  .test_lo=0, .test_hi=1},
      {.fmin=-f0,  .fmax=  f0, .pmin=-0.1, .pmax= 0.1, .test_lo=1, .test_hi=1},
      {.fmin= f1,  .fmax= 0.5, .pmin= 0,   .pmax=-As,  .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/firdespm_halfband_m%u_ft%.3u.m", _m, (int)(_ft*1000));
    liquid_autotest_validate_psd_signalf(h, h_len, regions, 3,
        liquid_autotest_verbose ? filename : NULL);
}

void autotest_firdespm_halfband_m2_ft400()  { testbench_firdespm_halfband_ft( 3, 0.400f); }
void autotest_firdespm_halfband_m4_ft400()  { testbench_firdespm_halfband_ft( 4, 0.400f); }
void autotest_firdespm_halfband_m4_ft200()  { testbench_firdespm_halfband_ft( 4, 0.200f); }
void autotest_firdespm_halfband_m10_ft200() { testbench_firdespm_halfband_ft(10, 0.200f); }
void autotest_firdespm_halfband_m12_ft100() { testbench_firdespm_halfband_ft(12, 0.100f); }
void autotest_firdespm_halfband_m20_ft050() { testbench_firdespm_halfband_ft(20, 0.050f); }
void autotest_firdespm_halfband_m40_ft050() { testbench_firdespm_halfband_ft(40, 0.050f); }
void autotest_firdespm_halfband_m80_ft010() { testbench_firdespm_halfband_ft(80, 0.010f); }

void autotest_firdespm_copy()
{
    // create valid object
    float bands[4] = {0.0, 0.2, 0.3, 0.5};  // regions
    float   des[2] = {1.0,      0.0};       // desired values
    float     w[2] = {1.0,      1.0};       // weights
    liquid_firdespm_wtype wtype[2] = {LIQUID_FIRDESPM_FLATWEIGHT, LIQUID_FIRDESPM_FLATWEIGHT};
    firdespm q0 = firdespm_create(51, 2, bands, des, w, wtype, LIQUID_FIRDESPM_BANDPASS);

    // copy object
    firdespm q1 = firdespm_copy(q0);

    // execute both
    float h0[51], h1[51];
    firdespm_execute(q0, h0);
    firdespm_execute(q1, h1);
    CONTEND_SAME_DATA(h0, h1, 51*sizeof(float));

    // destroy objects
    firdespm_destroy(q0);
    firdespm_destroy(q1);
}

void autotest_firdespm_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firdespm config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    float h[51];
    CONTEND_EQUALITY(   LIQUID_OK, firdespm_lowpass(51, 0.2, 60, 0.0, h) ) // ok
    CONTEND_INEQUALITY( LIQUID_OK, firdespm_lowpass( 0, 0.2, 60, 0.0, h) )
    CONTEND_INEQUALITY( LIQUID_OK, firdespm_lowpass(51, 0.2, 60,-1.0, h) )
    CONTEND_INEQUALITY( LIQUID_OK, firdespm_lowpass(51, 0.2, 60, 1.0, h) )
    CONTEND_INEQUALITY( LIQUID_OK, firdespm_lowpass(51, 0.8, 60, 0.0, h) )
    CONTEND_INEQUALITY( LIQUID_OK, firdespm_lowpass(51,-0.2, 60, 0.0, h) )

    // try to create object with filter length 0
    CONTEND_ISNULL( firdespm_create( 0, 3, NULL, NULL, NULL, NULL, LIQUID_FIRDESPM_BANDPASS) )

    // try to create object with 0 bands
    CONTEND_ISNULL( firdespm_create(71, 0, NULL, NULL, NULL, NULL, LIQUID_FIRDESPM_BANDPASS) )

    // create valid object
    float bands[4] = {0.0, 0.2, 0.3, 0.5};  // regions
    float   des[2] = {1.0,      0.0};       // desired values
    float     w[2] = {1.0,      1.0};       // weights
    liquid_firdespm_wtype wtype[2] = {LIQUID_FIRDESPM_FLATWEIGHT, LIQUID_FIRDESPM_FLATWEIGHT};
    firdespm q = firdespm_create(51, 2, bands, des, w, wtype, LIQUID_FIRDESPM_BANDPASS);
    CONTEND_EQUALITY(   LIQUID_OK, firdespm_print(q) )
    firdespm_destroy(q);

    // invalid bands & weights
    float bands_0[4] = { 0.0, 0.3, 0.2, 0.5}; // overlapping bands
    float bands_1[4] = {-0.1, 0.0, 0.3, 0.6}; // bands out of range
    float w_0    [2] = { 1.0,-1.0};           // weights out of range

    // try to create regular object with invalid configuration
    CONTEND_ISNULL( firdespm_create( 0, 2, bands,   des, w  , wtype, LIQUID_FIRDESPM_BANDPASS) )
    CONTEND_ISNULL( firdespm_create(51, 0, bands,   des, w  , wtype, LIQUID_FIRDESPM_BANDPASS) )
    CONTEND_ISNULL( firdespm_create(51, 2, bands_0, des, w  , wtype, LIQUID_FIRDESPM_BANDPASS) )
    CONTEND_ISNULL( firdespm_create(51, 2, bands_1, des, w  , wtype, LIQUID_FIRDESPM_BANDPASS) )
    CONTEND_ISNULL( firdespm_create(51, 2, bands,   des, w_0, wtype, LIQUID_FIRDESPM_BANDPASS) )

    // try to create callback object with invalid configuration
    CONTEND_ISNULL( firdespm_create_callback( 0, 2, bands,   LIQUID_FIRDESPM_BANDPASS, NULL, NULL) )
    CONTEND_ISNULL( firdespm_create_callback(51, 0, bands,   LIQUID_FIRDESPM_BANDPASS, NULL, NULL) )
    CONTEND_ISNULL( firdespm_create_callback(51, 2, bands_0, LIQUID_FIRDESPM_BANDPASS, NULL, NULL) )
    CONTEND_ISNULL( firdespm_create_callback(51, 2, bands_1, LIQUID_FIRDESPM_BANDPASS, NULL, NULL) )
}

void autotest_firdespm_differentiator()
{
    AUTOTEST_WARN("firdespm_differentiator(), unsupported configuration");
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firdespm differentiator test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // create valid object
    unsigned int n = 51;
    float bands[4] = {0.0, 0.2, 0.3, 0.5};  // regions
    float   des[2] = {1.0,      0.0};       // desired values
    float     w[2] = {1.0,      1.0};       // weights
    float     h[n];
    liquid_firdespm_wtype wtype[2] = {LIQUID_FIRDESPM_FLATWEIGHT, LIQUID_FIRDESPM_FLATWEIGHT};
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_DIFFERENTIATOR;
    firdespm q = firdespm_create(n, 2, bands, des, w, wtype, btype);
    // unsupported configuration
    CONTEND_EQUALITY( LIQUID_EINT, firdespm_execute(q,h) )
    firdespm_destroy(q);
}

void autotest_firdespm_hilbert()
{
    AUTOTEST_WARN("firdespm_hilbert(), unsupported configuration");
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firdespm hilbert test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // create valid object
    unsigned int n = 51;
    float bands[4] = {0.0, 0.2, 0.3, 0.5};  // regions
    float   des[2] = {1.0,      0.0};       // desired values
    float     w[2] = {1.0,      1.0};       // weights
    float     h[n];
    liquid_firdespm_wtype wtype[2] = {LIQUID_FIRDESPM_FLATWEIGHT, LIQUID_FIRDESPM_FLATWEIGHT};
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_HILBERT;
    firdespm q = firdespm_create(n, 2, bands, des, w, wtype, btype);
    // unsupported configuration
    CONTEND_EQUALITY( LIQUID_EINT, firdespm_execute(q,h) )
    firdespm_destroy(q);
}

