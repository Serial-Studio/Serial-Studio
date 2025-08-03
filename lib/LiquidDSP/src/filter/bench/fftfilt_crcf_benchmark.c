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
#include <math.h>
#include "liquid.h"

// Helper function to keep code base small
void fftfilt_crcf_bench(struct rusage *     _start,
                        struct rusage *     _finish,
                        unsigned long int * _num_iterations,
                        unsigned int        _n)
{
    // adjust number of iterations:
    *_num_iterations = *_num_iterations * 5 / (_n*logf(_n));
    if (*_num_iterations < 1) *_num_iterations = 1;

    // generate coefficients
    unsigned int h_len = _n+1;
    float h[h_len];
    unsigned long int i;
    for (i=0; i<h_len; i++)
        h[i] = randnf();

    // create filter object
    fftfilt_crcf q = fftfilt_crcf_create(h,h_len,_n);

    // generate input vector
    float complex x[_n + 4];
    for (i=0; i<_n+4; i++)
        x[i] = randnf() + _Complex_I*randnf();

    // output vector
    float complex y[_n];

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        fftfilt_crcf_execute(q, &x[0], y);
        fftfilt_crcf_execute(q, &x[1], y);
        fftfilt_crcf_execute(q, &x[2], y);
        fftfilt_crcf_execute(q, &x[3], y);
    }
    getrusage(RUSAGE_SELF, _finish);

    // scale number of iterations: loop unrolled 4 times, _n samples/block
    *_num_iterations *= 4 * _n;

    // destroy filter object
    fftfilt_crcf_destroy(q);
}

#define FFTFILT_CRCF_BENCHMARK_API(N)   \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ fftfilt_crcf_bench(_start, _finish, _num_iterations, N); }

void benchmark_fftfilt_crcf_4    FFTFILT_CRCF_BENCHMARK_API(4)
void benchmark_fftfilt_crcf_8    FFTFILT_CRCF_BENCHMARK_API(8)
void benchmark_fftfilt_crcf_16   FFTFILT_CRCF_BENCHMARK_API(16)
void benchmark_fftfilt_crcf_32   FFTFILT_CRCF_BENCHMARK_API(32)
void benchmark_fftfilt_crcf_64   FFTFILT_CRCF_BENCHMARK_API(64)

