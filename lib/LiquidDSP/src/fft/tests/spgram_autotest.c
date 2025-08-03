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

// test spectral periodogram (spgram) objects

#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

void testbench_spgramcf_noise(unsigned int _nfft,
                              unsigned int _wlen,
                              unsigned int _delay,
                              int          _wtype,
                              float        _noise_floor)
{
    unsigned int num_samples = 2000*_nfft;  // number of samples to generate
    float        nstd        = powf(10.0f,_noise_floor/20.0f); // noise std. dev.
    float        tol         = 0.5f; // error tolerance [dB]
    if (liquid_autotest_verbose) {
        printf("  spgramcf test  (noise): nfft=%6u, wtype=%24s, noise floor=%6.1f\n",
                _nfft, liquid_window_str[_wtype][1], _noise_floor);
    }

    // create spectral periodogram
    spgramcf q = NULL;
    if (_wlen==0 || _delay==0 || _wtype==LIQUID_WINDOW_UNKNOWN)
        q = spgramcf_create_default(_nfft);
    else
        q = spgramcf_create(_nfft, _wtype, _wlen, _delay);

    unsigned int i;
    for (i=0; i<num_samples; i++)
        spgramcf_push(q, nstd*( randnf() + _Complex_I*randnf() ) * M_SQRT1_2);

    // verify number of samples processed
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),       num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q), num_samples);

    // compute power spectral density output
    float psd[_nfft];
    spgramcf_get_psd(q, psd);

    // verify result
    for (i=0; i<_nfft; i++)
        CONTEND_DELTA(psd[i], _noise_floor, tol)

    // destroy objects
    spgramcf_destroy(q);
}

// test different transform sizes, default parameters
void autotest_spgramcf_noise_440()  { testbench_spgramcf_noise( 440, 0, 0, 0, -80.0); }
void autotest_spgramcf_noise_1024() { testbench_spgramcf_noise(1024, 0, 0, 0, -80.0); }
void autotest_spgramcf_noise_1200() { testbench_spgramcf_noise(1200, 0, 0, 0, -80.0); }

// test different transform sizes, specific parameters
void autotest_spgramcf_noise_custom_0() { testbench_spgramcf_noise(400, 400, 100, LIQUID_WINDOW_HAMMING, -80.0); }
void autotest_spgramcf_noise_custom_1() { testbench_spgramcf_noise(512, 200, 120, LIQUID_WINDOW_HAMMING, -80.0); }
void autotest_spgramcf_noise_custom_2() { testbench_spgramcf_noise(640, 100,  10, LIQUID_WINDOW_HAMMING, -80.0); }
void autotest_spgramcf_noise_custom_3() { testbench_spgramcf_noise(960,  83,  17, LIQUID_WINDOW_HAMMING, -80.0); }

// test different window types
void autotest_spgramcf_noise_hamming        () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_HAMMING,        -80.0); }
void autotest_spgramcf_noise_hann           () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_HANN,           -80.0); }
void autotest_spgramcf_noise_blackmanharris () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_BLACKMANHARRIS, -80.0); }
void autotest_spgramcf_noise_blackmanharris7() { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_BLACKMANHARRIS7,-80.0); }
void autotest_spgramcf_noise_kaiser         () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_KAISER,         -80.0); }
void autotest_spgramcf_noise_flattop        () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_FLATTOP,        -80.0); }
void autotest_spgramcf_noise_triangular     () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_TRIANGULAR,     -80.0); }
void autotest_spgramcf_noise_rcostaper      () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_RCOSTAPER,      -80.0); }
void autotest_spgramcf_noise_kbd            () { testbench_spgramcf_noise(800, 0, 0, LIQUID_WINDOW_KBD,            -80.0); }

