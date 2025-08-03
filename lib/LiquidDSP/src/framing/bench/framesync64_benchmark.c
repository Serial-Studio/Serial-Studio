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

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <math.h>
#include "liquid.h"

typedef struct {
    unsigned int num_frames_tx;         // number of transmitted frames
    unsigned int num_frames_detected;   // number of received frames (detected)
    unsigned int num_frames_valid;      // number of valid payloads
} framedata;

static int callback(unsigned char *  _header,
                    int              _header_valid,
                    unsigned char *  _payload,
                    unsigned int     _payload_len,
                    int              _payload_valid,
                    framesyncstats_s _stats,
                    void *           _userdata)
{
    framedata * fd = (framedata*) _userdata;
    fd->num_frames_detected += 1;
    fd->num_frames_valid    += _payload_valid ? 1 : 0;
    return 0;
}

// Helper function to keep code base small
void benchmark_framesync64(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations)
{
    *_num_iterations /= 128;
    unsigned long int i;

    framegen64 fg = framegen64_create();

    // frame data
    unsigned char header[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    unsigned char payload[64];
    // initialize payload
    for (i=0; i<64; i++)
        payload[i] = rand() & 0xff;
    framedata fd = {0, 0, 0};

    // create framesync64 object
    framesync64 fs = framesync64_create(callback,(void*)&fd);

    // generate the frame
    //unsigned int frame_len = framegen64_getframelen(fg);
    unsigned int frame_len = LIQUID_FRAME64_LEN;
    float complex frame[frame_len];
    framegen64_execute(fg, header, payload, frame);

    // add some noise
    for (i=0; i<frame_len; i++)
        frame[i] += 0.01f*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        framesync64_execute(fs, frame, frame_len);
    }
    getrusage(RUSAGE_SELF, _finish);

    framegen64_destroy(fg);
    framesync64_destroy(fs);
#if 0
    fd.num_frames_tx = *_num_iterations;
    printf("  frames detected/valid/transmitted  :   %6u / %6u / %6u\n",
            fd.num_frames_detected,
            fd.num_frames_valid,
            fd.num_frames_tx);
#endif
}

