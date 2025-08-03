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

// Helper function to keep code base small
void polyf_fit_bench(struct rusage *_start,
                     struct rusage *_finish,
                     unsigned long int *_num_iterations,
                     unsigned int _Q,
                     unsigned int _N)
{
    // normalize number of iterations
    // time ~ 0.2953 + 0.03381 * _N
    *_num_iterations /= 0.2953 + 0.03381 * _N * 40;
    if (*_num_iterations < 1) *_num_iterations = 1;

    float p[_Q+1];

    float x[_N];
    float y[_N];
    unsigned int i;
    for (i=0; i<_N; i++) {
        x[i] = randnf();
        y[i] = randnf();
    }
    
    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        polyf_fit(x,y,_N, p,_Q+1);
        polyf_fit(x,y,_N, p,_Q+1);
        polyf_fit(x,y,_N, p,_Q+1);
        polyf_fit(x,y,_N, p,_Q+1);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
}

#define POLYF_FIT_BENCHMARK_API(Q,N)    \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ polyf_fit_bench(_start, _finish, _num_iterations, Q, N); }

void benchmark_polyf_fit_q3_n8      POLYF_FIT_BENCHMARK_API(3, 8)
void benchmark_polyf_fit_q3_n16     POLYF_FIT_BENCHMARK_API(3, 16)
void benchmark_polyf_fit_q3_n32     POLYF_FIT_BENCHMARK_API(3, 32)
void benchmark_polyf_fit_q3_n64     POLYF_FIT_BENCHMARK_API(3, 64)
void benchmark_polyf_fit_q3_n128    POLYF_FIT_BENCHMARK_API(3, 128)