void testbench_spgramcf_signal(unsigned int _nfft, int _wtype, float _fc, float _SNRdB)
{
    float bw = 0.25f; // signal bandwidth (relative)
    unsigned int m = 25;
    float beta = 0.2f, n0 = -80.0f, tol = 0.5f;
    if (liquid_autotest_verbose) {
        printf("  spgramcf test (signal): nfft=%6u, wtype=%24s, fc=%6.2f Fs, snr=%6.1f dB\n",
                _nfft, liquid_window_str[_wtype][1], _fc, _SNRdB);
    }

    // create objects
    spgramcf     q     = spgramcf_create(_nfft, _wtype, _nfft/2, _nfft/4);
    symstreamrcf gen   = symstreamrcf_create_linear(LIQUID_FIRFILT_KAISER,bw,m,beta,LIQUID_MODEM_QPSK);
    nco_crcf     mixer = nco_crcf_create(LIQUID_VCO);

    // set parameters
    float nstd = powf(10.0f,n0/20.0f); // noise std. dev.
    symstreamrcf_set_gain(gen, powf(10.0f, (n0 + _SNRdB + 10*log10f(bw))/20.0f));
    nco_crcf_set_frequency(mixer, 2*M_PI*_fc);

    // generate samples and push through spgram object
    unsigned int i, buf_len = 256, num_samples = 0;
    float complex buf[buf_len];
    while (num_samples < 2000*_nfft) {
        // generate block of samples
        symstreamrcf_write_samples(gen, buf, buf_len);

        // mix to desired frequency and add noise
        nco_crcf_mix_block_up(mixer, buf, buf, buf_len);
        for (i=0; i<buf_len; i++)
            buf[i] += nstd*(randnf()+_Complex_I*randnf())*M_SQRT1_2;

        // run samples through the spgram object
        spgramcf_write(q, buf, buf_len);
        num_samples += buf_len;
    }

    // verify result
    float psd[_nfft];
    spgramcf_get_psd(q, psd);
    float sn  = 10*log10f(powf(10,(_SNRdB+n0)/10.0f) + powf(10.0f,n0/10.0f));// signal + noise
    autotest_psd_s regions[] = {
        {.fmin=-0.5f,       .fmax=_fc-0.6f*bw, .pmin=n0-tol, .pmax=n0+tol, .test_lo=1, .test_hi=1},
        {.fmin=_fc-0.4f*bw, .fmax=_fc+0.4f*bw, .pmin=sn-tol, .pmax=sn+tol, .test_lo=1, .test_hi=1},
        {.fmin=_fc+0.6f*bw, .fmax=+0.5f,       .pmin=n0-tol, .pmax=n0+tol, .test_lo=1, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/spgramcf_signal_%s_n%u_f%c%.0f_s%c%.0f.m",
        liquid_window_str[_wtype][0], _nfft,
        _fc < 0 ? 'm' : 'p', fabsf(_fc*1000),
        _SNRdB < 0 ? 'm' : 'p', fabsf(_SNRdB*1000));
    liquid_autotest_validate_spectrum(psd, _nfft, regions, 3,
        liquid_autotest_verbose ? filename : NULL);

    // destroy objects
    spgramcf_destroy(q);
    symstreamrcf_destroy(gen);
    nco_crcf_destroy(mixer);
}

void autotest_spgramcf_signal_00() { testbench_spgramcf_signal(800,LIQUID_WINDOW_HAMMING, 0.0f,30.0f); }
void autotest_spgramcf_signal_01() { testbench_spgramcf_signal(800,LIQUID_WINDOW_HAMMING, 0.2f,10.0f); }
void autotest_spgramcf_signal_02() { testbench_spgramcf_signal(800,LIQUID_WINDOW_HANN,    0.2f,10.0f); }
void autotest_spgramcf_signal_03() { testbench_spgramcf_signal(400,LIQUID_WINDOW_KAISER, -0.3f,40.0f); }
void autotest_spgramcf_signal_04() { testbench_spgramcf_signal(640,LIQUID_WINDOW_HAMMING,-0.2f, 0.0f); }
void autotest_spgramcf_signal_05() { testbench_spgramcf_signal(640,LIQUID_WINDOW_HAMMING, 0.1f,-3.0f); }

void autotest_spgramcf_counters()
{
    // create spectral periodogram with specific parameters
    unsigned int nfft=1200, wlen=400, delay=200;
    int wtype = LIQUID_WINDOW_HAMMING;
    float alpha = 0.0123456f;
    spgramcf q = spgramcf_create(nfft, wtype, wlen, delay);

    // check setting bandwidth
    CONTEND_EQUALITY ( spgramcf_set_alpha(q, 0.1),  0 ); // valid
    CONTEND_DELTA    ( spgramcf_get_alpha(q), 0.1, 1e-6f);
    CONTEND_EQUALITY ( spgramcf_set_alpha(q,-7.0), -1 ); // invalid
    CONTEND_DELTA    ( spgramcf_get_alpha(q), 0.1, 1e-6f);
    CONTEND_EQUALITY ( spgramcf_set_alpha(q,alpha),  0); // valid
    CONTEND_DELTA    ( spgramcf_get_alpha(q), alpha, 1e-6f);
    spgramcf_print(q); // test for code coverage

    // check parameters
    CONTEND_EQUALITY( spgramcf_get_nfft(q),       nfft );
    CONTEND_EQUALITY( spgramcf_get_window_len(q), wlen );
    CONTEND_EQUALITY( spgramcf_get_delay(q),      delay);
    CONTEND_EQUALITY( spgramcf_get_alpha(q),      alpha);

    unsigned int block_len = 1117, num_blocks = 1123;
    unsigned int i, num_samples = block_len * num_blocks;
    unsigned int num_transforms = num_samples / delay;
    for (i=0; i<num_samples; i++)
        spgramcf_push(q, randnf() + _Complex_I*randnf());

    // verify number of samples and transforms processed
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),          num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q),    num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_transforms(q),       num_transforms);
    CONTEND_EQUALITY(spgramcf_get_num_transforms_total(q), num_transforms);

    // clear object and run in blocks
    spgramcf_clear(q);
    float complex block[block_len];
    for (i=0; i<block_len; i++)
        block[i] = randnf() + _Complex_I*randnf();
    for (i=0; i<num_blocks; i++)
        spgramcf_write(q, block, block_len);

    // re-verify number of samples and transforms processed
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),          num_samples);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q),    num_samples * 2);
    CONTEND_EQUALITY(spgramcf_get_num_transforms(q),       num_transforms);
    CONTEND_EQUALITY(spgramcf_get_num_transforms_total(q), num_transforms * 2);

    // reset object and ensure counters are zero
    spgramcf_reset(q);
    CONTEND_EQUALITY(spgramcf_get_num_samples(q),          0);
    CONTEND_EQUALITY(spgramcf_get_num_samples_total(q),    0);
    CONTEND_EQUALITY(spgramcf_get_num_transforms(q),       0);
    CONTEND_EQUALITY(spgramcf_get_num_transforms_total(q), 0);

    // destroy object(s)
    spgramcf_destroy(q);
}

