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

#include <sys/resource.h>
#include "liquid.h"

// benchmark CVSD encoder
void benchmark_cvsd_encode(struct rusage *_start,
                           struct rusage *_finish,
                           unsigned long int *_num_iterations)
{
    unsigned long int i;

    // options
    unsigned int nbits=4;   // number of adjacent bits to observe
    float zeta=1.5f;        // slope adjustment multiplier
    float alpha = 0.95;     // pre-/post-filter coefficient
    char bit    = 0;        // output bit

    // create cvsd encoder
    cvsd encoder = cvsd_create(nbits, zeta, alpha);

    // input time series (random)
    float x[8] = {
       1.19403f,  -0.76765f,  -1.08415f,   0.65095f,
       0.11647f,  -0.80130f,  -0.87540f,  -0.14888f};

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        bit ^= cvsd_encode(encoder, x[0]);
        bit ^= cvsd_encode(encoder, x[2]);
        bit ^= cvsd_encode(encoder, x[3]);
        bit ^= cvsd_encode(encoder, x[3]);
        bit ^= cvsd_encode(encoder, x[4]);
        bit ^= cvsd_encode(encoder, x[5]);
        bit ^= cvsd_encode(encoder, x[6]);
        bit ^= cvsd_encode(encoder, x[7]);

        // randomize input
        x[i%8] += bit ? 0.1f : -0.1f;
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 8;

    // destroy cvsd encoder
    cvsd_destroy(encoder);
}

// benchmark CVSD decoder
void benchmark_cvsd_decode(struct rusage *_start,
                           struct rusage *_finish,
                           unsigned long int *_num_iterations)
{
    unsigned long int i;

    // options
    unsigned int nbits=4;   // number of adjacent bits to observe
    float zeta=1.5f;        // slope adjustment multiplier
    float alpha = 0.95;     // pre-/post-filter coefficient
    float x     = 0.0f;

    // create cvsd decoder
    cvsd decoder = cvsd_create(nbits, zeta, alpha);

    // input bit sequence (random)
    unsigned char b[8] = {1, 1, 1, 0, 1, 0, 0, 0};

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        x += cvsd_decode(decoder, b[0]);
        x += cvsd_decode(decoder, b[2]);
        x += cvsd_decode(decoder, b[3]);
        x += cvsd_decode(decoder, b[3]);
        x += cvsd_decode(decoder, b[4]);
        x += cvsd_decode(decoder, b[5]);
        x += cvsd_decode(decoder, b[6]);
        x += cvsd_decode(decoder, b[7]);

        // randomize input
        b[0] ^= (x > 0);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 8;

    // destroy cvsd decoder
    cvsd_destroy(decoder);
}

