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

#define WINDOW_READ_BENCH_API(N)        \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ window_read_bench(_start, _finish, _num_iterations, N); }

// Helper function to keep code base small
void window_read_bench(struct rusage *_start,
                       struct rusage *_finish,
                       unsigned long int *_num_iterations,
                       unsigned int _n)
{
    // normalize number of iterations
    if (*_num_iterations < 1) *_num_iterations = 1;

    // initialize port
    windowcf w = windowcf_create(_n);

    unsigned long int i;

    float complex * r;

    // start trials:
    //   write to buffer, read from buffer
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        windowcf_push(w,1.0f);  windowcf_read(w, &r);
        windowcf_push(w,1.0f);  windowcf_read(w, &r);
        windowcf_push(w,1.0f);  windowcf_read(w, &r);
        windowcf_push(w,1.0f);  windowcf_read(w, &r);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    windowcf_destroy(w);
}

// 
void benchmark_windowcf_read_n16    WINDOW_READ_BENCH_API(16)
void benchmark_windowcf_read_n32    WINDOW_READ_BENCH_API(32)
void benchmark_windowcf_read_n64    WINDOW_READ_BENCH_API(64)
void benchmark_windowcf_read_n128   WINDOW_READ_BENCH_API(128)
void benchmark_windowcf_read_n256   WINDOW_READ_BENCH_API(256)

