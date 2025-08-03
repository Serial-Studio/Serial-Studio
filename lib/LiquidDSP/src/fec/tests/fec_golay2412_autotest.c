/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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

#include <stdlib.h>
#include <stdio.h>

#include "autotest/autotest.h"
#include "liquid.internal.h"

// generate random error vector with 'n' ones;
// maybe not efficient but effective
unsigned int golay2412_generate_error_vector(unsigned int _n)
{
    if (_n > 24) {
        liquid_error(LIQUID_EINT,"golay2412_generate_error_vector(), cannot generate more than 24 errors");
        return 0;
    }

    unsigned int i;

    unsigned int error_locations[24];
    for (i=0; i<24; i++)
        error_locations[i] = 0;

    unsigned int t=0;
    for (i=0; i<_n; i++) {
        do {
            // generate random error location
            t = rand() % 24;

            // check error location
        } while (error_locations[t]);

        error_locations[t] = 1;
    }

    // generate error vector
    unsigned int e = 0;
    for (i=0; i<24; i++)
        e |= error_locations[i] ? (1 << i) : 0;

    return e;
}

//
// AUTOTEST: Golay(24,12) codec
//
void autotest_golay2412_codec()
{
    unsigned int num_trials=50; // number of symbol trials
    unsigned int num_errors;    // number of errors
    unsigned int i;

    for (num_errors=0; num_errors<=3; num_errors++) {
        for (i=0; i<num_trials; i++) {
            // generate symbol
            unsigned int sym_org = rand() % (1<<12);

            // encoded symbol
            unsigned int sym_enc = fec_golay2412_encode_symbol(sym_org);

            // generate error vector
            unsigned int e = golay2412_generate_error_vector(num_errors);

            // received symbol
            unsigned int sym_rec = sym_enc ^ e;

            // decoded symbol
            unsigned int sym_dec = fec_golay2412_decode_symbol(sym_rec);

#if 0
            printf("error index : %u\n", i);
            // print results
            printf("    sym org     :   "); liquid_print_bitstring(sym_org, n); printf("\n");
            printf("    sym enc     :   "); liquid_print_bitstring(sym_enc, k); printf("\n");
            printf("    sym rec     :   "); liquid_print_bitstring(sym_rec, k); printf("\n");
            printf("    sym dec     :   "); liquid_print_bitstring(sym_dec, n); printf("\n");

            // print number of bit errors
            printf("    bit errors  :   %u\n", count_bit_errors(sym_org, sym_dec));
#endif

            // validate data are the same
            CONTEND_EQUALITY(sym_org, sym_dec);
        }
    }
}

