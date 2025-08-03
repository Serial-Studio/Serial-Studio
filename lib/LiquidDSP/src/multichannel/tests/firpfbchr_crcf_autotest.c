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

#include <assert.h>
#include "autotest/autotest.h"
#include "liquid.h"

void autotest_firpfbchr_crcf()
{
    // options
    unsigned int M = 16;            // number of channels
    unsigned int P =  6;            // output decimation rate
    unsigned int m = 12;            // filter semi-length (symbols)
    unsigned int num_blocks=1<<16;  // number of symbols
    float As = 60.0f;               // filter stop-band attenuation
    
    // create filterbank objects from prototype
    firpfbchr_crcf qa = firpfbchr_crcf_create_kaiser(M, P, m, As);
    firpfbchr_crcf_print(qa);

    // create multi-signal source generator
    msourcecf gen = msourcecf_create_default();

    // add signals     (gen,  fc,    bw,    gain, {options})
    msourcecf_add_noise(gen,  0.00f, 1.00f, -60);   // wide-band noise
    msourcecf_add_noise(gen, -0.30f, 0.10f, -20);   // narrow-band noise
    msourcecf_add_noise(gen,  0.08f, 0.01f, -30);   // very narrow-band noise
    // modulated data
    msourcecf_add_modem(gen,
       0.1875f,             // center frequency
       0.065f,              // bandwidth (symbol rate)
       -20,                 // gain
       LIQUID_MODEM_QPSK,   // modulation scheme
       12,                  // filter semi-length
       0.3f);               // modem parameters

    // create spectral periodogoram
    unsigned int nfft = 2400;
    spgramcf     p0   = spgramcf_create_default(nfft);
    spgramcf     c1   = spgramcf_create_default(nfft);
    spgramcf     c3   = spgramcf_create_default(nfft);

    // run channelizer
    float complex buf_0[P];
    float complex buf_1[M];
    unsigned int i;
    for (i=0; i<num_blocks; i++) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf_0, P);

        // run analysis filterbank
        firpfbchr_crcf_push   (qa, buf_0);
        firpfbchr_crcf_execute(qa, buf_1);

        // push results through periodograms
        spgramcf_write(p0, buf_0, P);
        spgramcf_push (c1, buf_1[1]);
        spgramcf_push (c3, buf_1[3]);
    }

    // verify results: full spectrum
    autotest_psd_s regions[] = {
        {.fmin=-0.50f, .fmax=-0.37f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
        {.fmin=-0.34f, .fmax=-0.26f, .pmin=-25, .pmax=-15, .test_lo=1, .test_hi=1},
        {.fmin=-0.24f, .fmax= 0.05f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
        {.fmin= 0.10f, .fmax= 0.13f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
        {.fmin= 0.16f, .fmax= 0.21f, .pmin=-25, .pmax=-15, .test_lo=1, .test_hi=1},
        {.fmin= 0.25f, .fmax= 0.50f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_psd_spgramcf(p0, regions, 6,
        liquid_autotest_verbose ? "autotest/logs/firpfbchr_crcf_psd.m" : NULL);

    // verify results: channel 2
    autotest_psd_s regions_2[] = {
        {.fmin=-0.47f, .fmax= 0.05f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
        {.fmin= 0.08f, .fmax= 0.12f, .pmin=-35, .pmax=-25, .test_lo=1, .test_hi=1},
        {.fmin= 0.15f, .fmax= 0.47f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_psd_spgramcf(c1, regions_2, 3,
        liquid_autotest_verbose ? "autotest/logs/firpfbchr_crcf_c1.m" : NULL);

    // verify results: channel 3
    autotest_psd_s regions_3[] = {
        {.fmin=-0.47f, .fmax=-0.28f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
        {.fmin=-0.15f, .fmax=+0.15f, .pmin=-25, .pmax=-15, .test_lo=1, .test_hi=1},
        {.fmin= 0.28f, .fmax=+0.47f, .pmin=-65, .pmax=-55, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_psd_spgramcf(c3, regions_3, 3,
        liquid_autotest_verbose ? "autotest/logs/firpfbchr_crcf_c3.m" : NULL);

    // destroy objects
    firpfbchr_crcf_destroy(qa);
    msourcecf_destroy(gen);
    spgramcf_destroy(p0);
    spgramcf_destroy(c1);
    spgramcf_destroy(c3);
}

void autotest_firpfbchr_crcf_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firpfbchr_crcf config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // design prototype filter
    unsigned int h_len = 2*64*12+1;
    float h[2*64*12+1];
    liquid_firdes_kaiser(h_len, 0.1f, 60.0f, 0.0f, h);

    // check invalid function calls
    CONTEND_ISNULL(firpfbchr_crcf_create( 0, 76, 12,    h)) // too few channels
    CONTEND_ISNULL(firpfbchr_crcf_create(64,  0, 12,    h)) // decimation rate too small
    CONTEND_ISNULL(firpfbchr_crcf_create(64, 76,  0,    h)) // filter delay too small
    CONTEND_ISNULL(firpfbchr_crcf_create(64, 76, 12, NULL)) // coefficients pointer set to NULL

    // kaiser
    CONTEND_ISNULL(firpfbchr_crcf_create_kaiser( 0, 76, 12, 60.0f)) // too few channels
    CONTEND_ISNULL(firpfbchr_crcf_create_kaiser(64,  0, 12, 60.0f)) // decimation rate too small
    CONTEND_ISNULL(firpfbchr_crcf_create_kaiser(64, 76,  0, 60.0f)) // filter delay too small
    CONTEND_ISNULL(firpfbchr_crcf_create_kaiser(64, 76, 12, -1.0f)) // stop-band suppression out of range

    //CONTEND_ISNULL(firpfbchr_crcf_copy(NULL))

    // create proper object and test configurations
    firpfbchr_crcf q = firpfbchr_crcf_create_kaiser(64, 76, 12, 60.0f);

    CONTEND_EQUALITY(LIQUID_OK, firpfbchr_crcf_print(q))
    CONTEND_EQUALITY(64, firpfbchr_crcf_get_num_channels(q))
    CONTEND_EQUALITY(76, firpfbchr_crcf_get_decim_rate(q))
    CONTEND_EQUALITY(12, firpfbchr_crcf_get_m(q))

    firpfbchr_crcf_destroy(q);
}

