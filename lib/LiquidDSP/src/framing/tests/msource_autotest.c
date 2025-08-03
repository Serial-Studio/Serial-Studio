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
#include "liquid.internal.h"

// user-defined callback; generate tones
int callback_msourcecf_autotest(void *          _userdata,
                                float complex * _v,
                                unsigned int    _n)
{
    unsigned int * counter = (unsigned int*)_userdata;
    unsigned int i;
    for (i=0; i<_n; i++) {
        _v[i] = *counter==0 ? 1 : 0;
        *counter = (*counter+1) % 8;
    }
    return 0;
}

// test tone source
void autotest_msourcecf_tone()
{
    // spectral periodogram options
    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_samples = 192000;  // number of samples

    // create spectral periodogram
    spgramcf periodogram = spgramcf_create_default(nfft);

    // create multi-signal source generator and add signal(s)
    msourcecf gen = msourcecf_create_default();
    // add signals     (gen,  fc,   bw,    gain
    msourcecf_add_noise(gen,  0.0f, 1.00f, -40);    // wide-band noise
    msourcecf_add_tone (gen, -0.4f, 0.00f,  20);    // tone
    msourcecf_add_tone (gen, -0.3f, 0.00f,  10);    // tone
    msourcecf_add_tone (gen, -0.2f, 0.00f,   0);    // tone
    msourcecf_add_tone (gen, -0.1f, 0.00f, -10);    // tone
    msourcecf_add_tone (gen,  0.0f, 0.00f, -20);    // tone
    msourcecf_add_tone (gen,  0.1f, 0.00f, -30);    // tone

    // generate samples
    unsigned int buf_len = 1024;
    float complex buf[buf_len];
    while (msourcecf_get_num_samples(gen) < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(periodogram, buf, buf_len);
    }

    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    msourcecf_destroy(gen);
    spgramcf_destroy(periodogram);

    // verify spectrum
    autotest_psd_s regions[] = {
      // noise floor between signals
      {.fmin=-0.500,.fmax=-0.405, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.395,.fmax=-0.305, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.295,.fmax=-0.205, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.195,.fmax=-0.105, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.095,.fmax=-0.005, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.005,.fmax= 0.095, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.105,.fmax= 0.500, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      // tone
      {.fmin=-0.401,.fmax=-0.399, .pmin= 10.0, .pmax= 22.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.301,.fmax=-0.299, .pmin=  0.0, .pmax= 12.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.201,.fmax=-0.199, .pmin=-10.0, .pmax=  2.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.101,.fmax=-0.099, .pmin=-20.0, .pmax= -8.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.001,.fmax= 0.001, .pmin=-30.0, .pmax=-18.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.099,.fmax= 0.101, .pmin=-40.0, .pmax=-28.0, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_spectrum(psd, nfft, regions, 13,
        liquid_autotest_verbose ? "autotest/logs/msourcecf_tone_autotest.m" : NULL);
}

// test chirp source
void autotest_msourcecf_chirp()
{
    // spectral periodogram options
    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_samples = 192000;  // number of samples

    // create spectral periodogram
    spgramcf periodogram = spgramcf_create_default(nfft);

    // create multi-signal source generator and add signal(s)
    msourcecf gen = msourcecf_create_default();
    // add signals     (gen,  fc,   bw,    gain
    msourcecf_add_noise(gen,  0.0f, 1.00f, -40); // wide-band noise
    msourcecf_add_chirp(gen,  0.0f, 0.60f,  20, num_samples*0.9, 0, 1);

    // generate samples
    unsigned int buf_len = 1024;
    float complex buf[buf_len];
    while (msourcecf_get_num_samples(gen) < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(periodogram, buf, buf_len);
    }

    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    msourcecf_destroy(gen);
    spgramcf_destroy(periodogram);

    // verify spectrum
    autotest_psd_s regions[] = {
      // noise floor between signals
      {.fmin=-0.500,.fmax=-0.305, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.295,.fmax= 0.295, .pmin= 15.0, .pmax= 22.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.305,.fmax= 0.500, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
    };
    liquid_autotest_validate_spectrum(psd, nfft, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/msourcecf_chirp_autotest.m" : NULL);
}

// test signals in aggregate
void autotest_msourcecf_aggregate()
{
    // msource parameters
    int          ms     = LIQUID_MODEM_QPSK;    // linear modulation scheme
    unsigned int m      =    12;                // modulation filter semi-length
    float        beta   = 0.30f;                // modulation filter excess bandwidth factor
    float        bt     = 0.35f;                // GMSK filter bandwidth-time factor

    // spectral periodogram options
    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_samples = 192000;  // number of samples

    // create spectral periodogram
    spgramcf periodogram = spgramcf_create_default(nfft);

    unsigned int buf_len = 1024;
    float complex buf[buf_len];

    // create multi-signal source generator
    msourcecf gen = msourcecf_create_default();

    // add signals     (gen,  fc,   bw,    gain, {options})
    msourcecf_add_noise(gen,  0.00f, 1.00f, -40);               // wide-band noise
    msourcecf_add_tone (gen, -0.45f, 0.00f,  20);               // tone
    msourcecf_add_fsk  (gen, -0.33f, 0.05f, -10, 3, 16);        // FSK
    msourcecf_add_gmsk (gen, -0.20f, 0.05f,   0, m, bt);        // modulated data (GMSK)
    msourcecf_add_noise(gen, -0.05f, 0.10f,   0);               // narrow-band noise
    msourcecf_add_chirp(gen,  0.07f, 0.07f,  20, 8000, 0, 0);   // chirp
    msourcecf_add_modem(gen,  0.20f, 0.10f,   0, ms, m, beta);  // modulated data (linear)
    unsigned int counter = 0;
    msourcecf_add_user (gen,  0.40f, 0.15f, -10, (void*)&counter, callback_msourcecf_autotest); // tones

    // print source generator object
    CONTEND_EQUALITY(LIQUID_OK, msourcecf_print(gen));

    unsigned int total_samples = 0;
    while (total_samples < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(periodogram, buf, buf_len);

        // accumulated samples
        total_samples += buf_len;
    }
    printf("total samples: %u\n", total_samples);

    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    msourcecf_destroy(gen);
    spgramcf_destroy(periodogram);

    // verify spectrum
    autotest_psd_s regions[] = {
      // noise floor between signals
      {.fmin=-0.500,.fmax=-0.455, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.445,.fmax=-0.385, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.275,.fmax=-0.260, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.140,.fmax=-0.110, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.005,.fmax=+0.030, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.110,.fmax=+0.130, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.270,.fmax=+0.320, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      // space between tones
      {.fmin=+0.328,.fmax=+0.338, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.348,.fmax=+0.358, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.368,.fmax=+0.378, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.386,.fmax=+0.396, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.405,.fmax=+0.415, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.424,.fmax=+0.434, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      {.fmin=+0.442,.fmax=+0.452, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      // end
      {.fmin=+0.461,.fmax=+0.500, .pmin=-43.0, .pmax=-37.0, .test_lo=1, .test_hi=1},
      // signals
      {.fmin=-0.451,.fmax=-0.449, .pmin=+10.0, .pmax=+22.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.355,.fmax=-0.305, .pmin=-15.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin=-0.220,.fmax=-0.180, .pmin=-18.0, .pmax= +6.5, .test_lo=1, .test_hi=1},
      {.fmin=-0.095,.fmax=-0.005, .pmin= -5.0, .pmax= +2.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.037,.fmax=+0.102, .pmin= 18.0, .pmax=+22.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.160,.fmax=+0.240, .pmin= -5.0, .pmax= +2.0, .test_lo=1, .test_hi=1},
      // tones
      {.fmin= 0.3245,.fmax=+0.3255, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.3432,.fmax=+0.3442, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.3620,.fmax=+0.3630, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.3810,.fmax=+0.3820, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.3995,.fmax=+0.4005, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.4182,.fmax=+0.4192, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.4370,.fmax=+0.4380, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.4555,.fmax=+0.4565, .pmin=-20.0, .pmax= +0.0, .test_lo=1, .test_hi=1},
    };
    char filename[256];
    sprintf(filename,"autotest/logs/msourcecf_aggregate_autotest.m");
    liquid_autotest_validate_spectrum(psd, nfft, regions, 7+7+1+6+8,
        liquid_autotest_verbose ? filename : NULL);
}

//
void autotest_msourcecf_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping msource config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // no need to check every combination
    CONTEND_ISNULL(msourcecf_create( 0, 12, 60));   // too few subcarriers
    CONTEND_ISNULL(msourcecf_create(17, 12, 60));   // odd-numbered subcarriers
    CONTEND_ISNULL(msourcecf_create(64,  0, 60));   // filter semi-length too small
    CONTEND_ISNULL(msourcecf_copy(NULL));

    // create proper object and test configurations
    msourcecf q = msourcecf_create(64, 12, 60);

    // try to configure signals with invalid IDs
    float rv;
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_remove       (q, 12345));
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_enable       (q, 12345));
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_disable      (q, 12345));
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_set_gain     (q, 12345, 0.0f));
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_get_gain     (q, 12345, &rv));
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_set_frequency(q, 12345, 0.0f));
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_get_frequency(q, 12345, &rv));

    // add signals and check setting values appropriately
    int id_tone = msourcecf_add_tone (q, -0.123456f, 0.00f, 20);
    int id_gmsk = msourcecf_add_gmsk (q,  0.220780f, 0.05f,  0, 4, 0.3f);

    CONTEND_EQUALITY(LIQUID_OK, msourcecf_print(q));

    // remove tone
    CONTEND_EQUALITY  (LIQUID_OK, msourcecf_remove  (q, id_tone));
    CONTEND_INEQUALITY(LIQUID_OK, msourcecf_set_gain(q, id_tone, 10.0f));

    // disable GMSK signal
    CONTEND_EQUALITY(LIQUID_OK, msourcecf_disable(q, id_gmsk));

    // assert buffer is zeros (only GMSK signal present and it's disabled)
    unsigned int buf_len = 1024;
    float complex buf[buf_len];
    msourcecf_write_samples(q, buf, buf_len);
    CONTEND_EQUALITY(0.0f, liquid_sumsqcf(buf, buf_len));

    // enable GMSK signal
    CONTEND_EQUALITY(LIQUID_OK, msourcecf_enable(q, id_gmsk));
    msourcecf_write_samples(q, buf, buf_len);
    CONTEND_GREATER_THAN(liquid_sumsqcf(buf, buf_len), 0.0f);

    // destroy object
    msourcecf_destroy(q);
}

