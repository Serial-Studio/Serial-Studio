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
#include <string.h>
#include <math.h>
#include "autotest/autotest.h"
#include "liquid.h"

// AUTOTEST : test simple recovery of frame in noise
void autotest_fskframesync()
{
    // options
    float SNRdB       =  20.0f; // signal-to-noise ratio
    float noise_floor = -20.0f; // noise floor
    //float dphi        =  0.01f; // carrier frequency offset
    //float theta       =  0.0f;  // carrier phase offset
    //float dt          =  -0.2f;  // fractional sample timing offset

    crc_scheme check         = LIQUID_CRC_32;   // data validity check
    fec_scheme fec0          = LIQUID_FEC_NONE; // fec (inner)
    fec_scheme fec1          = LIQUID_FEC_NONE; // fec (outer)

    // derived values
    float nstd  = powf(10.0f, noise_floor/20.0f);         // noise std. dev.
    float gamma = powf(10.0f, (SNRdB+noise_floor)/20.0f); // channel gain

    unsigned int i;
    unsigned int context = 0;

    // create objects
    fskframegen fg = fskframegen_create();
    fskframesync fs = fskframesync_create(framing_autotest_callback, (void*)&context);

    // assemble the frame
    unsigned char header [  8];
    unsigned char payload[200];
    for (i=0; i<  8; i++) header [i] = i;
    for (i=0; i<200; i++) payload[i] = rand() & 0xff;
    fskframegen_assemble(fg, header, payload, 200, check, fec0, fec1);

    // allocate memory for the frame samples
    unsigned int  buf_len = 256;
    float complex buf_tx[buf_len];  // receive buffer
    float complex buf_rx[buf_len];  // transmit buffer
    
    // try to receive the frame (operate in blocks)
    int frame_complete = 0;
    while (!frame_complete)
    {
        // generate frame samples
        frame_complete = fskframegen_write_samples(fg, buf_tx, buf_len);

        // add noise, channel gain
        for (i=0; i<buf_len; i++)
            buf_rx[i] = buf_tx[i]*gamma + nstd*(randnf() + randnf()*_Complex_I)*M_SQRT1_2;

        // synchronize/receive the frame
        fskframesync_execute_block(fs, buf_rx, buf_len);
    }

    // check to see that callback was invoked
    CONTEND_EQUALITY(context, FRAMING_AUTOTEST_SECRET);

#if 0
    // parse statistics
    framedatastats_s stats = fskframesync_get_framedatastats(fs);
    CONTEND_EQUALITY(stats.num_frames_detected, 1);
    CONTEND_EQUALITY(stats.num_headers_valid,   1);
    CONTEND_EQUALITY(stats.num_payloads_valid,  1);
    CONTEND_EQUALITY(stats.num_bytes_received, 64);
#endif

    // destroy objects
    fskframegen_destroy(fg);
    fskframesync_destroy(fs);
}

