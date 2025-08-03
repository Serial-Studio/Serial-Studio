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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "autotest/autotest.h"
#include "liquid.h"

// 
// AUTOTEST : test simple recovery of frame in noise
//
void autotest_flexframesync()
{
    unsigned int i;

    unsigned int _payload_len = 400;
    int _ms    = LIQUID_MODEM_QPSK;
    int _fec0  = LIQUID_FEC_NONE;
    int _fec1  = LIQUID_FEC_NONE;
    int _check = LIQUID_CRC_32;

    // create flexframegen object
    flexframegenprops_s fgprops;
    flexframegenprops_init_default(&fgprops);
    fgprops.mod_scheme  = _ms;
    fgprops.check       = _check;
    fgprops.fec0        = _fec0;
    fgprops.fec1        = _fec1;
    flexframegen fg = flexframegen_create(&fgprops);

    // create flexframesync object
    unsigned int context = 0;
    flexframesync fs = flexframesync_create(framing_autotest_callback, (void*)&context);
    
    // initialize header and payload
    unsigned char header[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    unsigned char payload[_payload_len];
    for (i=0; i<_payload_len; i++)
        payload[i] = rand() & 0xff;
    
    // assemble the frame
    flexframegen_assemble(fg, header, payload, _payload_len);
    if (liquid_autotest_verbose)
        flexframegen_print(fg);

    // generate the frame
    int frame_complete = 0;
    float complex buf[2];
    while (!frame_complete) {
        // write samples to buffer
        frame_complete = flexframegen_write_samples(fg, buf, 2);

        // run through frame synchronizer
        flexframesync_execute(fs, buf, 2);
    }

    // get frame data statistics
    framedatastats_s stats = flexframesync_get_framedatastats(fs);
    if (liquid_autotest_verbose)
        flexframesync_print(fs);

    // ensure callback was invoked
    CONTEND_EQUALITY(context, FRAMING_AUTOTEST_SECRET);

    // check to see that frame was recovered
    CONTEND_EQUALITY( stats.num_frames_detected, 1 );
    CONTEND_EQUALITY( stats.num_headers_valid,   1 );
    CONTEND_EQUALITY( stats.num_payloads_valid,  1 );
    CONTEND_EQUALITY( stats.num_bytes_received,  _payload_len );

    // destroy objects
    flexframegen_destroy(fg);
    flexframesync_destroy(fs);
}

