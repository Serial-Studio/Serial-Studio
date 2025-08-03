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

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <math.h>
#include <assert.h>
#include "liquid.h"

// benchmark regular frame synchronizer with short frames; effectively
// test acquisition complexity
void benchmark_gmskframesync(struct rusage *     _start,
                             struct rusage *     _finish,
                             unsigned long int * _num_iterations)
{
    *_num_iterations /= 128;
    unsigned long int i;

    // options
    unsigned int k = 2;                 // samples/symbol
    unsigned int m = 3;                 // filter delay (symbols)
    float BT = 0.5f;                    // filter bandwidth-time product
    unsigned int payload_len = 8;       // length of payload (bytes)
    float SNRdB = 30.0f;                // SNR

    // derived values
    float nstd  = powf(10.0f, -SNRdB/20.0f);

    // create gmskframegen object and assemble the frame
    gmskframegen fg = gmskframegen_create_set(k, m, BT);
    gmskframegen_assemble(fg, NULL, NULL, payload_len,
            LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE);
    unsigned int frame_len = gmskframegen_getframelen(fg);
    float complex frame[frame_len];
    gmskframegen_write(fg, frame, frame_len);
    // add some noise
    for (i=0; i<frame_len; i++)
        frame[i] += nstd*(randnf() + _Complex_I*randnf());

    // create gmskframesync object
    gmskframesync fs = gmskframesync_create_set(k, m, BT, NULL, NULL);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        gmskframesync_execute(fs, frame, frame_len);
    }
    getrusage(RUSAGE_SELF, _finish);

    // destroy objects
    gmskframegen_destroy(fg);
    gmskframesync_destroy(fs);
}

// benchmark regular frame synchronizer with noise; essentially test
// complexity when no signal is present
void benchmark_gmskframesync_noise(struct rusage *     _start,
                                   struct rusage *     _finish,
                                   unsigned long int * _num_iterations)
{
    *_num_iterations /= 2000;
    unsigned long int i;

    // create frame synchronizer
    gmskframesync fs = gmskframesync_create_set(2, 3, 0.5f, NULL, NULL);

    // allocate memory for noise buffer and initialize
    unsigned int num_samples = 1024;
    float complex y[num_samples];
    for (i=0; i<num_samples; i++)
        y[i] = 0.01f*(randnf() + randnf()*_Complex_I)*M_SQRT1_2;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        // push samples through synchronizer
        gmskframesync_execute(fs, y, num_samples);
    }
    getrusage(RUSAGE_SELF, _finish);
    //gmskframesync_print(fs);

    // scale result by number of samples in buffer
    *_num_iterations *= num_samples;

    // destroy framing objects
    gmskframesync_destroy(fs);
}

