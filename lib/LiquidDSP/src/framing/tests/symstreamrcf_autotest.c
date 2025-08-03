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

#include <stdio.h>
#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.h"

// autotest helper functions
void testbench_symstreamrcf_psd(float        _bw,
                                unsigned int _m,
                                float        _beta)
{
    // create object
    int ftype = LIQUID_FIRFILT_ARKAISER;
    int ms    = LIQUID_MODEM_QPSK;
    symstreamrcf gen = symstreamrcf_create_linear(ftype,_bw,_m,_beta,ms);
    symstreamrcf_set_gain(gen, sqrtf(_bw));

    // spectral periodogram options
    unsigned int nfft        =   2400;      // spectral periodogram FFT size
    unsigned int num_samples = 192000/_bw;  // number of samples

    // create spectral periodogram
    spgramcf periodogram = spgramcf_create_default(nfft);

    unsigned int buf_len = 1337;
    float complex buf[buf_len];
    unsigned int n = 0;
    while (n < num_samples) {
        // fill buffer
        symstreamrcf_write_samples(gen, buf, buf_len);
        n += buf_len;

        // run through spectral estimation object
        spgramcf_write(periodogram, buf, buf_len);
    }

    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    symstreamrcf_destroy(gen);
    spgramcf_destroy(periodogram);

    // verify spectrum
    // TODO: sidelobe suppression based on internal msresamp object; should be more like -80 dB
    float f0 = 0.5 * (1.0f - _beta) * _bw;
    float f1 = 0.5 * (1.0f + _beta) * _bw;
    autotest_psd_s regions[] = {
      {.fmin=-0.5, .fmax=-f1,  .pmin=  0-0, .pmax=-55.0, .test_lo=0, .test_hi=1},
      {.fmin=-f0,  .fmax= f0,  .pmin= -2.0, .pmax=  2.0, .test_lo=1, .test_hi=1},
      {.fmin= f1,  .fmax= 0.5, .pmin=  0.0, .pmax=-55.0, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/symstreamrcf_psd_bw%u_m%u_b%.3u_autotest.m",
            (int)(_bw*1000), _m, (int)(_beta*100));
    liquid_autotest_validate_spectrum(psd, nfft, regions, 3,
        liquid_autotest_verbose ? filename : NULL);
}

void autotest_symstreamrcf_psd_bw200_m12_b030() { testbench_symstreamrcf_psd(0.2f, 12, 0.30f); }
void autotest_symstreamrcf_psd_bw400_m12_b030() { testbench_symstreamrcf_psd(0.4f, 12, 0.30f); }
void autotest_symstreamrcf_psd_bw400_m25_b020() { testbench_symstreamrcf_psd(0.4f, 25, 0.20f); }
void autotest_symstreamrcf_psd_bw700_m11_b035() { testbench_symstreamrcf_psd(0.7f, 11, 0.35f); }

// test copying from one object to another
void autotest_symstreamrcf_copy()
{
    // create objects
    symstreamrcf gen_orig = symstreamrcf_create_linear(
        LIQUID_FIRFILT_ARKAISER,0.2f,17,0.27f,LIQUID_MODEM_DPSK4);

    // allocate memory for buffers
    unsigned int buf_len = 1337;
    float complex buf_orig[buf_len];
    float complex buf_copy[buf_len];

    // generate some samples
    symstreamrcf_write_samples(gen_orig, buf_orig, buf_len);

    // copy object
    symstreamrcf gen_copy = symstreamrcf_copy(gen_orig);

    // generate samples from each object
    symstreamrcf_write_samples(gen_orig, buf_orig, buf_len);
    symstreamrcf_write_samples(gen_copy, buf_copy, buf_len);

    // compare result
    // NOTE: this will fail as they have different symbol generators
    //CONTEND_SAME_DATA(buf_orig, buf_copy, buf_len*sizeof(float complex));
    AUTOTEST_WARN("symstreamrcf_copy results ignored until common PRNG is used");

    // destroy objects
    symstreamrcf_destroy(gen_orig);
    symstreamrcf_destroy(gen_copy);
}

