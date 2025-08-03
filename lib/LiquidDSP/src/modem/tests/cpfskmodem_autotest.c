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

#include "autotest/autotest.h"
#include "liquid.h"

// Help function to keep code base small
void cpfskmodem_test_mod_demod(cpfskmod mod, cpfskdem dem)
{
    // derived values
    unsigned int delay = cpfskmod_get_delay(mod) + cpfskdem_get_delay(dem);
    unsigned int k   = cpfskmod_get_samples_per_symbol(mod);
    unsigned int bps = cpfskmod_get_bits_per_symbol(mod);

    unsigned int num_symbols = 180 + delay; // number of symbols to test
    float complex buf[k];      // sample buffer
    unsigned int  sym_in [num_symbols]; // symbol buffer
    unsigned int  sym_out[num_symbols]; // symbol buffer

    // modulate, demodulate, count errors
    msequence ms = msequence_create_default(7);
    unsigned int i;
    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        sym_in[i] = msequence_generate_symbol(ms, bps);

        // modulate
        cpfskmod_modulate(mod, sym_in[i], buf);

        // demodulate
        sym_out[i] = cpfskdem_demodulate(dem, buf);
    }
    msequence_destroy(ms);

    // count errors
    for (i=0; i<num_symbols; i++) {
        if (i >= delay) {
            // print results
            if (liquid_autotest_verbose) {
                printf("  %3u : input = %2u, output = %2u %s\n",
                        i, sym_in[i-delay], sym_out[i],
                        (sym_in[i-delay] == sym_out[i]) ? "" : "*");
            }

            // check result
            CONTEND_EQUALITY(sym_in[i-delay], sym_out[i]);
        }
    }
}

// Help function to keep code base small
void cpfskmodem_test_harness(unsigned int _bps,
                             float        _h,
                             unsigned int _k,
                             unsigned int _m,
                             float        _beta,
                             int          _filter_type)
{
    // create modulator/demodulator pair
    cpfskmod mod = cpfskmod_create(_bps, _h, _k, _m, _beta, _filter_type);
    cpfskdem dem = cpfskdem_create(_bps, _h, _k, _m, _beta, _filter_type);

    // ensure values match
    CONTEND_EQUALITY( cpfskmod_get_samples_per_symbol(mod), _k );
    CONTEND_EQUALITY( cpfskdem_get_samples_per_symbol(dem), _k );

    // run modulation/demodulation tests
    cpfskmodem_test_mod_demod(mod, dem);

    // clean it up
    cpfskmod_destroy(mod);
    cpfskdem_destroy(dem);
}

//
// AUTOTESTS: check different modulation indices
//

// square pulse shape
void autotest_cpfskmodem_bps1_h0p5000_k4_m3_square()    { cpfskmodem_test_harness( 1, 0.5000f, 4, 3, 0.25f, LIQUID_CPFSK_SQUARE ); }
void autotest_cpfskmodem_bps1_h0p0250_k4_m3_square()    { cpfskmodem_test_harness( 1, 0.2500f, 4, 3, 0.25f, LIQUID_CPFSK_SQUARE ); }
void autotest_cpfskmodem_bps1_h0p1250_k4_m3_square()    { cpfskmodem_test_harness( 1, 0.1250f, 4, 3, 0.25f, LIQUID_CPFSK_SQUARE ); }
void autotest_cpfskmodem_bps1_h0p0625_k4_m3_square()    { cpfskmodem_test_harness( 1, 0.0625f, 4, 3, 0.25f, LIQUID_CPFSK_SQUARE ); }

// raised-cosine pulse shape (full)
void autotest_cpfskmodem_bps1_h0p5000_k4_m3_rcosfull()  { cpfskmodem_test_harness( 1, 0.5000f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_FULL ); }
void autotest_cpfskmodem_bps1_h0p0250_k4_m3_rcosfull()  { cpfskmodem_test_harness( 1, 0.2500f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_FULL ); }
void autotest_cpfskmodem_bps1_h0p1250_k4_m3_rcosfull()  { cpfskmodem_test_harness( 1, 0.1250f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_FULL ); }
void autotest_cpfskmodem_bps1_h0p0625_k4_m3_rcosfull()  { cpfskmodem_test_harness( 1, 0.0625f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_FULL ); }

