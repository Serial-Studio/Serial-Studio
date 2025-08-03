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
void firdecim_crcf_bench(struct rusage *     _start,
                         struct rusage *     _finish,
                         unsigned long int * _num_iterations,
                         unsigned int        _M,
                         unsigned int        _h_len)
{
    // normalize number of iterations
    *_num_iterations /= _h_len;
    if (*_num_iterations < 1) *_num_iterations = 1;

    float h[_h_len];
    unsigned int i;
    for (i=0; i<_h_len; i++)
        h[i] = 1.0f;

    firdecim_crcf q = firdecim_crcf_create(_M,h,_h_len);

    // initialize input
    float complex x[_M];
    for (i=0; i<_M; i++)
        x[i] = (i%2) ? 1.0f : -1.0f;

    float complex y;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        firdecim_crcf_execute(q, x, &y);
        firdecim_crcf_execute(q, x, &y);
        firdecim_crcf_execute(q, x, &y);
        firdecim_crcf_execute(q, x, &y);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    firdecim_crcf_destroy(q);
}

#define FIRDECIM_CRCF_BENCHMARK_API(M,H_LEN)    \
(   struct rusage *_start,                      \
    struct rusage *_finish,                     \
    unsigned long int *_num_iterations)         \
{ firdecim_crcf_bench(_start, _finish, _num_iterations, M, H_LEN); }

void benchmark_firdecim_crcf_m2_h8     FIRDECIM_CRCF_BENCHMARK_API(2, 8)
void benchmark_firdecim_crcf_m4_h16    FIRDECIM_CRCF_BENCHMARK_API(4, 16)
void benchmark_firdecim_crcf_m8_h32    FIRDECIM_CRCF_BENCHMARK_API(8, 32)
void benchmark_firdecim_crcf_m16_h64   FIRDECIM_CRCF_BENCHMARK_API(16,64)
void benchmark_firdecim_cccf_m32_h128  FIRDECIM_CRCF_BENCHMARK_API(32,128)

