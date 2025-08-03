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

#include "autotest/autotest.h"
#include "liquid.h"

static int bpacketsync_autotest_callback(unsigned char *  _payload,
                                         int              _payload_valid,
                                         unsigned int     _payload_len,
                                         framesyncstats_s _stats,
                                         void *           _userdata)
{
    if (!_payload_valid)
        return 0;

    unsigned int * num_packets_found = (unsigned int *) _userdata;

    *num_packets_found += 1;

    return 0;
}

// 
// AUTOTEST: bpacketsync
//
void autotest_bpacketsync()
{
    // options
    unsigned int num_packets = 50;          // number of packets to encode
    unsigned int dec_msg_len = 64;          // original data message length
    crc_scheme check = LIQUID_CRC_32;       // data integrity check
    fec_scheme fec0 = LIQUID_FEC_HAMMING74; // inner code
    fec_scheme fec1 = LIQUID_FEC_NONE;      // outer code

    // create packet generator
    bpacketgen pg = bpacketgen_create(0, dec_msg_len, check, fec0, fec1);
    if (liquid_autotest_verbose)
        bpacketgen_print(pg);

    // compute packet length
    unsigned int enc_msg_len = bpacketgen_get_packet_len(pg);

    // initialize arrays
    unsigned char msg_org[dec_msg_len]; // original message
    unsigned char msg_enc[enc_msg_len]; // encoded message

    unsigned int num_packets_found=0;

    // create packet synchronizer
    bpacketsync ps = bpacketsync_create(0, bpacketsync_autotest_callback, (void*)&num_packets_found);

    unsigned int i;
    unsigned int n;
    for (n=0; n<num_packets; n++) {
        // initialize original data message
        for (i=0; i<dec_msg_len; i++)
            msg_org[i] = rand() % 256;

        // encode packet
        bpacketgen_encode(pg,msg_org,msg_enc);

        // push packet through synchronizer
        bpacketsync_execute(ps, msg_enc, enc_msg_len);
    }

    // count number of packets
    if (liquid_autotest_verbose)
        printf("found %u / %u packets\n", num_packets_found, num_packets);

    CONTEND_EQUALITY( num_packets_found, num_packets );

    // clean up allocated objects
    bpacketgen_destroy(pg);
    bpacketsync_destroy(ps);
}