// raised-cosine pulse shape (partial)
void autotest_cpfskmodem_bps1_h0p5000_k4_m3_rcospart()  { cpfskmodem_test_harness( 1, 0.5000f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_PARTIAL ); }
void autotest_cpfskmodem_bps1_h0p0250_k4_m3_rcospart()  { cpfskmodem_test_harness( 1, 0.2500f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_PARTIAL ); }
void autotest_cpfskmodem_bps1_h0p1250_k4_m3_rcospart()  { cpfskmodem_test_harness( 1, 0.1250f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_PARTIAL ); }
void autotest_cpfskmodem_bps1_h0p0625_k4_m3_rcospart()  { cpfskmodem_test_harness( 1, 0.0625f, 4, 3, 0.25f, LIQUID_CPFSK_RCOS_PARTIAL ); }

// Gauss minimum-shift keying
void autotest_cpfskmodem_bps1_h0p5000_k4_m3_gmsk()      { cpfskmodem_test_harness( 1, 0.5000f, 4, 3, 0.25f, LIQUID_CPFSK_GMSK ); }
void autotest_cpfskmodem_bps1_h0p0250_k4_m3_gmsk()      { cpfskmodem_test_harness( 1, 0.2500f, 4, 3, 0.25f, LIQUID_CPFSK_GMSK ); }
void autotest_cpfskmodem_bps1_h0p1250_k4_m3_gmsk()      { cpfskmodem_test_harness( 1, 0.1250f, 4, 3, 0.25f, LIQUID_CPFSK_GMSK ); }
void autotest_cpfskmodem_bps1_h0p0625_k4_m3_gmsk()      { cpfskmodem_test_harness( 1, 0.0625f, 4, 3, 0.25f, LIQUID_CPFSK_GMSK ); }

//
// AUTOTESTS: check different bits per symbol
//

// square pulse shape
void autotest_cpfskmodem_bps2_h0p0250_k4_m3_square()    { cpfskmodem_test_harness( 2, 0.2500f, 4, 3, 0.25f, LIQUID_CPFSK_SQUARE ); }
void autotest_cpfskmodem_bps3_h0p1250_k4_m3_square()    { cpfskmodem_test_harness( 3, 0.1250f, 4, 3, 0.25f, LIQUID_CPFSK_SQUARE ); }
void autotest_cpfskmodem_bps4_h0p0625_k4_m3_square()    { cpfskmodem_test_harness( 4, 0.0625f, 4, 3, 0.25f, LIQUID_CPFSK_SQUARE ); }

//
// AUTOTESTS: check different samples per symbol
//

// GMSK
// TODO: allow samples per symbol to be odd
void autotest_cpfskmodem_bps1_h0p5_k2_m7_gmsk() { cpfskmodem_test_harness( 1, 0.5f, 2, 7, 0.30f, LIQUID_CPFSK_GMSK ); }
void autotest_cpfskmodem_bps1_h0p5_k4_m7_gmsk() { cpfskmodem_test_harness( 1, 0.5f, 4, 7, 0.30f, LIQUID_CPFSK_GMSK ); }
void autotest_cpfskmodem_bps1_h0p5_k6_m7_gmsk() { cpfskmodem_test_harness( 1, 0.5f, 6, 7, 0.30f, LIQUID_CPFSK_GMSK ); }
void autotest_cpfskmodem_bps1_h0p5_k8_m7_gmsk() { cpfskmodem_test_harness( 1, 0.5f, 8, 7, 0.30f, LIQUID_CPFSK_GMSK ); }

