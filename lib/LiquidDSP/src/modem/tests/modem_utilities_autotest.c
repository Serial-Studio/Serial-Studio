/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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

// test printing schemes
void autotest_modemcf_print_schemes()
{
    CONTEND_EQUALITY(liquid_print_modulation_schemes(), LIQUID_OK);
}

// test parsing string to modulation scheme
void autotest_modemcf_str2mod()
{
    // start with invalid case
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
    CONTEND_EQUALITY(liquid_getopt_str2mod("invalid scheme"), LIQUID_MODEM_UNKNOWN);

    // check normal cases
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk2"     ), LIQUID_MODEM_PSK2);
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk4"     ), LIQUID_MODEM_PSK4);
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk8"     ), LIQUID_MODEM_PSK8);
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk16"    ), LIQUID_MODEM_PSK16);
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk32"    ), LIQUID_MODEM_PSK32);
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk64"    ), LIQUID_MODEM_PSK64);
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk128"   ), LIQUID_MODEM_PSK128);
    CONTEND_EQUALITY(liquid_getopt_str2mod("psk256"   ), LIQUID_MODEM_PSK256);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk2"    ), LIQUID_MODEM_DPSK2);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk4"    ), LIQUID_MODEM_DPSK4);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk8"    ), LIQUID_MODEM_DPSK8);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk16"   ), LIQUID_MODEM_DPSK16);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk32"   ), LIQUID_MODEM_DPSK32);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk64"   ), LIQUID_MODEM_DPSK64);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk128"  ), LIQUID_MODEM_DPSK128);
    CONTEND_EQUALITY(liquid_getopt_str2mod("dpsk256"  ), LIQUID_MODEM_DPSK256);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask2"     ), LIQUID_MODEM_ASK2);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask4"     ), LIQUID_MODEM_ASK4);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask8"     ), LIQUID_MODEM_ASK8);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask16"    ), LIQUID_MODEM_ASK16);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask32"    ), LIQUID_MODEM_ASK32);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask64"    ), LIQUID_MODEM_ASK64);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask128"   ), LIQUID_MODEM_ASK128);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ask256"   ), LIQUID_MODEM_ASK256);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qam4"     ), LIQUID_MODEM_QAM4);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qam8"     ), LIQUID_MODEM_QAM8);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qam16"    ), LIQUID_MODEM_QAM16);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qam32"    ), LIQUID_MODEM_QAM32);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qam64"    ), LIQUID_MODEM_QAM64);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qam128"   ), LIQUID_MODEM_QAM128);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qam256"   ), LIQUID_MODEM_QAM256);
    CONTEND_EQUALITY(liquid_getopt_str2mod("apsk4"    ), LIQUID_MODEM_APSK4);
    CONTEND_EQUALITY(liquid_getopt_str2mod("apsk8"    ), LIQUID_MODEM_APSK8);
    CONTEND_EQUALITY(liquid_getopt_str2mod("apsk16"   ), LIQUID_MODEM_APSK16);
    CONTEND_EQUALITY(liquid_getopt_str2mod("apsk32"   ), LIQUID_MODEM_APSK32);
    CONTEND_EQUALITY(liquid_getopt_str2mod("apsk64"   ), LIQUID_MODEM_APSK64);
    CONTEND_EQUALITY(liquid_getopt_str2mod("apsk128"  ), LIQUID_MODEM_APSK128);
    CONTEND_EQUALITY(liquid_getopt_str2mod("apsk256"  ), LIQUID_MODEM_APSK256);
    CONTEND_EQUALITY(liquid_getopt_str2mod("bpsk"     ), LIQUID_MODEM_BPSK);
    CONTEND_EQUALITY(liquid_getopt_str2mod("qpsk"     ), LIQUID_MODEM_QPSK);
    CONTEND_EQUALITY(liquid_getopt_str2mod("ook"      ), LIQUID_MODEM_OOK);
    CONTEND_EQUALITY(liquid_getopt_str2mod("sqam32"   ), LIQUID_MODEM_SQAM32);
    CONTEND_EQUALITY(liquid_getopt_str2mod("sqam128"  ), LIQUID_MODEM_SQAM128);
    CONTEND_EQUALITY(liquid_getopt_str2mod("V29"      ), LIQUID_MODEM_V29);
    CONTEND_EQUALITY(liquid_getopt_str2mod("arb16opt" ), LIQUID_MODEM_ARB16OPT);
    CONTEND_EQUALITY(liquid_getopt_str2mod("arb32opt" ), LIQUID_MODEM_ARB32OPT);
    CONTEND_EQUALITY(liquid_getopt_str2mod("arb64opt" ), LIQUID_MODEM_ARB64OPT);
    CONTEND_EQUALITY(liquid_getopt_str2mod("arb128opt"), LIQUID_MODEM_ARB128OPT);
    CONTEND_EQUALITY(liquid_getopt_str2mod("arb256opt"), LIQUID_MODEM_ARB256OPT);
    CONTEND_EQUALITY(liquid_getopt_str2mod("arb64vt"  ), LIQUID_MODEM_ARB64VT);
}

