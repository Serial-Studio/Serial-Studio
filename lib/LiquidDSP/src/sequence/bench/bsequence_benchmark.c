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

#include <sys/resource.h>
#include "liquid.h"

// Helper function to keep code base small
void bsequence_correlate_bench(struct rusage *_start,
                               struct rusage *_finish,
                               unsigned long int *_num_iterations,
                               unsigned int _n)
{
    // normalize number of iterations
    *_num_iterations *= 1000;
    *_num_iterations /= _n;
    if (*_num_iterations < 1) *_num_iterations = 1;

    // create and initialize binary sequences
    bsequence bs1 = bsequence_create(_n);
    bsequence bs2 = bsequence_create(_n);

    unsigned long int i;
    int rxy = 0;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        rxy += bsequence_correlate(bs1, bs2);
        rxy -= bsequence_correlate(bs1, bs2);
        rxy += bsequence_correlate(bs1, bs2);
        rxy -= bsequence_correlate(bs1, bs2);

        bsequence_push(rxy > 0 ? bs1 : bs2, 1);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // clean up memory
    bsequence_destroy(bs1);
    bsequence_destroy(bs2);
}

#define BSEQUENCE_BENCHMARK_API(N)          \
(   struct rusage *_start,                  \
    struct rusage *_finish,                 \
    unsigned long int *_num_iterations)     \
{ bsequence_correlate_bench(_start, _finish, _num_iterations, N); }

// 
void benchmark_bsequence_xcorr_n16      BSEQUENCE_BENCHMARK_API(16)
void benchmark_bsequence_xcorr_n64      BSEQUENCE_BENCHMARK_API(64)
void benchmark_bsequence_xcorr_n256     BSEQUENCE_BENCHMARK_API(256)
void benchmark_bsequence_xcorr_n1024    BSEQUENCE_BENCHMARK_API(1024)

