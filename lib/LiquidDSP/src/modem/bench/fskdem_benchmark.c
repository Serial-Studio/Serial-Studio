/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
#include <math.h>
#include <sys/resource.h>
#include "liquid.internal.h"

#define FSKDEM_BENCH_API(m,k,bandwidth)     \
(   struct rusage *     _start,             \
    struct rusage *     _finish,            \
    unsigned long int * _num_iterations)    \
{ fskdem_bench(_start, _finish, _num_iterations, m, k, bandwidth); }

// Helper function to keep code base small
void fskdem_bench(struct rusage *     _start,
                  struct rusage *     _finish,
                  unsigned long int * _num_iterations,
                  unsigned int        _m,
                  unsigned int        _k,
                  float               _bandwidth)
{
    // normalize number of iterations
    *_num_iterations /= _k;

    if (*_num_iterations < 1) *_num_iterations = 1;

    // initialize demodulator
    fskdem dem = fskdem_create(_m,_k,_bandwidth);

    //unsigned int M = 1 << _m;   // constellation size
    
    unsigned long int i;

    // generate input vector to demodulate (spiral)
    float complex buf[_k+10];
    for (i=0; i<_k+10; i++)
        buf[i] = 0.07 * i * cexpf(_Complex_I*2*M_PI*0.1*i);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        fskdem_demodulate(dem, &buf[0]);
        fskdem_demodulate(dem, &buf[1]);
        fskdem_demodulate(dem, &buf[2]);
        fskdem_demodulate(dem, &buf[3]);
        fskdem_demodulate(dem, &buf[4]);
        fskdem_demodulate(dem, &buf[5]);
        fskdem_demodulate(dem, &buf[6]);
        fskdem_demodulate(dem, &buf[7]);
        fskdem_demodulate(dem, &buf[8]);
        fskdem_demodulate(dem, &buf[9]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 10;

    fskdem_destroy(dem);
}

// BENCHMARKS: basic properties: M=2^m, k = 2*M, bandwidth = 0.25
void benchmark_fskdem_norm_M2      FSKDEM_BENCH_API( 1,    4, 0.25f    )
void benchmark_fskdem_norm_M4      FSKDEM_BENCH_API( 2,    8, 0.25f    )
void benchmark_fskdem_norm_M8      FSKDEM_BENCH_API( 3,   16, 0.25f    )
void benchmark_fskdem_norm_M16     FSKDEM_BENCH_API( 4,   32, 0.25f    )
void benchmark_fskdem_norm_M32     FSKDEM_BENCH_API( 5,   64, 0.25f    )
void benchmark_fskdem_norm_M64     FSKDEM_BENCH_API( 6,  128, 0.25f    )
void benchmark_fskdem_norm_M128    FSKDEM_BENCH_API( 7,  256, 0.25f    )
void benchmark_fskdem_norm_M256    FSKDEM_BENCH_API( 8,  512, 0.25f    )
void benchmark_fskdem_norm_M512    FSKDEM_BENCH_API( 9, 1024, 0.25f    )
void benchmark_fskdem_norm_M1024   FSKDEM_BENCH_API(10, 2048, 0.25f    )

// BENCHMARKS: obscure properties: M=2^m, k not relative to M, bandwidth basically irrational
void benchmark_fskdem_misc_M2      FSKDEM_BENCH_API( 1,    5, 0.3721451)
void benchmark_fskdem_misc_M4      FSKDEM_BENCH_API( 2,   10, 0.3721451)
void benchmark_fskdem_misc_M8      FSKDEM_BENCH_API( 3,   20, 0.3721451)
void benchmark_fskdem_misc_M16     FSKDEM_BENCH_API( 4,   30, 0.3721451)
void benchmark_fskdem_misc_M32     FSKDEM_BENCH_API( 5,   60, 0.3721451)
void benchmark_fskdem_misc_M64     FSKDEM_BENCH_API( 6,  100, 0.3721451)
void benchmark_fskdem_misc_M128    FSKDEM_BENCH_API( 7,  200, 0.3721451)
void benchmark_fskdem_misc_M256    FSKDEM_BENCH_API( 8,  500, 0.3721451)
void benchmark_fskdem_misc_M512    FSKDEM_BENCH_API( 9, 1000, 0.3721451)
void benchmark_fskdem_misc_M1024   FSKDEM_BENCH_API(10, 2000, 0.3721451)