// test accessor methods
void autotest_msourcecf_accessor()
{
    // create object and add signals:(q,  fc,        bw,    gain
    msourcecf q = msourcecf_create(240, 12, 60);
    int id_tone = msourcecf_add_tone (q, -0.123456f, 0.00f, 20);
    int id_noise= msourcecf_add_noise(q,  0.220780f, 0.10f,  0);
    float rv;

    // check center frequency of tone
    msourcecf_get_frequency(q, id_tone, &rv);
    CONTEND_EQUALITY(rv, -0.123456f);

    // check center frequency of noise signal
    msourcecf_get_frequency(q, id_noise, &rv);
    CONTEND_EQUALITY(rv,  0.220780f);

    // check gain of tone
    msourcecf_get_gain(q, id_tone, &rv);
    CONTEND_EQUALITY(rv, 20.0f);

    // check gain of noise signal
    msourcecf_get_gain(q, id_noise, &rv);
    CONTEND_EQUALITY(rv,  0.0f);

    // remove tone
    msourcecf_remove(q, id_tone);

    // disable noise signal
    msourcecf_disable(q, id_noise);

    // set frequency of noise signal
    msourcecf_set_frequency(q, id_noise, 0.33333f);
    msourcecf_get_frequency(q, id_noise, &rv);
    CONTEND_EQUALITY(rv, 0.33333f);

    // set gain of noise signal
    msourcecf_set_gain(q, id_noise, 30.0f);
    msourcecf_get_gain(q, id_noise, &rv);
    CONTEND_EQUALITY(rv, 30.0f);

    // assert buffer is zeros
    unsigned int buf_len = 1024;
    float complex buf[buf_len];
    msourcecf_write_samples(q, buf, buf_len);
    CONTEND_EQUALITY(0.0f, liquid_sumsqcf(buf, buf_len));

    // enable noise signal and check power spectral density
    msourcecf_enable(q, id_noise);
    //msourcecf_write_samples(q, buf, buf_len);
    // create spectral periodogram
    unsigned int nfft =  2400;  // spectral periodogram FFT size
    spgramcf periodogram = spgramcf_create_default(nfft);
    while (msourcecf_get_num_samples(q) < 192000) {
        // write samples to buffer
        msourcecf_write_samples(q, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(periodogram, buf, buf_len);
    }
    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    msourcecf_destroy(q);
    spgramcf_destroy(periodogram);

    // verify spectrum
    autotest_psd_s regions[] = {
      // noise floor between signals
      {.fmin=-0.500,.fmax= 0.275, .pmin=-80.0, .pmax=-40.0, .test_lo=0, .test_hi=1},
      {.fmin= 0.285,.fmax= 0.375, .pmin= 28.0, .pmax= 32.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.385,.fmax= 0.500, .pmin=-80.0, .pmax=-40.0, .test_lo=0, .test_hi=1},
    };
    liquid_autotest_validate_spectrum(psd, nfft, regions, 3,
        liquid_autotest_verbose ? "autotest/logs/msourcecf_accessor_autotest.m" : NULL);
}

// test copying object and ensure output spectrum aligns
void autotest_msourcecf_copy()
{
    // test options
    float        tol    = 1.5f;                 // error tolerance

    // msource parameters
    int          ms     = LIQUID_MODEM_QPSK;    // linear modulation scheme
    unsigned int m      =    12;                // modulation filter semi-length
    float        beta   = 0.30f;                // modulation filter excess bandwidth factor
    float        bt     = 0.35f;                // GMSK filter bandwidth-time factor

    // spectral periodogram options
    unsigned int nfft        =   1200;  // spectral periodogram FFT size
    unsigned int num_samples = 720000;  // number of samples

    // create spectral periodogram
    spgramcf periodogram_orig = spgramcf_create_default(nfft);
    spgramcf periodogram_copy = spgramcf_create_default(nfft);

    unsigned int buf_len = 1024;
    float complex buf_orig[buf_len];
    float complex buf_copy[buf_len];

    // create multi-signal source generator
    msourcecf gen_orig = msourcecf_create_default();

    // add signals     (gen,  fc,   bw,    gain, {options})
    msourcecf_add_noise(gen_orig,  0.00f, 1.00f, -40);               // wide-band noise
    msourcecf_add_tone (gen_orig, -0.45f, 0.00f,  20);               // tone
    msourcecf_add_fsk  (gen_orig, -0.33f, 0.05f, -10, 3, 16);        // FSK
    msourcecf_add_gmsk (gen_orig, -0.20f, 0.05f,   0, m, bt);        // modulated data (GMSK)
    msourcecf_add_noise(gen_orig, -0.05f, 0.10f,   0);               // narrow-band noise
    msourcecf_add_chirp(gen_orig,  0.07f, 0.07f,  20, 8000, 0, 0);   // chirp
    msourcecf_add_modem(gen_orig,  0.20f, 0.10f,   0, ms, m, beta);  // modulated data (linear)
    //unsigned int counter = 0;
    //msourcecf_add_user (gen_orig,  0.40f, 0.15f, -10, (void*)&counter, callback_msourcecf_autotest); // tones

    // copy object
    msourcecf gen_copy = msourcecf_copy(gen_orig);

    unsigned int total_samples = 0;
    while (total_samples < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen_orig, buf_orig, buf_len);
        msourcecf_write_samples(gen_copy, buf_copy, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(periodogram_orig, buf_orig, buf_len);
        spgramcf_write(periodogram_copy, buf_copy, buf_len);

        // accumulated samples
        total_samples += buf_len;
    }
    printf("total samples: %u\n", total_samples);

    // compute power spectral density output
    float psd_orig[nfft];
    float psd_copy[nfft];
    spgramcf_get_psd(periodogram_orig, psd_orig);
    spgramcf_get_psd(periodogram_copy, psd_copy);

    // destroy objects
    msourcecf_destroy(gen_orig);
    msourcecf_destroy(gen_copy);
    spgramcf_destroy(periodogram_orig);
    spgramcf_destroy(periodogram_copy);

    // verify spectrum output
    // TODO: ideally output would be identical streams; however PRNGs are different
    //       so we can currently only check their statistical outputs
    unsigned int i;
    char filename[] = "autotest/logs/msourcecf_copy.m";
    FILE * fid = fopen(filename,"w");
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"nfft=%u; psd_orig=zeros(1,nfft); psd_copy=zeros(1,nfft);\n", nfft);
    for (i=0; i<nfft; i++) {
        CONTEND_DELTA(psd_orig[i], psd_copy[i], tol);
        fprintf(fid," psd_orig(%3u)=%12.4e; psd_copy(%3u)=%12.4e;\n",
                i+1, psd_orig[i], i+1, psd_copy[i]);
    }
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"subplot(2,1,1), plot(f,psd_copy,f,psd_orig); legend('orig','copy'); grid on;\n");
    fprintf(fid,"  axis([-0.5 0.5 -50 30]); xlabel('Normalized Frequency [f/F_s]'); ylabel('PSD [dB]');\n");
    fprintf(fid,"subplot(2,1,2), plot(f,psd_copy - psd_orig); grid on;\n");
    fprintf(fid,"  axis([-0.5 0.5 -1  1 ]); xlabel('Normalized Frequency [f/F_s]'); ylabel('Error [dB]');\n");
    fclose(fid);
    printf("results written to %s\n", filename);
}

