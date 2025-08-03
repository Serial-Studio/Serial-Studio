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

//
// fft_r2r_benchmark.h
//
// Real even/odd FFT benchmarks (discrete cosine/sine transforms)
//

#include <sys/resource.h>
#include "liquid.h"

#define LIQUID_FFT_R2R_BENCH_API(N,K)   \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ fft_r2r_bench(_start, _finish, _num_iterations, N, K); }

// Helper function to keep code base small
void fft_r2r_bench(struct rusage *_start,
                   struct rusage *_finish,
                   unsigned long int *_num_iterations,
                   unsigned int _n,
                   int _kind)
{
    // initialize arrays, plan
    float x[_n], y[_n];
    int _flags = 0;
    fftplan p = fft_create_plan_r2r_1d(_n, x, y, _kind, _flags);
    
    unsigned long int i;

    // initialize input with random values
    for (i=0; i<_n; i++)
        x[i] = randnf();

    // scale number of iterations to keep execution time
    // relatively linear
    *_num_iterations /= _n * _n;
    *_num_iterations *= 10;
    *_num_iterations += 1;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        fft_execute(p);
        fft_execute(p);
        fft_execute(p);
        fft_execute(p);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    fft_destroy_plan(p);
}

// Radix-2

void benchmark_fft_REDFT00_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_REDFT00)
void benchmark_fft_REDFT01_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_REDFT01)
void benchmark_fft_REDFT10_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_REDFT10)
void benchmark_fft_REDFT11_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_REDFT11)

void benchmark_fft_RODFT00_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_RODFT00)
void benchmark_fft_RODFT01_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_RODFT01)
void benchmark_fft_RODFT10_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_RODFT10)
void benchmark_fft_RODFT11_128  LIQUID_FFT_R2R_BENCH_API(128,  LIQUID_FFT_RODFT11)


// prime number

void benchmark_fft_REDFT00_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_REDFT00)
void benchmark_fft_REDFT01_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_REDFT01)
void benchmark_fft_REDFT10_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_REDFT10)
void benchmark_fft_REDFT11_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_REDFT11)

void benchmark_fft_RODFT00_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_RODFT00)
void benchmark_fft_RODFT01_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_RODFT01)
void benchmark_fft_RODFT10_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_RODFT10)
void benchmark_fft_RODFT11_127  LIQUID_FFT_R2R_BENCH_API(127,  LIQUID_FFT_RODFT11)

