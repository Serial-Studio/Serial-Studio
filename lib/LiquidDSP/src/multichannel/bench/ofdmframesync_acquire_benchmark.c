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
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/resource.h>
#include "liquid.h"

#define OFDMFRAMESYNC_ACQUIRE_BENCH_API(M,CP_LEN)   \
(   struct rusage *_start,                          \
    struct rusage *_finish,                         \
    unsigned long int *_num_iterations)             \
{ ofdmframesync_acquire_bench(_start, _finish, _num_iterations, M, CP_LEN); }

// Helper function to keep code base small
void ofdmframesync_acquire_bench(struct rusage *_start,
                                 struct rusage *_finish,
                                 unsigned long int *_num_iterations,
                                 unsigned int _num_subcarriers,
                                 unsigned int _cp_len)
{
    // options
    unsigned int M         = _num_subcarriers;
    unsigned int cp_len    = _cp_len;
    unsigned int taper_len = 0;

    // derived values
    unsigned int num_samples = 3*(M + cp_len);

    // create synthesizer/analyzer objects
    ofdmframegen fg = ofdmframegen_create(M, cp_len, taper_len, NULL);
    //ofdmframegen_print(fg);

    ofdmframesync fs = ofdmframesync_create(M,cp_len,taper_len,NULL,NULL,NULL);

    unsigned int i;
    float complex y[num_samples];   // frame samples

    // assemble full frame
    unsigned int n=0;

    // write first S0 symbol
    ofdmframegen_write_S0a(fg, &y[n]);
    n += M + cp_len;

    // write second S0 symbol
    ofdmframegen_write_S0b(fg, &y[n]);
    n += M + cp_len;

    // write S1 symbol
    ofdmframegen_write_S1( fg, &y[n]);
    n += M + cp_len;

    assert(n == num_samples);

    // add noise
    for (i=0; i<num_samples; i++)
        y[i] += 0.02f*randnf()*cexpf(_Complex_I*2*M_PI*randf());

    // start trials
    *_num_iterations /= M*sqrtf(M);
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        //
        ofdmframesync_execute(fs,y,num_samples);

        //
        ofdmframesync_reset(fs);
    }
    getrusage(RUSAGE_SELF, _finish);
    //*_num_iterations *= 4;

    // destroy objects
    ofdmframegen_destroy(fg);
    ofdmframesync_destroy(fs);
}

//
void benchmark_ofdmframesync_acquire_n64    OFDMFRAMESYNC_ACQUIRE_BENCH_API(64, 8)
void benchmark_ofdmframesync_acquire_n128   OFDMFRAMESYNC_ACQUIRE_BENCH_API(128,16)
void benchmark_ofdmframesync_acquire_n256   OFDMFRAMESYNC_ACQUIRE_BENCH_API(256,32)
void benchmark_ofdmframesync_acquire_n512   OFDMFRAMESYNC_ACQUIRE_BENCH_API(512,64)