// test basic types
void autotest_modemcf_types()
{
    // Phase-shift keying (PSK)
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK2),     1);
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK4),     1);
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK8),     1);
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK16),    1);
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK32),    1);
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK64),    1);
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK128),   1);
    CONTEND_EQUALITY(liquid_modem_is_psk(LIQUID_MODEM_PSK256),   1);

    // Differential phase-shift keying (DPSK)
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK2),    1);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK4),    1);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK8),    1);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK16),   1);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK32),   1);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK64),   1);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK128),  1);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_DPSK256),  1);

    // amplitude-shift keying (ASK)
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK2),     1);
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK4),     1);
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK8),     1);
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK16),    1);
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK32),    1);
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK64),    1);
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK128),   1);
    CONTEND_EQUALITY(liquid_modem_is_ask(LIQUID_MODEM_ASK256),   1);

    // rectangular quadrature amplitude-shift keying (QAM)
    CONTEND_EQUALITY(liquid_modem_is_qam(LIQUID_MODEM_QAM4),     1);
    CONTEND_EQUALITY(liquid_modem_is_qam(LIQUID_MODEM_QAM8),     1);
    CONTEND_EQUALITY(liquid_modem_is_qam(LIQUID_MODEM_QAM16),    1);
    CONTEND_EQUALITY(liquid_modem_is_qam(LIQUID_MODEM_QAM32),    1);
    CONTEND_EQUALITY(liquid_modem_is_qam(LIQUID_MODEM_QAM64),    1);
    CONTEND_EQUALITY(liquid_modem_is_qam(LIQUID_MODEM_QAM128),   1);
    CONTEND_EQUALITY(liquid_modem_is_qam(LIQUID_MODEM_QAM256),   1);

    // amplitude phase-shift keying (APSK)
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_APSK4),    1);
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_APSK8),    1);
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_APSK16),   1);
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_APSK32),   1);
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_APSK64),   1);
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_APSK128),  1);
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_APSK256),  1);

#if 0
    // specific modems
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_BPSK),      1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_QPSK),      1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_OOK),       1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_SQAM32),    1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_SQAM128),   1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_V29),       1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_ARB16OPT),  1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_ARB32OPT),  1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_ARB64OPT),  1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_ARB128OPT), 1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_ARB256OPT), 1);
    CONTEND_EQUALITY(liquid_modem_is_specific(LIQUID_MODEM_ARB64VT),   1);
#endif

    // test some negative cases
    CONTEND_EQUALITY(liquid_modem_is_psk (LIQUID_MODEM_QPSK), 0);
    CONTEND_EQUALITY(liquid_modem_is_dpsk(LIQUID_MODEM_QPSK), 0);
    CONTEND_EQUALITY(liquid_modem_is_ask (LIQUID_MODEM_QPSK), 0);
    CONTEND_EQUALITY(liquid_modem_is_qam (LIQUID_MODEM_QPSK), 0);
    CONTEND_EQUALITY(liquid_modem_is_apsk(LIQUID_MODEM_QPSK), 0);
}

