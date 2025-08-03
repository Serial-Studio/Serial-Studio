/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
#include <sys/resource.h>
#include <assert.h>
#include "liquid.h"

// Helper function to keep code base small
void benchmark_flexframesync(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations)
{
    *_num_iterations /= 128;
    unsigned long int i;

    // create flexframegen object
    flexframegenprops_s fgprops;
    flexframegenprops_init_default(&fgprops);
    fgprops.check      = LIQUID_CRC_32;
    fgprops.fec0       = LIQUID_FEC_NONE;
    fgprops.fec1       = LIQUID_FEC_NONE;
    fgprops.mod_scheme = LIQUID_MODEM_QPSK;
    flexframegen fg = flexframegen_create(&fgprops);

    // generate the frame
    unsigned int payload_len = 8;
    unsigned char header[14];
    unsigned char payload[payload_len];
    flexframegen_assemble(fg, header, payload, payload_len);
    unsigned int frame_len = flexframegen_getframelen(fg);
    float complex frame[frame_len];
    flexframegen_write_samples(fg, frame, frame_len);

    // add some noise
    for (i=0; i<frame_len; i++)
        frame[i] += 0.02f*(randnf() + _Complex_I*randnf());

    // create flexframesync object
    flexframesync fs = flexframesync_create(NULL, NULL);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        flexframesync_execute(fs, frame, frame_len);
    }
    getrusage(RUSAGE_SELF, _finish);

    // print frame data statistics
    //flexframesync_print(fs);

    // destroy objects
    flexframegen_destroy(fg);
    flexframesync_destroy(fs);
}