void autotest_spgramcf_invalid_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping spgram config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check that object returns NULL for invalid configurations
    CONTEND_ISNULL(spgramcf_create(  0, LIQUID_WINDOW_HAMMING,       200, 200)); // nfft too small
    CONTEND_ISNULL(spgramcf_create(  1, LIQUID_WINDOW_HAMMING,       200, 200)); // nfft too small
    CONTEND_ISNULL(spgramcf_create(  2, LIQUID_WINDOW_HAMMING,       200, 200)); // window length too large
    CONTEND_ISNULL(spgramcf_create(400, LIQUID_WINDOW_HAMMING,         0, 200)); // window length too small
    CONTEND_ISNULL(spgramcf_create(400, LIQUID_WINDOW_UNKNOWN,       200, 200)); // invalid window type
    CONTEND_ISNULL(spgramcf_create(400, LIQUID_WINDOW_NUM_FUNCTIONS, 200, 200)); // invalid window type
    CONTEND_ISNULL(spgramcf_create(400, LIQUID_WINDOW_KBD,           201, 200)); // KBD must be even
    CONTEND_ISNULL(spgramcf_create(400, LIQUID_WINDOW_HAMMING,       200,   0)); // delay too small

    // check that object returns NULL for invalid configurations (default)
    CONTEND_ISNULL(spgramcf_create_default(0)); // nfft too small
    CONTEND_ISNULL(spgramcf_create_default(1)); // nfft too small

    // create proper object but test invalid internal configurations
    spgramcf q = spgramcf_create_default(540);

    CONTEND_INEQUALITY(LIQUID_OK, spgramcf_set_rate(q, -10e6))

    spgramcf_destroy(q);
}

void autotest_spgramcf_standalone()
{
    unsigned int nfft        = 1200;
    unsigned int num_samples = 20*nfft;  // number of samples to generate
    float        noise_floor = -20.0f;
    float        nstd        = powf(10.0f,noise_floor/20.0f); // noise std. dev.

    float complex * buf = (float complex*)malloc(num_samples*sizeof(float complex));
    unsigned int i;
    for (i=0; i<num_samples; i++)
        buf[i] = 0.1f + nstd*(randnf()+_Complex_I*randnf())*M_SQRT1_2;

    float psd[nfft];
    spgramcf_estimate_psd(nfft, buf, num_samples, psd);

    // check mask
    for (i=0; i<nfft; i++) {
        float mask_lo = i ==nfft/2                     ? 2.0f : noise_floor - 3.0f;
        float mask_hi = i > nfft/2-10 && i < nfft/2+10 ? 8.0f : noise_floor + 3.0f;
        if (liquid_autotest_verbose)
            printf("%6u : %8.2f < %8.2f < %8.2f\n", i, mask_lo, psd[i], mask_hi);
        CONTEND_GREATER_THAN( psd[i], mask_lo );
        CONTEND_LESS_THAN   ( psd[i], mask_hi );
    }

    // free memory
    free(buf);
}

