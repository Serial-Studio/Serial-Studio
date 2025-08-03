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

#include <sys/resource.h>
#include "liquid.h"

#define FIRPFBCHR_EXECUTE_BENCH_API(M,P,m)                  \
(   struct rusage *     _start,                             \
    struct rusage *     _finish,                            \
    unsigned long int * _num_iterations)                    \
{ firpfbchr_crcf_execute_bench(_start, _finish, _num_iterations, M, P, m); }

// Helper function to keep code base small
void firpfbchr_crcf_execute_bench(struct rusage *     _start,
                                  struct rusage *     _finish,
                                  unsigned long int * _num_iterations,
                                  unsigned int        _M,
                                  unsigned int        _P,
                                  unsigned int        _m)
{
    // scale trials appropriately
    *_num_iterations = *_num_iterations * 4 / _M;
    if (*_num_iterations < 1) *_num_iterations = 1;

    // initialize channelizer
    float As         = 60.0f;
    firpfbchr_crcf q = firpfbchr_crcf_create_kaiser(_M,_P,_m,As);

    unsigned long int i;

    float complex x[_P];
    float complex y[_M];
    for (i=0; i<_P; i++)
        x[i] = randnf() + _Complex_I*randnf();

    // scale number of iterations to keep execution time
    // relatively linear
    unsigned long int n = (*_num_iterations * 20) / _M;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<n; i++) {
        firpfbchr_crcf_push(q, x);  firpfbchr_crcf_execute(q, y);
        firpfbchr_crcf_push(q, x);  firpfbchr_crcf_execute(q, y);
        firpfbchr_crcf_push(q, x);  firpfbchr_crcf_execute(q, y);
        firpfbchr_crcf_push(q, x);  firpfbchr_crcf_execute(q, y);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations = n*4;

    firpfbchr_crcf_destroy(q);
}

// analysis
void benchmark_firpfbchr_crcf_M0064_P0063  FIRPFBCHR_EXECUTE_BENCH_API(  64,   63, 4)
void benchmark_firpfbchr_crcf_M0128_P0127  FIRPFBCHR_EXECUTE_BENCH_API( 128,  127, 4)
void benchmark_firpfbchr_crcf_M0256_P0255  FIRPFBCHR_EXECUTE_BENCH_API( 256,  255, 4)
void benchmark_firpfbchr_crcf_M0512_P0511  FIRPFBCHR_EXECUTE_BENCH_API( 512,  511, 4)
void benchmark_firpfbchr_crcf_M1024_P1023  FIRPFBCHR_EXECUTE_BENCH_API(1024, 1023, 4)
void benchmark_firpfbchr_crcf_M2048_P2047  FIRPFBCHR_EXECUTE_BENCH_API(2048, 2047, 4)
void benchmark_firpfbchr_crcf_M4096_P4095  FIRPFBCHR_EXECUTE_BENCH_API(4096, 4095, 4)

