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

#include "autotest/autotest.h"
#include "liquid.h"

// generic window testbench
void liquid_window_testbench(int _wtype, unsigned int _n, float _arg)
{
    float w[_n];
    float wsum = 0.0f;
    unsigned int i;
    for (i=0; i<_n; i++) {
        w[i] = liquid_windowf(_wtype, i, _n, _arg);
        wsum += w[i];
    }

    // compute spectral response
    unsigned int nfft = 1200;
    float complex buf_time[nfft];
    float complex buf_freq[nfft];
    for (i=0; i<nfft; i++)
        buf_time[i] = i < _n ? w[i]/wsum : 0.0f;
    fft_run(nfft, buf_time, buf_freq, LIQUID_FFT_FORWARD, 0);
    fft_shift(buf_freq, nfft);

    // compute bandwidth of window, ensure reasonable range
    for (i=0; i<nfft; i++) {
        unsigned int r = i % 2;
        unsigned int L = (i-r)/2;
        unsigned int k = nfft/2 + (r==0 ? -L : L+r);
        if (20*log10f(cabsf(buf_freq[k])) < -6.0f)
            break;
    }
    float bw = (float)i / (float)nfft;
    if (liquid_autotest_verbose) printf("  bw: %12.8f\n", bw);
    CONTEND_GREATER_THAN(bw, 0.02f);
    CONTEND_LESS_THAN   (bw, 0.08f);

    // check side lobes at band edges
    for (i=0; i<nfft; i++) {
        float f = (float)i/(float)nfft - 0.5f;
        if (fabsf(f) > 0.20f)
            CONTEND_LESS_THAN(20*log10f(cabsf(buf_freq[i])), -40.0f);
    }
#if 0
    // DEBUG: export to file for testing
    FILE * fid = fopen("window_autotest.m","w");
    fprintf(fid,"clear all; close all; nfft=%u; n=%u;\n", nfft, _n);
    fprintf(fid,"w=zeros(n,1); W=zeros(nfft,1);\n");
    for (i=0; i<_n; i++)
        fprintf(fid,"w(%u) = %12.8f;\n", i+1, w[i]);
    for (i=0; i<nfft; i++)
        fprintf(fid,"W(%u) = %12.8f;\n", i+1, 20*log10(cabsf(buf_freq[i])));
    fprintf(fid,"plot([0:(nfft-1)]/nfft-0.5,W); grid on; axis([-0.5 0.5 -120 10]);\n");
    fprintf(fid,"title('%s');\n", liquid_window_str[_wtype][1]);
    fclose(fid);
#endif

}

void autotest_window_hamming        () { liquid_window_testbench(LIQUID_WINDOW_HAMMING,         71,  0.f); }
void autotest_window_hann           () { liquid_window_testbench(LIQUID_WINDOW_HANN,            71,  0.f); }
void autotest_window_blackmanharris () { liquid_window_testbench(LIQUID_WINDOW_BLACKMANHARRIS,  71,  0.f); }
void autotest_window_blackmanharris7() { liquid_window_testbench(LIQUID_WINDOW_BLACKMANHARRIS7, 71,  0.f); }
void autotest_window_kaiser         () { liquid_window_testbench(LIQUID_WINDOW_KAISER,          71, 10.f); }
void autotest_window_flattop        () { liquid_window_testbench(LIQUID_WINDOW_FLATTOP,         71,  0.f); }
void autotest_window_triangular     () { liquid_window_testbench(LIQUID_WINDOW_TRIANGULAR,      71, 71.f); }
void autotest_window_rcostaper      () { liquid_window_testbench(LIQUID_WINDOW_RCOSTAPER,       71, 25.f); }
void autotest_window_kbd            () { liquid_window_testbench(LIQUID_WINDOW_KBD,             72,  0.f); }

// Kaiser-Bessel derived window
void liquid_kbd_window_test(unsigned int _n, float _beta)
{
    unsigned int i;
    float tol = 1e-3f;

    // compute window
    float w[_n];
    liquid_kbd_window(_n,_beta,w);

    // square window
    float w2[_n];
    for (i=0; i<_n; i++)
        w2[i] = w[i]*w[i];

    // ensure w[i]^2 + w[i+M]^2 == 1
    unsigned int M = _n/2;
    for (i=0; i<M; i++)
        CONTEND_DELTA(w2[i]+w2[(i+M)%_n], 1.0f, tol);

    // ensure sum(w[i]^2) == _n/2
    float sum=0.0f;
    for (i=0; i<_n; i++)
        sum += w2[i];
    CONTEND_DELTA(sum, 0.5f*_n, tol);
}

