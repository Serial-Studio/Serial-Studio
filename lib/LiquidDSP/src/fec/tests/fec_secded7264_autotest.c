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

#include <stdlib.h>
#include <stdio.h>

#include "autotest/autotest.h"
#include "liquid.internal.h"

//
// AUTOTEST: SEC-DEC (72,64) codec (no errors)
//
void autotest_secded7264_codec_e0()
{
    // arrays
    unsigned char sym_org[8];   // original symbol
    unsigned char sym_enc[9];   // encoded symbol
    unsigned char sym_dec[8];   // decoded symbol

    // generate symbol
    sym_org[0] = rand() & 0xff;
    sym_org[1] = rand() & 0xff;
    sym_org[2] = rand() & 0xff;
    sym_org[3] = rand() & 0xff;
    sym_org[4] = rand() & 0xff;
    sym_org[5] = rand() & 0xff;
    sym_org[6] = rand() & 0xff;
    sym_org[7] = rand() & 0xff;

    // encoded symbol
    fec_secded7264_encode_symbol(sym_org, sym_enc);

    // decoded symbol
    fec_secded7264_decode_symbol(sym_enc, sym_dec);

    // validate data are the same
    CONTEND_EQUALITY(sym_org[0], sym_dec[0]);
    CONTEND_EQUALITY(sym_org[1], sym_dec[1]);
    CONTEND_EQUALITY(sym_org[2], sym_dec[2]);
    CONTEND_EQUALITY(sym_org[3], sym_dec[3]);
    CONTEND_EQUALITY(sym_org[4], sym_dec[4]);
    CONTEND_EQUALITY(sym_org[5], sym_dec[5]);
    CONTEND_EQUALITY(sym_org[6], sym_dec[6]);
    CONTEND_EQUALITY(sym_org[7], sym_dec[7]);
}

//
// AUTOTEST: SEC-DEC (72,64) codec (single error)
//
void autotest_secded7264_codec_e1()
{
    // arrays
    unsigned char sym_org[8];   // original symbol
    unsigned char sym_enc[9];   // encoded symbol
    unsigned char e[9];         // error vector
    unsigned char sym_rec[9];   // received symbol
    unsigned char sym_dec[8];   // decoded symbol

    unsigned int i;
    unsigned int k; // error location

    for (k=0; k<72; k++) {
        // generate symbol
        for (i=0; i<8; i++)
            sym_org[i] = rand() & 0xff;

        // encoded symbol
        fec_secded7264_encode_symbol(sym_org, sym_enc);

        // generate error vector (single error)
        for (i=0; i<9; i++)
            e[i] = 0;
        div_t d = div(k,8);
        e[9-d.quot-1] = 1 << d.rem;   // flip bit at index k

        // received symbol
        for (i=0; i<9; i++)
            sym_rec[i] = sym_enc[i] ^ e[i];

        // decoded symbol
        fec_secded7264_decode_symbol(sym_rec, sym_dec);

        // validate data are the same
        for (i=0; i<8; i++)
            CONTEND_EQUALITY(sym_org[i], sym_dec[i]);
    }
}

//
// AUTOTEST: SEC-DEC (72,64) codec (double error detection)
//
void autotest_secded7264_codec_e2()
{
    // total combinations of double errors: nchoosek(72,2) = 2556

    // arrays
    unsigned char sym_org[8];   // original symbol
    unsigned char sym_enc[9];   // encoded symbol
    unsigned char e[9];         // error vector
    unsigned char sym_rec[9];   // received symbol
    unsigned char sym_dec[8];   // decoded symbol

    unsigned int i;
    unsigned int j;
    unsigned int k;

    for (j=0; j<72-1; j++) {
#if 0
        if (liquid_autotest_verbose)
            printf("***** %2u *****\n", j);
#endif
        
        for (k=0; k<72-1-j; k++) {
            // generate symbol
            for (i=0; i<8; i++)
                sym_org[i] = rand() & 0xff;

            // encoded symbol
            fec_secded7264_encode_symbol(sym_org, sym_enc);

            // generate error vector (single error)
            for (i=0; i<9; i++)
                e[i] = 0;

            div_t dj = div(j,8);
            e[9-dj.quot-1] |= 1 << dj.rem;

            div_t dk = div(k+j+1,8);
            e[9-dk.quot-1] |= 1 << dk.rem;

            // received symbol
            for (i=0; i<9; i++)
                sym_rec[i] = sym_enc[i] ^ e[i];

            // decoded symbol
            int syndrome_flag = fec_secded7264_decode_symbol(sym_rec, sym_dec);

#if 0
            if (liquid_autotest_verbose) {
                // print error vector
                printf("%3u, e = ", k);
                liquid_print_bitstring(e[0], 8);
                liquid_print_bitstring(e[1], 8);
                liquid_print_bitstring(e[2], 8);
                liquid_print_bitstring(e[3], 8);
                liquid_print_bitstring(e[4], 8);
                liquid_print_bitstring(e[5], 8);
                liquid_print_bitstring(e[6], 8);
                liquid_print_bitstring(e[7], 8);
                liquid_print_bitstring(e[8], 8);
                printf(" flag=%2d\n", syndrome_flag);
            }
#endif

            // validate syndrome flag is '2'
            CONTEND_EQUALITY(syndrome_flag, 2);
        }
    }
}

