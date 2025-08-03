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
#include "liquid.internal.h"

//
// AUTOTEST: Hamming (8,4) codec
//
void autotest_hamming84_codec()
{
    unsigned int n=4;
    unsigned char msg[] = {0x25, 0x62, 0x3F, 0x52};
    fec_scheme fs = LIQUID_FEC_HAMMING84;

    // create arrays
    unsigned int n_enc = fec_get_enc_msg_length(fs,n);
    unsigned char msg_dec[n];
    unsigned char msg_enc[n_enc];

    // create object
    fec q = fec_create(fs,NULL);
    if (liquid_autotest_verbose)
        fec_print(q);

    // encode message
    fec_encode(q, n, msg, msg_enc);
    
    // corrupt encoded message
    msg_enc[0] ^= 0x04; // position 5
    msg_enc[1] ^= 0x04; //
    msg_enc[2] ^= 0x02; //
    msg_enc[3] ^= 0x01; //
    msg_enc[4] ^= 0x80; //
    msg_enc[5] ^= 0x40; //
    msg_enc[6] ^= 0x20; //
    msg_enc[7] ^= 0x10; //

    // decode message
    fec_decode(q, n, msg_enc, msg_dec);

    // validate data are the same
    CONTEND_SAME_DATA(msg, msg_dec, n);

    // clean up objects
    fec_destroy(q);
}

//
// AUTOTEST: Hamming (8,4) codec (soft decoding)
//
void autotest_hamming84_codec_soft()
{
    // generate each of the 2^4=16 symbols, encode, and decode
    // using soft decoding algorithm
    unsigned char s;            // original 4-bit symbol
    unsigned char c;            // encoded 8-bit symbol
    unsigned char c_soft[8];    // soft bits
    unsigned char s_hat;        // decoded symbol

    for (s=0; s<16; s++) {
        // encode using look-up table
        c = hamming84_enc_gentab[s];

        // expand soft bits
        c_soft[0] = (c & 0x80) ? 255 : 0;
        c_soft[1] = (c & 0x40) ? 255 : 0;
        c_soft[2] = (c & 0x20) ? 255 : 0;
        c_soft[3] = (c & 0x10) ? 255 : 0;
        c_soft[4] = (c & 0x08) ? 255 : 0;
        c_soft[5] = (c & 0x04) ? 255 : 0;
        c_soft[6] = (c & 0x02) ? 255 : 0;
        c_soft[7] = (c & 0x01) ? 255 : 0;

        // decode using internal soft decoding method
        s_hat = fecsoft_hamming84_decode(c_soft);

        // contend that data are the same
        CONTEND_EQUALITY(s, s_hat);
    }
}

