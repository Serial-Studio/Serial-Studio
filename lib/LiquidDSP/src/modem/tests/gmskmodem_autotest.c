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

// Help function to keep code base small
void gmskmodem_test_mod_demod(unsigned int _k,
                              unsigned int _m,
                              float        _bt)
{
    // create modulator/demodulator pair
    gmskmod mod = gmskmod_create(_k, _m, _bt);
    gmskdem dem = gmskdem_create(_k, _m, _bt);

    // derived values
    unsigned int delay = _m + _m;
    
    unsigned int  num_symbols = 80 + delay; // number of symbols to test

    msequence ms = msequence_create_default(7);

    float complex buf[_k];      // sample buffer
    unsigned int  sym_in [num_symbols]; // symbol buffer
    unsigned int  sym_out[num_symbols]; // symbol buffer

    // modulate, demodulate, count errors
    unsigned int i;
    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        sym_in[i] = msequence_generate_symbol(ms, 1);

        // modulate
        gmskmod_modulate(mod, sym_in[i], buf);

        // demodulate
        gmskdem_demodulate(dem, buf, sym_out + i);
    }

    // count errors
    for (i=0; i<num_symbols; i++) {
        if (i >= delay) {
#if 0
            // print results
            if (liquid_autotest_verbose) {
                printf("  %3u : input = %2u, output = %2u %s\n",
                        i, sym_in[i-delay], sym_out[i],
                        (sym_in[i-delay] == sym_out[i]) ? "" : "*");
            }
#endif

            // check result
            CONTEND_EQUALITY(sym_in[i-delay], sym_out[i]);
        }
    }

    // clean it up
    msequence_destroy(ms);
    gmskmod_destroy(mod);
    gmskdem_destroy(dem);
}

// base configuration
void autotest_gmskmodem_k4_m3_b025() { gmskmodem_test_mod_demod( 4, 3, 0.25f); }

// test different samples/symbol
void autotest_gmskmodem_k2_m3_b025() { gmskmodem_test_mod_demod( 2, 3, 0.25f); }
void autotest_gmskmodem_k3_m3_b025() { gmskmodem_test_mod_demod( 3, 3, 0.25f); }
void autotest_gmskmodem_k5_m3_b025() { gmskmodem_test_mod_demod( 5, 3, 0.25f); }
void autotest_gmskmodem_k8_m3_b033() { gmskmodem_test_mod_demod( 8, 3, 0.25f); }

// test different filter semi-lengths
void autotest_gmskmodem_k4_m1_b025() { gmskmodem_test_mod_demod( 4, 1, 0.25f); }
void autotest_gmskmodem_k4_m2_b025() { gmskmodem_test_mod_demod( 4, 2, 0.25f); }
void autotest_gmskmodem_k4_m8_b025() { gmskmodem_test_mod_demod( 4, 8, 0.25f); }

// test different filter bandwidth factors
void autotest_gmskmodem_k4_m3_b020() { gmskmodem_test_mod_demod( 4, 3, 0.20f); }
void autotest_gmskmodem_k4_m3_b033() { gmskmodem_test_mod_demod( 4, 3, 0.25f); }
void autotest_gmskmodem_k4_m3_b050() { gmskmodem_test_mod_demod( 4, 3, 0.25f); }

// test modulator copy
void autotest_gmskmod_copy()
{
    // options
    unsigned int k  = 5;
    unsigned int m  = 3;
    float        bt = 0.2345f;

    // create modulator/demodulator pair
    gmskmod mod_orig = gmskmod_create(k, m, bt);

    unsigned int num_symbols = 16;
    float complex buf_orig[k];
    float complex buf_copy[k];
    msequence ms = msequence_create_default(7);

    // run original object
    unsigned int i;
    for (i=0; i<num_symbols; i++) {
        // generate random symbol and modulate
        unsigned char s = msequence_generate_symbol(ms, 1);
        gmskmod_modulate(mod_orig, s, buf_orig);
    }

    // copy object
    gmskmod mod_copy = gmskmod_copy(mod_orig);

    // run through both objects and compare
    for (i=0; i<num_symbols; i++) {
        // generate random symbol and modulate
        unsigned char s = msequence_generate_symbol(ms, 1);
        gmskmod_modulate(mod_orig, s, buf_orig);
        gmskmod_modulate(mod_copy, s, buf_copy);
        // check result
        CONTEND_SAME_DATA(buf_orig, buf_copy, k*sizeof(float complex));
    }

    // clean it up
    msequence_destroy(ms);
    gmskmod_destroy(mod_orig);
    gmskmod_destroy(mod_copy);
}

// test demodulator copy
void autotest_gmskdem_copy()
{
    // options
    unsigned int k  = 5;
    unsigned int m  = 3;
    float        bt = 0.2345f;

    // create modulator/demodulator pair
    gmskdem dem_orig = gmskdem_create(k, m, bt);

    unsigned int num_symbols = 16;
    float complex buf[k];
    unsigned int  sym_orig;
    unsigned int  sym_copy;

    // run original object
    unsigned int i, j;
    for (i=0; i<num_symbols; i++) {
        // generate random signal and demodulate
        for (j=0; j<k; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        gmskdem_demodulate(dem_orig, buf, &sym_orig);
    }

    // copy object
    gmskdem dem_copy = gmskdem_copy(dem_orig);

    // run through both objects and compare
    for (i=0; i<num_symbols; i++) {
        // generate random signal and demodulate
        for (j=0; j<k; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        gmskdem_demodulate(dem_orig, buf, &sym_orig);
        gmskdem_demodulate(dem_copy, buf, &sym_copy);
        // check result
        CONTEND_EQUALITY(sym_orig, sym_copy);
    }

    // clean it up
    gmskdem_destroy(dem_orig);
    gmskdem_destroy(dem_copy);
}

