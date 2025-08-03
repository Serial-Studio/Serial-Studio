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

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include "liquid.internal.h"

#define MODEM_MODULATE_BENCH_API(MS)    \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ modemcf_modulate_bench(_start, _finish, _num_iterations, MS); }

// Helper function to keep code base small
void modemcf_modulate_bench(struct rusage *_start,
                            struct rusage *_finish,
                            unsigned long int *_num_iterations,
                            modulation_scheme _ms)
{
    // initialize modulator
    modemcf mod = modemcf_create(_ms);

    unsigned long int i;
    float complex x;
    unsigned int symbol_in = 0;
    
    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        modemcf_modulate(mod, symbol_in, &x);
        modemcf_modulate(mod, symbol_in, &x);
        modemcf_modulate(mod, symbol_in, &x);
        modemcf_modulate(mod, symbol_in, &x);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    modemcf_destroy(mod);
}

// specific modems
void benchmark_modulate_bpsk    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_BPSK)
void benchmark_modulate_qpsk    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QPSK)
void benchmark_modulate_ook     MODEM_MODULATE_BENCH_API(LIQUID_MODEM_OOK)
void benchmark_modulate_sqam32  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_SQAM32)
void benchmark_modulate_sqam128 MODEM_MODULATE_BENCH_API(LIQUID_MODEM_SQAM128)

// ASK
void benchmark_modulate_ask2    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ASK2)
void benchmark_modulate_ask4    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ASK4)
void benchmark_modulate_ask8    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ASK8)
void benchmark_modulate_ask16   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ASK16)

// PSK
void benchmark_modulate_psk2    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_PSK2)
void benchmark_modulate_psk4    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_PSK4)
void benchmark_modulate_psk8    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_PSK8)
void benchmark_modulate_psk16   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_PSK16)
void benchmark_modulate_psk32   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_PSK32)
void benchmark_modulate_psk64   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_PSK64)

// Differential PSK
void benchmark_modulate_dpsk2   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_DPSK2)
void benchmark_modulate_dpsk4   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_DPSK4)
void benchmark_modulate_dpsk8   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_DPSK8)
void benchmark_modulate_dpsk16  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_DPSK16)
void benchmark_modulate_dpsk32  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_DPSK32)
void benchmark_modulate_dpsk64  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_DPSK64)

// QAM
void benchmark_modulate_qam4    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QAM4)
void benchmark_modulate_qam8    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QAM8)
void benchmark_modulate_qam16   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QAM16)
void benchmark_modulate_qam32   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QAM32)
void benchmark_modulate_qam64   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QAM64)
void benchmark_modulate_qam128  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QAM128)
void benchmark_modulate_qam256  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_QAM256)

// APSK
void benchmark_modulate_apsk4   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_APSK4)
void benchmark_modulate_apsk8   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_APSK8)
void benchmark_modulate_apsk16  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_APSK16)
void benchmark_modulate_apsk32  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_APSK32)
void benchmark_modulate_apsk64  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_APSK64)
void benchmark_modulate_apsk128 MODEM_MODULATE_BENCH_API(LIQUID_MODEM_APSK128)
void benchmark_modulate_apsk256 MODEM_MODULATE_BENCH_API(LIQUID_MODEM_APSK256)

// ARB
void benchmark_modulate_arbV29    MODEM_MODULATE_BENCH_API(LIQUID_MODEM_V29)
void benchmark_modulate_arb16opt  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ARB16OPT)
void benchmark_modulate_arb32opt  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ARB32OPT)
void benchmark_modulate_arb64opt  MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ARB64OPT)
void benchmark_modulate_arb128opt MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ARB128OPT)
void benchmark_modulate_arb256opt MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ARB256OPT)
void benchmark_modulate_arb64vt   MODEM_MODULATE_BENCH_API(LIQUID_MODEM_ARB64VT)

