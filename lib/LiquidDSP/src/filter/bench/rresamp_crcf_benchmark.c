/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
#include <stdlib.h>
#include "liquid.h"

// Helper function to keep code base small
void rresamp_crcf_bench(struct rusage *     _start,
                        struct rusage *     _finish,
                        unsigned long int * _num_iterations,
                        unsigned int        _P,
                        unsigned int        _Q)
{
    // adjust number of iterations
    *_num_iterations = *_num_iterations * liquid_nextpow2(_Q+1) / (4*_Q);

    // create resampling object
    unsigned int m  = 12;
    float        bw = 0.45f;
    float        As = 60.0f;
    rresamp_crcf q = rresamp_crcf_create_kaiser(_P,_Q,m,bw,As);

    // input/output buffers
    unsigned int buf_len = _P > _Q ? _P : _Q; // max(_P,_Q)
    float complex * buf = (float complex*) malloc(buf_len*sizeof(float complex));

    // initialize buffer
    unsigned long int i;
    for (i=0; i<buf_len; i++)
        buf[i] = i==0 ? 1.0 : 0.0;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        rresamp_crcf_execute(q, buf, buf);
        rresamp_crcf_execute(q, buf, buf);
        rresamp_crcf_execute(q, buf, buf);
        rresamp_crcf_execute(q, buf, buf);
        buf[0] = 1.0f;
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    free(buf);
    rresamp_crcf_destroy(q);
}

#define RRESAMP_CRCF_BENCHMARK_API(P,Q) \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ rresamp_crcf_bench(_start, _finish, _num_iterations, P, Q); }

//
// Rational-rate resampler benchmark prototypes; compare to arbitrary rate resampler
//
void benchmark_rresamp_crcf_P17_Q1   RRESAMP_CRCF_BENCHMARK_API(17,   1)
void benchmark_rresamp_crcf_P17_Q2   RRESAMP_CRCF_BENCHMARK_API(17,   2)
void benchmark_rresamp_crcf_P17_Q4   RRESAMP_CRCF_BENCHMARK_API(17,   4)
void benchmark_rresamp_crcf_P17_Q8   RRESAMP_CRCF_BENCHMARK_API(17,   8)
void benchmark_rresamp_crcf_P17_Q16  RRESAMP_CRCF_BENCHMARK_API(17,  16)
void benchmark_rresamp_crcf_P17_Q32  RRESAMP_CRCF_BENCHMARK_API(17,  32)
void benchmark_rresamp_crcf_P17_Q64  RRESAMP_CRCF_BENCHMARK_API(17,  64)
void benchmark_rresamp_crcf_P17_Q128 RRESAMP_CRCF_BENCHMARK_API(17, 128)
void benchmark_rresamp_crcf_P17_Q256 RRESAMP_CRCF_BENCHMARK_API(17, 256)

