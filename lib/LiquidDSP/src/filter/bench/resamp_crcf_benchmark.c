/*
 * Copyright (c) 2007 - 2018 Joseph Gaeddert
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
#include <math.h>
#include "liquid.h"

// Helper function to keep code base small
void resamp_crcf_bench(struct rusage *     _start,
                       struct rusage *     _finish,
                       unsigned long int * _num_iterations,
                       unsigned int        _P,
                       unsigned int        _Q)
{
    // adjust number of iterations: cycles/trial ~ 500 + 100 Q
    *_num_iterations /= (500 + 100*_Q);

    // create resampling object; irrational rate is just less than Q/P
    float        rate = (float)_Q/(float)_P*sqrt(3301.0f/3302.0f);
    unsigned int m    = 12;     // filter semi-length
    float        bw   = 0.45f;  // filter bandwidth
    float        As   = 60.0f;  // stop-band attenuation [dB]
    unsigned int npfb = 64;     // number of polyphase filters
    resamp_crcf q = resamp_crcf_create(rate,m,bw,As,npfb);

    // buffering
    float complex buf_0[_P];
    float complex buf_1[_Q*4];
    unsigned int num_written;
    
    unsigned long int i;
    for (i=0; i<_P; i++)
        buf_0[i] = i % 7 ? 1 : -1;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        resamp_crcf_execute_block(q, buf_0, _P, buf_1, &num_written);
        resamp_crcf_execute_block(q, buf_0, _P, buf_1, &num_written);
        resamp_crcf_execute_block(q, buf_0, _P, buf_1, &num_written);
        resamp_crcf_execute_block(q, buf_0, _P, buf_1, &num_written);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // destroy object
    resamp_crcf_destroy(q);
}

#define RESAMP_CRCF_BENCHMARK_API(P,Q)  \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ resamp_crcf_bench(_start, _finish, _num_iterations, P, Q); }

//
// Resampler benchmark prototypes; compare to rational rate resampler
//
void benchmark_resamp_crcf_P17_Q1   RESAMP_CRCF_BENCHMARK_API(17,   1)
void benchmark_resamp_crcf_P17_Q2   RESAMP_CRCF_BENCHMARK_API(17,   2)
void benchmark_resamp_crcf_P17_Q4   RESAMP_CRCF_BENCHMARK_API(17,   4)
void benchmark_resamp_crcf_P17_Q8   RESAMP_CRCF_BENCHMARK_API(17,   8)
void benchmark_resamp_crcf_P17_Q16  RESAMP_CRCF_BENCHMARK_API(17,  16)
void benchmark_resamp_crcf_P17_Q32  RESAMP_CRCF_BENCHMARK_API(17,  32)
void benchmark_resamp_crcf_P17_Q64  RESAMP_CRCF_BENCHMARK_API(17,  64)
void benchmark_resamp_crcf_P17_Q128 RESAMP_CRCF_BENCHMARK_API(17, 128)
void benchmark_resamp_crcf_P17_Q256 RESAMP_CRCF_BENCHMARK_API(17, 256)

