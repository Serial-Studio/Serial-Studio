/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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

// Help function to keep code base small
//  _kf     :   modulation factor
void freqmodem_test(float _kf)
{
    // options
    unsigned int num_samples = 1024;
    float tol = 5e-2f;

    unsigned int i;

    // create mod/demod objects
    freqmod mod = freqmod_create(_kf);  // modulator
    freqdem dem = freqdem_create(_kf);  // demodulator

    // allocate arrays
    float         m[num_samples];       // message signal
    float complex r[num_samples];       // received signal (complex baseband)
    float         y[num_samples];       // demodulator output

    // generate message signal (sum of sines)
    for (i=0; i<num_samples; i++) {
        m[i] = 0.3f*cosf(2*M_PI*0.013f*i + 0.0f) +
               0.2f*cosf(2*M_PI*0.021f*i + 0.4f) +
               0.4f*cosf(2*M_PI*0.037f*i + 1.7f);
    }

    // modulate signal
    freqmod_modulate_block(mod, m, num_samples, r);

    // demodulate signal
    freqdem_demodulate_block(dem, r, num_samples, y);

    // delete modem objects
    freqmod_destroy(mod);
    freqdem_destroy(dem);

    // compare demodulated signal to original, skipping first sample
    for (i=1; i<num_samples; i++)
        CONTEND_DELTA( y[i], m[i], tol );
}

// AUTOTESTS: generic PSK
void autotest_freqmodem_kf_0_02() { freqmodem_test(0.02f); }
void autotest_freqmodem_kf_0_04() { freqmodem_test(0.04f); }
void autotest_freqmodem_kf_0_08() { freqmodem_test(0.08f); }

