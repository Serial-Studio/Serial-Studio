/*
 * Copyright (c) 2007 - 2019 Joseph Gaeddert
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
void ampmodem_test_harness(float                _mod_index,
                           liquid_ampmodem_type _type,
                           int                  _suppressed_carrier,
                           float                _dphi,
                           float                _phi)
{
    // options
    float   SNRdB   = 40.0f;    // signal-to-noise ratio (set very high for testing)
    int     debug   = 0;        // enable debugging
    const char * filename_debug = "ampmodem_autotest_debug.dat";

    // derived values
    float nstd = powf(10.0f,-SNRdB/20.0f);

    // create mod/demod objects
    ampmodem mod   = ampmodem_create(_mod_index, _type, _suppressed_carrier);
    ampmodem demod = ampmodem_create(_mod_index, _type, _suppressed_carrier);
    if (liquid_autotest_verbose)
        ampmodem_print(mod);

    // compute end-to-end delay
    unsigned int delay = ampmodem_get_delay_mod(mod) + ampmodem_get_delay_demod(demod);
    wdelayf message_delay = wdelayf_create(delay);

    // run trials
    unsigned int i=0;
    unsigned int skip = 2400; // wait for PLL and filters to settle
    unsigned int num_samples_compare = 0;   // number of samples compared
    float rmse_0 = 0, rmse_1 = 0;           // RMS error in phase and 180 out of phase
    float f0 = 1.0f / sqrtf(1031.0f), f1 = 1.0f / sqrtf(1723.0f);
    FILE * fid = debug ? fopen(filename_debug,"w") : NULL;
    if (debug) {
        fprintf(fid,"# %12s %12s %12s %12s %12s %12s\n",
                "msg in", "x (real)", "x (imag)", "y (real)", "y (imag)", "msg out");
    }
    while (num_samples_compare < 8000) {
        // generate original message signal
        float msg_in = 0.6f*cos(2*M_PI*f0*i) + 0.4f*cos(2*M_PI*f1*i);
        wdelayf_push(message_delay, msg_in);

        // modulate
        float complex x;
        ampmodem_modulate(mod, msg_in, &x);

        // add channel impairments
        float complex y = x*cexpf(_Complex_I*_phi) +
            nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // update phase
        _phi += _dphi;
        while (_phi >  M_PI) _phi -= 2*M_PI;
        while (_phi < -M_PI) _phi += 2*M_PI;

        // demodulate signal
        float msg_out;
        ampmodem_demodulate(demod, y, &msg_out);

        // compute error
        wdelayf_read(message_delay, &msg_in);
        if (debug) {
            fprintf(fid,"  %12.4e %12.4e %12.4e %12.4e %12.4e %12.4e\n",
                    msg_in, crealf(x), cimagf(x), crealf(y), cimagf(y), msg_out);
        }
        if (i >= skip) {
            float e0 = msg_in - msg_out;
            float e1 = msg_in + msg_out;
            rmse_0 += e0*e0;
            rmse_1 += e1*e1;
            num_samples_compare++;
        }
        i++;
    }
    if (debug) fclose(fid);

    // destroy objects
    ampmodem_destroy(mod);
    ampmodem_destroy(demod);
    wdelayf_destroy (message_delay);

    // finally, check if test passed based on modulation type; for
    // double side-band suppressed carrier, we can have a 180 degree phase offset
    rmse_0 = 10*log10f( rmse_0 / (float)num_samples_compare );  // in-phase
    rmse_1 = 10*log10f( rmse_1 / (float)num_samples_compare );  // 180-degree out of phase
    float rmse = (_type == LIQUID_AMPMODEM_DSB && _suppressed_carrier ) ? (rmse_0 < rmse_1 ? rmse_0 : rmse_1) : rmse_0;
    if (liquid_autotest_verbose)
        printf("rms error : %.3f (in-phase: %.3f, 180 phase: %.3f) dB\n", rmse, rmse_0, rmse_1);
    CONTEND_LESS_THAN( rmse, -18.0f );
}

// AUTOTESTS: basic properties: M=2^m, k = 2*M, bandwidth = 0.25
void autotest_ampmodem_dsb_carrier_on () { ampmodem_test_harness(0.8f,LIQUID_AMPMODEM_DSB,0,0.02,0.0); }
void autotest_ampmodem_usb_carrier_on () { ampmodem_test_harness(0.8f,LIQUID_AMPMODEM_USB,0,0.02,0.0); }
void autotest_ampmodem_lsb_carrier_on () { ampmodem_test_harness(0.8f,LIQUID_AMPMODEM_LSB,0,0.02,0.0); }

void autotest_ampmodem_dsb_carrier_off() { ampmodem_test_harness(0.8f,LIQUID_AMPMODEM_DSB,1,0.02,0.0); }
void autotest_ampmodem_usb_carrier_off() { ampmodem_test_harness(0.8f,LIQUID_AMPMODEM_USB,1,0.00,0.0); }
void autotest_ampmodem_lsb_carrier_off() { ampmodem_test_harness(0.8f,LIQUID_AMPMODEM_LSB,1,0.00,0.0); }


