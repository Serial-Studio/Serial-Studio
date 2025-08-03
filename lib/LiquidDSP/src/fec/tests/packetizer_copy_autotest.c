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

#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.h"

void autotest_packetizer_copy()
{
    unsigned int msg_len_dec = 57;
    crc_scheme   crc         = LIQUID_CRC_32;
    fec_scheme   fec0        = LIQUID_FEC_HAMMING128;
    fec_scheme   fec1        = LIQUID_FEC_GOLAY2412;

    // compute encoded message length
    unsigned int  msg_len_enc = packetizer_compute_enc_msg_len(msg_len_dec,crc,fec0,fec1);

    // allocate arrays for testing
    unsigned char msg_org  [msg_len_dec];   // original message
    unsigned char msg_enc_0[msg_len_enc];   // encoded (original object)
    unsigned char msg_enc_1[msg_len_enc];   // encoded (copy object)
    unsigned char msg_dec_0[msg_len_dec];   // decoded (original object)
    unsigned char msg_dec_1[msg_len_dec];   // encoded (copy object)

    // create object
    packetizer q0 = packetizer_create(msg_len_dec,crc,fec0,fec1);

    // initialize random data
    unsigned int i;
    for (i=0; i<msg_len_dec; i++)
        msg_org[i] = rand() & 0xff;

    // encode packet
    packetizer_encode(q0, msg_org, msg_enc_0);

    // copy object, encode, and compare result
    packetizer q1 = packetizer_copy(q0);
    packetizer_encode(q1, msg_org, msg_enc_1);
    CONTEND_SAME_DATA(msg_enc_0, msg_enc_1, msg_len_enc);

    // initialize random data for decoder input
    for (i=0; i<msg_len_enc; i++) {
        msg_enc_0[i] = rand() & 0xff;
        msg_enc_1[i] = msg_enc_0[i];
    }

    // decode and compare
    // NOTE: we don't care if the output is valid; just that they match
    int crc_pass_0 = packetizer_decode(q0, msg_enc_0, msg_dec_0);
    int crc_pass_1 = packetizer_decode(q1, msg_enc_1, msg_dec_1);

    CONTEND_SAME_DATA(msg_dec_0, msg_dec_1, msg_len_dec);
    CONTEND_EQUALITY(crc_pass_0, crc_pass_1);

    // clean up objects
    packetizer_destroy(q0);
    packetizer_destroy(q1);
}

