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

void autotest_dsssframesync()
{
    unsigned int _payload_len = 400;

    // create dsssframegen object
    dsssframegenprops_s fgprops;
    fgprops.check = LIQUID_CRC_32;
    fgprops.fec0  = LIQUID_FEC_NONE;
    fgprops.fec1  = LIQUID_FEC_NONE;
    dsssframegen fg = dsssframegen_create(&fgprops);
    
    // assemble the frame
    dsssframegen_assemble(fg, NULL, NULL, _payload_len);

    // create dsssframesync object
    unsigned int context = 0;
    dsssframesync fs = dsssframesync_create(framing_autotest_callback, (void*)&context);

    // generate the frame
    int frame_complete = 0;
    unsigned int buf_len = 256;
    float complex buf[buf_len];
    while (!frame_complete) {
        // write samples to buffer
        frame_complete = dsssframegen_write_samples(fg, buf, buf_len);

        // run through frame synchronizer
        dsssframesync_execute(fs, buf, buf_len);
    }

    // get frame data statistics
    if (liquid_autotest_verbose)
        dsssframesync_print(fs);

    // check to see that frame was recovered
#if 0
    framedatastats_s stats = dsssframesync_get_framedatastats(fs);
    CONTEND_EQUALITY( stats.num_frames_detected, 1 );
    CONTEND_EQUALITY( stats.num_headers_valid,   1 );
    CONTEND_EQUALITY( stats.num_payloads_valid,  1 );
    CONTEND_EQUALITY( stats.num_bytes_received,  _payload_len );
#endif
    CONTEND_EQUALITY(context, FRAMING_AUTOTEST_SECRET);

    // destroy objects
    dsssframegen_destroy(fg);
    dsssframesync_destroy(fs);
}

