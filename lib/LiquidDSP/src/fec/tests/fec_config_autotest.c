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

#include <stdlib.h>
#include <stdio.h>

#include "autotest/autotest.h"
#include "liquid.internal.h"

void autotest_fec_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping fec_hamming3126 config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // symbols too large
    CONTEND_EQUALITY(fec_golay2412_encode_symbol(1u<<12), 0)
    CONTEND_EQUALITY(fec_golay2412_decode_symbol(1u<<24), 0)

    CONTEND_EQUALITY(fec_hamming3126_encode_symbol(1u<<26), 0)
    CONTEND_EQUALITY(fec_hamming3126_decode_symbol(1u<<31), 0)

    CONTEND_EQUALITY(fec_hamming1511_encode_symbol(1u<<11), 0)
    CONTEND_EQUALITY(fec_hamming1511_decode_symbol(1u<<15), 0)

    CONTEND_EQUALITY(fec_hamming128_encode_symbol(1u<<8), 0)
    CONTEND_EQUALITY(fec_hamming128_decode_symbol(1u<<12), 0)

    // check printing schemes
    CONTEND_EQUALITY(liquid_print_fec_schemes(), LIQUID_OK);
}

void autotest_fec_str2fec()
{
    // start with invalid case
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
    CONTEND_EQUALITY(liquid_getopt_str2fec("invalid scheme"), LIQUID_MODEM_UNKNOWN);

    // check normal cases
    CONTEND_EQUALITY( liquid_getopt_str2fec("none"),       LIQUID_FEC_NONE);
    CONTEND_EQUALITY( liquid_getopt_str2fec("rep3"),       LIQUID_FEC_REP3);
    CONTEND_EQUALITY( liquid_getopt_str2fec("rep5"),       LIQUID_FEC_REP5);
    CONTEND_EQUALITY( liquid_getopt_str2fec("h74"),        LIQUID_FEC_HAMMING74);
    CONTEND_EQUALITY( liquid_getopt_str2fec("h84"),        LIQUID_FEC_HAMMING84);
    CONTEND_EQUALITY( liquid_getopt_str2fec("h128"),       LIQUID_FEC_HAMMING128);
    CONTEND_EQUALITY( liquid_getopt_str2fec("g2412"),      LIQUID_FEC_GOLAY2412);
    CONTEND_EQUALITY( liquid_getopt_str2fec("secded2216"), LIQUID_FEC_SECDED2216);
    CONTEND_EQUALITY( liquid_getopt_str2fec("secded3932"), LIQUID_FEC_SECDED3932);
    CONTEND_EQUALITY( liquid_getopt_str2fec("secded7264"), LIQUID_FEC_SECDED7264);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v27"),        LIQUID_FEC_CONV_V27);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v29"),        LIQUID_FEC_CONV_V29);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v39"),        LIQUID_FEC_CONV_V39);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v615"),       LIQUID_FEC_CONV_V615);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v27p23"),     LIQUID_FEC_CONV_V27P23);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v27p34"),     LIQUID_FEC_CONV_V27P34);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v27p45"),     LIQUID_FEC_CONV_V27P45);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v27p56"),     LIQUID_FEC_CONV_V27P56);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v27p67"),     LIQUID_FEC_CONV_V27P67);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v27p78"),     LIQUID_FEC_CONV_V27P78);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v29p23"),     LIQUID_FEC_CONV_V29P23);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v29p34"),     LIQUID_FEC_CONV_V29P34);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v29p45"),     LIQUID_FEC_CONV_V29P45);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v29p56"),     LIQUID_FEC_CONV_V29P56);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v29p67"),     LIQUID_FEC_CONV_V29P67);
    CONTEND_EQUALITY( liquid_getopt_str2fec("v29p78"),     LIQUID_FEC_CONV_V29P78);
    CONTEND_EQUALITY( liquid_getopt_str2fec("rs8"),        LIQUID_FEC_RS_M8);
}

void autotest_fec_is_convolutional()
{
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_NONE),        0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_REP3),        0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_REP5),        0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_HAMMING74),   0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_HAMMING84),   0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_HAMMING128),  0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_GOLAY2412),   0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_SECDED2216),  0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_SECDED3932),  0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_SECDED7264),  0 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V27),    1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V29),    1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V39),    1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V615),   1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V27P23), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V27P34), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V27P45), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V27P56), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V27P67), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V27P78), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V29P23), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V29P34), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V29P45), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V29P56), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V29P67), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_CONV_V29P78), 1 );
    CONTEND_EQUALITY( fec_scheme_is_convolutional(LIQUID_FEC_RS_M8),       0 );
}

void autotest_fec_is_punctured()
{
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_NONE),        0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_REP3),        0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_REP5),        0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_HAMMING74),   0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_HAMMING84),   0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_HAMMING128),  0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_GOLAY2412),   0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_SECDED2216),  0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_SECDED3932),  0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_SECDED7264),  0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V27),    0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V29),    0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V39),    0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V615),   0 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V27P23), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V27P34), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V27P45), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V27P56), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V27P67), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V27P78), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V29P23), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V29P34), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V29P45), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V29P56), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V29P67), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_CONV_V29P78), 1 );
    CONTEND_EQUALITY( fec_scheme_is_punctured(LIQUID_FEC_RS_M8),       0 );
}

void autotest_fec_is_reedsolomon()
{
    CONTEND_EQUALITY( fec_scheme_is_reedsolomon(LIQUID_FEC_NONE),        0 );
    CONTEND_EQUALITY( fec_scheme_is_reedsolomon(LIQUID_FEC_REP3),        0 );
    CONTEND_EQUALITY( fec_scheme_is_reedsolomon(LIQUID_FEC_HAMMING74),   0 );
    CONTEND_EQUALITY( fec_scheme_is_reedsolomon(LIQUID_FEC_GOLAY2412),   0 );
    CONTEND_EQUALITY( fec_scheme_is_reedsolomon(LIQUID_FEC_SECDED2216),  0 );
    CONTEND_EQUALITY( fec_scheme_is_reedsolomon(LIQUID_FEC_CONV_V27),    0 );
    CONTEND_EQUALITY( fec_scheme_is_reedsolomon(LIQUID_FEC_RS_M8),       1 );
}

void autotest_fec_is_hamming()
{
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_NONE),        0 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_REP3),        0 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_HAMMING74),   1 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_HAMMING84),   1 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_HAMMING128),  1 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_GOLAY2412),   0 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_SECDED2216),  0 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_CONV_V27),    0 );
    CONTEND_EQUALITY( fec_scheme_is_hamming(LIQUID_FEC_RS_M8),       0 );
}

