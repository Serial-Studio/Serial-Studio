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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

//
// AUTOTEST: Reed-Solomon codecs
//
void autotest_reedsolomon_223_255()
{
#if !LIBFEC_ENABLED
    liquid_error(LIQUID_EUMODE,"Reed-Solomon codes unavailable (install libfec)");
    return;
#endif

    unsigned int dec_msg_len = 223;

    // compute and test encoded message length
    unsigned int enc_msg_len = fec_get_enc_msg_length(LIQUID_FEC_RS_M8,dec_msg_len);
    CONTEND_EQUALITY( enc_msg_len, 255 );

    // create arrays
    unsigned char msg_org[dec_msg_len]; // original message
    unsigned char msg_enc[enc_msg_len]; // encoded message
    unsigned char msg_rec[enc_msg_len]; // received message
    unsigned char msg_dec[dec_msg_len]; // decoded message

    // initialize original message
    unsigned int i;
    for (i=0; i<dec_msg_len; i++)
        msg_org[i] = i & 0xff;

    // create object
    fec q = fec_create(LIQUID_FEC_RS_M8,NULL);
    if (liquid_autotest_verbose)
        fec_print(q);

    // encode message
    fec_encode(q, dec_msg_len, msg_org, msg_enc);
    
    // corrupt encoded message; can withstand up to 16 symbol errors
    memmove(msg_rec, msg_enc, enc_msg_len*sizeof(unsigned char));
    for (i=0; i<16; i++)
        msg_rec[i] ^= 0x75;

    // decode message
    fec_decode(q, dec_msg_len, msg_rec, msg_dec);

    // validate data are the same
    CONTEND_SAME_DATA(msg_org, msg_dec, dec_msg_len);

    if (liquid_autotest_verbose) {
        printf("enc   dec\n");
        printf("---   ---\n");
        for (i=0; i<dec_msg_len; i++)
            printf("%3u   %3u\n", msg_org[i], msg_dec[i]);
    }

    // clean up objects
    fec_destroy(q);
}