void autotest_kbd_n16() { liquid_kbd_window_test(16, 10.0f); }
void autotest_kbd_n32() { liquid_kbd_window_test(32, 20.0f); }
void autotest_kbd_n48() { liquid_kbd_window_test(48, 12.0f); }

// test window configuration and error handling
void autotest_window_config()
{
    //_liquid_error_downgrade_enable();
    CONTEND_EQUALITY(liquid_print_windows(), LIQUID_OK);

    // check normal cases
    CONTEND_EQUALITY(liquid_getopt_str2window("unknown"        ), LIQUID_WINDOW_UNKNOWN); // <- no error
    CONTEND_EQUALITY(liquid_getopt_str2window("hamming"        ), LIQUID_WINDOW_HAMMING);
    CONTEND_EQUALITY(liquid_getopt_str2window("hann"           ), LIQUID_WINDOW_HANN);
    CONTEND_EQUALITY(liquid_getopt_str2window("blackmanharris" ), LIQUID_WINDOW_BLACKMANHARRIS);
    CONTEND_EQUALITY(liquid_getopt_str2window("blackmanharris7"), LIQUID_WINDOW_BLACKMANHARRIS7);
    CONTEND_EQUALITY(liquid_getopt_str2window("kaiser"         ), LIQUID_WINDOW_KAISER);
    CONTEND_EQUALITY(liquid_getopt_str2window("flattop"        ), LIQUID_WINDOW_FLATTOP);
    CONTEND_EQUALITY(liquid_getopt_str2window("triangular"     ), LIQUID_WINDOW_TRIANGULAR);
    CONTEND_EQUALITY(liquid_getopt_str2window("rcostaper"      ), LIQUID_WINDOW_RCOSTAPER);
    CONTEND_EQUALITY(liquid_getopt_str2window("kbd"            ), LIQUID_WINDOW_KBD);
    // check invalid cases
    CONTEND_EQUALITY(liquid_getopt_str2window("invalid window" ), LIQUID_WINDOW_UNKNOWN);

    // invalid KBD window parameters
    CONTEND_EQUALITY(liquid_kbd(12, 10, 10.0f), 0.0f); // index exceeds maximum
    CONTEND_EQUALITY(liquid_kbd( 0,  0, 10.0f), 0.0f); // window length is zero
    CONTEND_EQUALITY(liquid_kbd(12, 27, 10.0f), 0.0f); // window length is odd

    float w[20];
    CONTEND_INEQUALITY(liquid_kbd_window( 0, 10.0f, w), LIQUID_OK); // length is zero
    CONTEND_INEQUALITY(liquid_kbd_window( 7, 10.0f, w), LIQUID_OK); // length is odd
    CONTEND_INEQUALITY(liquid_kbd_window(20, -1.0f, w), LIQUID_OK); // beta value is negative

    // invalid Kaiser window parameters
    CONTEND_EQUALITY(liquid_kaiser(12, 10, 10.0f), 0.0f); // index exceeds maximum
    CONTEND_EQUALITY(liquid_kaiser(12, 20, -1.0f), 0.0f); // beta value is negative

    // Hamming
    CONTEND_EQUALITY(liquid_hamming(12, 10), 0.0f); // index exceeds maximum

    // Hann
    CONTEND_EQUALITY(liquid_hann(12, 10), 0.0f); // index exceeds maximum

    // Blackman-harris
    CONTEND_EQUALITY(liquid_blackmanharris(12, 10), 0.0f); // index exceeds maximum

    // Blackman-harris 7
    CONTEND_EQUALITY(liquid_blackmanharris7(12, 10), 0.0f); // index exceeds maximum

    // flat-top
    CONTEND_EQUALITY(liquid_flattop(12, 10), 0.0f); // index exceeds maximum

    // triangular
    CONTEND_EQUALITY(liquid_triangular(12, 10, 10), 0.0f); // index exceeds maximum
    CONTEND_EQUALITY(liquid_triangular( 7, 10, 15), 0.0f); // sub-length is out of range
    CONTEND_EQUALITY(liquid_triangular( 1,  1,  0), 0.0f); // sub-length is zero

    // raised-cosine taper
    CONTEND_EQUALITY(liquid_rcostaper_window(12, 10,  4), 0.0f); // index exceeds maximum
    CONTEND_EQUALITY(liquid_rcostaper_window( 7, 10,  8), 0.0f); // taper length exceeds maximum

    //_liquid_error_downgrade_disable();
}