// test spectral response
void autotest_cpfskmodem_spectrum()
{
    // create modulator
    unsigned int bps    = 1;
    float        h      = 0.5f;
    unsigned int k      = 4;
    unsigned int m      = 3;
    float        beta   = 0.35f;
    int          type   = LIQUID_CPFSK_RCOS_PARTIAL;
    cpfskmod mod = cpfskmod_create(bps, h, k, m, beta, type);

    // spectral periodogram options
    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_symbols = 192000;  // number of symbols to generate
    float complex buf[k];
    unsigned int i;

    // modulate many, many symbols
    for (i=0; i<(1U<<24U); i++)
        cpfskmod_modulate(mod, 0, buf);

    // modulate several symbols and run result through spectral estimate
    spgramcf periodogram = spgramcf_create_default(nfft);
    for (i=0; i<num_symbols; i++) {
        cpfskmod_modulate(mod, rand() & ((1<<bps)-1), buf);
        spgramcf_write(periodogram, buf, k);
    }

    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    cpfskmod_destroy(mod);
    spgramcf_destroy(periodogram);

    // verify spectrum
    autotest_psd_s regions[] = {
      {.fmin=-0.50, .fmax=-0.35, .pmin= 0.0, .pmax=-40.0, .test_lo=0, .test_hi=1},
      {.fmin=-0.35, .fmax=-0.20, .pmin= 0.0, .pmax=-20.0, .test_lo=0, .test_hi=1},
      {.fmin=-0.10, .fmax= 0.10, .pmin= 0.0, .pmax= 10.0, .test_lo=1, .test_hi=1},
      {.fmin= 0.20, .fmax= 0.35, .pmin= 0.0, .pmax=-20.0, .test_lo=0, .test_hi=1},
      {.fmin= 0.35, .fmax= 0.50, .pmin= 0.0, .pmax=-40.0, .test_lo=0, .test_hi=1},
    };
    char filename[256];
    //sprintf(filename,"autotest/logs/cpfskmodem_psd_b%u_h%.3u_k%u_m%u_b%.3u_t%u_autotest.m",
    //        bps, (int)(h*100), k, m, (int)(beta*100), type);
    sprintf(filename,"autotest/logs/cpfskmodem_psd_autotest.m");
    liquid_autotest_validate_spectrum(psd, nfft, regions, 5,
        liquid_autotest_verbose ? filename : NULL);
}

// test errors and invalid configuration
void autotest_cpfskmodem_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping modem config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // test copying/creating invalid objects
    //CONTEND_ISNULL( modemcf_copy(NULL) );
    CONTEND_ISNULL( cpfskmod_create(0, 0.5f, 4, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _bps is less than 1
    CONTEND_ISNULL( cpfskmod_create(1, 0.0f, 4, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _h (mod index) is out of range
    CONTEND_ISNULL( cpfskmod_create(1, 0.5f, 0, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _k is too small
    CONTEND_ISNULL( cpfskmod_create(1, 0.5f, 5, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _k is not even
    CONTEND_ISNULL( cpfskmod_create(1, 0.5f, 4,  0, 0.25f, LIQUID_CPFSK_SQUARE) ); // _m is too small
    CONTEND_ISNULL( cpfskmod_create(1, 0.5f, 4, 12, 0.00f, LIQUID_CPFSK_SQUARE) ); // _beta is too small
    CONTEND_ISNULL( cpfskmod_create(1, 0.5f, 4, 12, 7.22f, LIQUID_CPFSK_SQUARE) ); // _beta is too large
    CONTEND_ISNULL( cpfskmod_create(1, 0.5f, 4, 12, 0.25f, -1) ); // invalid filter type

    CONTEND_ISNULL( cpfskdem_create(0, 0.5f, 4, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _bps is less than 1
    CONTEND_ISNULL( cpfskdem_create(1, 0.0f, 4, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _h (mod index) is out of range
    CONTEND_ISNULL( cpfskdem_create(1, 0.5f, 0, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _k is too small
    CONTEND_ISNULL( cpfskdem_create(1, 0.5f, 5, 12, 0.25f, LIQUID_CPFSK_SQUARE) ); // _k is not even
    CONTEND_ISNULL( cpfskdem_create(1, 0.5f, 4,  0, 0.25f, LIQUID_CPFSK_SQUARE) ); // _m is too small
    CONTEND_ISNULL( cpfskdem_create(1, 0.5f, 4, 12, 0.00f, LIQUID_CPFSK_SQUARE) ); // _beta is too small
    CONTEND_ISNULL( cpfskdem_create(1, 0.5f, 4, 12, 7.22f, LIQUID_CPFSK_SQUARE) ); // _beta is too large
    CONTEND_ISNULL( cpfskdem_create(1, 0.5f, 4, 12, 0.25f, -1) ); // invalid filter type

    // create modulator object and check configuration
    cpfskmod mod = cpfskmod_create(1, 0.5f, 4, 12, 0.5f, LIQUID_CPFSK_SQUARE);
    CONTEND_EQUALITY( LIQUID_OK, cpfskmod_print(mod) );
    cpfskmod_destroy(mod);

    // create demodulator object and check configuration
    cpfskdem dem = cpfskdem_create(1, 0.5f, 4, 12, 0.5f, LIQUID_CPFSK_SQUARE);
    CONTEND_EQUALITY( LIQUID_OK, cpfskdem_print(dem) );
    cpfskdem_destroy(dem);
}

