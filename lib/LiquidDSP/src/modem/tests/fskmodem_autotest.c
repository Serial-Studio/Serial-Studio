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
void fskmodem_test_mod_demod(unsigned int _m,
                             unsigned int _k,
                             float        _bandwidth)
{
    if (liquid_autotest_verbose)
        printf("fskmodem_test_mod_demod(m=%u, k=%u, bandwidth=%g)\n", _m, _k, _bandwidth);

    // create modulator/demodulator pair
    fskmod mod = fskmod_create(_m,_k,_bandwidth);
    fskdem dem = fskdem_create(_m,_k,_bandwidth);

    unsigned int M = 1 << _m;   // constellation size
    float complex buf[_k];      // transmit buffer
    
    // modulate, demodulate, count errors
    unsigned int i;
    for (i=0; i<M; i++) {
        // generate random symbol
        unsigned int sym_in = i;

        // modulate
        fskmod_modulate(mod, sym_in, buf);

        // demodulate
        unsigned int sym_out = fskdem_demodulate(dem, buf);

        // count errors
        CONTEND_EQUALITY(sym_in, sym_out);
    }

    // clean it up
    fskmod_destroy(mod);
    fskdem_destroy(dem);
}

// AUTOTESTS: basic properties: M=2^m, k = 2*M, bandwidth = 0.25
void autotest_fskmodem_norm_M2()    { fskmodem_test_mod_demod( 1,    4, 0.25f    ); }
void autotest_fskmodem_norm_M4()    { fskmodem_test_mod_demod( 2,    8, 0.25f    ); }
void autotest_fskmodem_norm_M8()    { fskmodem_test_mod_demod( 3,   16, 0.25f    ); }
void autotest_fskmodem_norm_M16()   { fskmodem_test_mod_demod( 4,   32, 0.25f    ); }
void autotest_fskmodem_norm_M32()   { fskmodem_test_mod_demod( 5,   64, 0.25f    ); }
void autotest_fskmodem_norm_M64()   { fskmodem_test_mod_demod( 6,  128, 0.25f    ); }
void autotest_fskmodem_norm_M128()  { fskmodem_test_mod_demod( 7,  256, 0.25f    ); }
void autotest_fskmodem_norm_M256()  { fskmodem_test_mod_demod( 8,  512, 0.25f    ); }
void autotest_fskmodem_norm_M512()  { fskmodem_test_mod_demod( 9, 1024, 0.25f    ); }
void autotest_fskmodem_norm_M1024() { fskmodem_test_mod_demod(10, 2048, 0.25f    ); }

// AUTOTESTS: obscure properties: M=2^m, k not relative to M, bandwidth basically irrational
void autotest_fskmodem_misc_M2()    { fskmodem_test_mod_demod( 1,    5, 0.3721451); }
void autotest_fskmodem_misc_M4()    { fskmodem_test_mod_demod( 2,   10, 0.3721451); }
void autotest_fskmodem_misc_M8()    { fskmodem_test_mod_demod( 3,   20, 0.3721451); }
void autotest_fskmodem_misc_M16()   { fskmodem_test_mod_demod( 4,   30, 0.3721451); }
void autotest_fskmodem_misc_M32()   { fskmodem_test_mod_demod( 5,   60, 0.3721451); }
void autotest_fskmodem_misc_M64()   { fskmodem_test_mod_demod( 6,  100, 0.3721451); }
void autotest_fskmodem_misc_M128()  { fskmodem_test_mod_demod( 7,  200, 0.3721451); }
void autotest_fskmodem_misc_M256()  { fskmodem_test_mod_demod( 8,  500, 0.3721451); }
void autotest_fskmodem_misc_M512()  { fskmodem_test_mod_demod( 9, 1000, 0.3721451); }
void autotest_fskmodem_misc_M1024() { fskmodem_test_mod_demod(10, 2000, 0.3721451); }

// test modulator copy
void autotest_fskmod_copy()
{
    // options
    unsigned int m  = 3;        // bits per symbol
    unsigned int k  = 200;      // samples per symbol
    float        bw = 0.2345f;  // occupied bandwidth

    // create modulator/demodulator pair
    fskmod mod_orig = fskmod_create(m,k,bw);

    unsigned int num_symbols = 96;
    float complex buf_orig[k];
    float complex buf_copy[k];
    msequence ms = msequence_create_default(7);

    // run original object
    unsigned int i;
    for (i=0; i<num_symbols; i++) {
        // generate random symbol and modulate
        unsigned char s = msequence_generate_symbol(ms, m);
        fskmod_modulate(mod_orig, s, buf_orig);
    }

    // copy object
    fskmod mod_copy = fskmod_copy(mod_orig);

    // run through both objects and compare
    for (i=0; i<num_symbols; i++) {
        // generate random symbol and modulate
        unsigned char s = msequence_generate_symbol(ms, m);
        fskmod_modulate(mod_orig, s, buf_orig);
        fskmod_modulate(mod_copy, s, buf_copy);
        // check result
        CONTEND_SAME_DATA(buf_orig, buf_copy, k*sizeof(float complex));
    }

    // clean it up
    msequence_destroy(ms);
    fskmod_destroy(mod_orig);
    fskmod_destroy(mod_copy);
}

// test demodulator copy
void autotest_fskdem_copy()
{
    // options
    unsigned int m  = 3;        // bits per symbol
    unsigned int k  = 200;      // samples per symbol
    float        bw = 0.2345f;  // occupied bandwidth

    // create modulator/demodulator pair
    fskdem dem_orig = fskdem_create(m, k, bw);

    unsigned int num_symbols = 96;
    float complex buf[k];

    // run original object
    unsigned int i, j;
    for (i=0; i<num_symbols; i++) {
        // generate random signal and demodulate
        for (j=0; j<k; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        fskdem_demodulate(dem_orig, buf);
    }

    // copy object
    fskdem dem_copy = fskdem_copy(dem_orig);

    // run through both objects and compare
    for (i=0; i<num_symbols; i++) {
        // generate random signal and demodulate
        for (j=0; j<k; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        unsigned int sym_orig = fskdem_demodulate(dem_orig, buf);
        unsigned int sym_copy = fskdem_demodulate(dem_copy, buf);
        // check result
        CONTEND_EQUALITY(sym_orig, sym_copy);
    }

    // clean it up
    fskdem_destroy(dem_orig);
    fskdem_destroy(dem_copy);
}
