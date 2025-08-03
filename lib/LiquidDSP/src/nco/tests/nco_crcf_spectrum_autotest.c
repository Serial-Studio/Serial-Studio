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

#include <stdlib.h>
#include <complex.h>
#include "autotest/autotest.h"
#include "liquid.h"

// autotest helper function
//  _id     : test id for saving log file
//  _type   : NCO type (e.g. LIQUID_NCO)
//  _freq   : normalized frequency (f/Fs), _freq in [-0.5,0.5)
void nco_crcf_spectrum_test(int _id, int _type, float _freq)
{
    unsigned long int num_samples = 1UL<<16;
    unsigned int nfft = 9600;

    // create object and initialize
    nco_crcf nco = nco_crcf_create(_type);
    nco_crcf_set_frequency(nco, 2*M_PI*_freq);

    // sample buffer
    unsigned int  buf_len = 3*nfft;
    float complex buf_0[buf_len];
    float complex buf_1[buf_len];
    unsigned int i;
    for (i=0; i<buf_len; i++)
        buf_0[i] = 1.0f / sqrtf((float)nfft);

    // spectrogram for measuring response
    spgramcf psd = spgramcf_create(nfft, LIQUID_WINDOW_BLACKMANHARRIS, nfft, nfft/2);
    //spgramcf psd = spgramcf_create_default(nfft);

    // generate signal
    while (spgramcf_get_num_samples_total(psd) < num_samples) {
        // mix signal up in block
        nco_crcf_mix_block_up(nco, buf_0, buf_1, buf_len);

        // apply tapering to beginning to prevent initial transient
        if (spgramcf_get_num_samples_total(psd) == 0UL) {
            for (i=0; i<buf_len; i++)
                buf_1[i] *= liquid_hann(i,2*buf_len);
        }

        // write samples to spectrogram
        spgramcf_write(psd, buf_1, buf_len);
    }

    // verify results: full spectrum
    autotest_psd_s regions[] = {
        {.fmin=-0.50f,      .fmax=_freq-0.002,.pmin=  0, .pmax=-60, .test_lo=0, .test_hi=1},
        {.fmin=_freq-0.002, .fmax=_freq+0.002,.pmin=  0, .pmax=  0, .test_lo=0, .test_hi=1},
        {.fmin=_freq+0.002, .fmax=0.5,        .pmin=  0, .pmax=-60, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/nco_crcf_spectrum_%s_f%.2d.m", _type==LIQUID_NCO ? "nco" : "vco", _id);
    liquid_autotest_validate_psd_spgramcf(psd, regions, 3,
        liquid_autotest_verbose ? filename : NULL);

    // destroy object
    nco_crcf_destroy(nco);
    spgramcf_destroy(psd);
}

// test NCO
void autotest_nco_crcf_spectrum_nco_f00() { nco_crcf_spectrum_test( 0, LIQUID_NCO, 0.    ); }
void autotest_nco_crcf_spectrum_nco_f01() { nco_crcf_spectrum_test( 1, LIQUID_NCO, 0.1234); }
void autotest_nco_crcf_spectrum_nco_f02() { nco_crcf_spectrum_test( 2, LIQUID_NCO,-0.1234); }
void autotest_nco_crcf_spectrum_nco_f03() { nco_crcf_spectrum_test( 3, LIQUID_NCO, 0.25  ); }
void autotest_nco_crcf_spectrum_nco_f04() { nco_crcf_spectrum_test( 4, LIQUID_NCO, 0.1   ); }

// test VCO interp
void autotest_nco_crcf_spectrum_vco_f00() { nco_crcf_spectrum_test( 0, LIQUID_VCO_INTERP, 0.    ); }
void autotest_nco_crcf_spectrum_vco_f01() { nco_crcf_spectrum_test( 1, LIQUID_VCO_INTERP, 0.1234); }
void autotest_nco_crcf_spectrum_vco_f02() { nco_crcf_spectrum_test( 2, LIQUID_VCO_INTERP,-0.1234); }
void autotest_nco_crcf_spectrum_vco_f03() { nco_crcf_spectrum_test( 3, LIQUID_VCO_INTERP, 0.25  ); }
void autotest_nco_crcf_spectrum_vco_f04() { nco_crcf_spectrum_test( 4, LIQUID_VCO_INTERP, 0.1   ); }