// check spectral periodogram operation where the input size is much shorter
// than the transform size
void autotest_spgramcf_short()
{
    unsigned int nfft        = 1200;    // transform size
    unsigned int num_samples =  200;    // number of samples to generate
    float        noise_floor = -20.0f;
    float        nstd        = powf(10.0f,noise_floor/20.0f); // noise std. dev.

    float complex * buf = (float complex*)malloc(num_samples*sizeof(float complex));
    unsigned int i;
    for (i=0; i<num_samples; i++)
        buf[i] = 1.0f + nstd*(randnf()+_Complex_I*randnf())*M_SQRT1_2;

    float psd[nfft];
    spgramcf_estimate_psd(nfft, buf, num_samples, psd);

    // use a very loose upper mask as we have only computed a few hundred samples
    for (i=0; i<nfft; i++) {
        float f       = (float)i / (float)nfft - 0.5f;
        float mask_hi = fabsf(f) < 0.2f ? 15.0f - 30*fabsf(f)/0.2f : -15.0f;
        if (liquid_autotest_verbose)
            printf("%6u : f=%6.3f, %8.2f < %8.2f\n", i, f, psd[i], mask_hi);
        CONTEND_LESS_THAN( psd[i], mask_hi );
    }
    // consider lower mask only for DC term
    float mask_lo = 0.0f;
    unsigned int nfft_2 = nfft/2;
    if (liquid_autotest_verbose)
        printf("    DC : f=%6.3f, %8.2f > %8.2f\n", 0.0f, psd[nfft_2], mask_lo);
    CONTEND_GREATER_THAN( psd[nfft_2], mask_lo );

    // free memory
    free(buf);
}

// check copy method
void autotest_spgramcf_copy()
{
    unsigned int nfft        = 1200;    // transform size
    unsigned int num_samples = 9600;    // number of samples to generate
    float        nstd        =  0.1f;   // noise standard deviation

    // create object with some odd properties
    spgramcf q0 = spgramcf_create(nfft, LIQUID_WINDOW_KAISER, 960, 373);

    // generate a bunch of random noise samples
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        float complex v = 0.1f + nstd * (randnf() + _Complex_I*randnf());
        spgramcf_push(q0, v);
    }

    // copy object and push same samples through both
    spgramcf q1 = spgramcf_copy(q0);
    for (i=0; i<num_samples; i++) {
        float complex v = 0.1f + nstd * (randnf() + _Complex_I*randnf());
        spgramcf_push(q0, v);
        spgramcf_push(q1, v);
    }

    // get spectrum and compare outputs
    float psd_0[nfft];
    float psd_1[nfft];
    spgramcf_get_psd(q0, psd_0);
    spgramcf_get_psd(q1, psd_1);
    CONTEND_SAME_DATA(psd_0, psd_1, nfft*sizeof(float));

    // check parameters
    CONTEND_EQUALITY(spgramcf_get_nfft                (q0),spgramcf_get_nfft                (q1));
    CONTEND_EQUALITY(spgramcf_get_window_len          (q0),spgramcf_get_window_len          (q1));
    CONTEND_EQUALITY(spgramcf_get_delay               (q0),spgramcf_get_delay               (q1));
    CONTEND_EQUALITY(spgramcf_get_wtype               (q0),spgramcf_get_wtype               (q1));
    CONTEND_EQUALITY(spgramcf_get_num_samples         (q0),spgramcf_get_num_samples         (q1));
    CONTEND_EQUALITY(spgramcf_get_num_samples_total   (q0),spgramcf_get_num_samples_total   (q1));
    CONTEND_EQUALITY(spgramcf_get_num_transforms      (q0),spgramcf_get_num_transforms      (q1));
    CONTEND_EQUALITY(spgramcf_get_num_transforms_total(q0),spgramcf_get_num_transforms_total(q1));

    // destroy objects
    spgramcf_destroy(q0);
    spgramcf_destroy(q1);
}

// check spectral periodogram behavior on null input (zero samples)
void autotest_spgramcf_null()
{
    unsigned int nfft = 1200;   // transform size
    float psd[nfft];
    spgramcf_estimate_psd(nfft, NULL, 0, psd);

    // value should be exactly minimum
    float psd_val = 10*log10f(LIQUID_SPGRAM_PSD_MIN);
    unsigned int i;
    for (i=0; i<nfft; i++)
        CONTEND_EQUALITY(psd[i], psd_val);
}

// test file export
void autotest_spgram_gnuplot()
{
    // create default object
    spgramcf q = spgramcf_create_default(540);
    unsigned int i;
    for (i=0; i<20000; i++)
        spgramcf_push(q, randnf() + _Complex_I*randnf());

    // export once before setting values
    CONTEND_EQUALITY(LIQUID_OK,spgramcf_export_gnuplot(q,"autotest/logs/spgram.gnu"))

    // set values and export again
    CONTEND_EQUALITY(LIQUID_OK,spgramcf_set_freq(q, 100e6))
    CONTEND_EQUALITY(LIQUID_OK,spgramcf_set_rate(q,  20e6))
    CONTEND_EQUALITY(LIQUID_OK,spgramcf_export_gnuplot(q,"autotest/logs/spgram.gnu"))

    spgramcf_destroy(q);
}

