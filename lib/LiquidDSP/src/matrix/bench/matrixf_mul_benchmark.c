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
void matrixf_mul_bench(struct rusage *_start,
                       struct rusage *_finish,
                       unsigned long int *_num_iterations,
                       unsigned int _n)
{
    // normalize number of iterations
    // time ~ _n ^ 2
    *_num_iterations /= _n * _n;
    if (*_num_iterations < 1) *_num_iterations = 1;

    float a[_n*_n];
    float b[_n*_n];
    float c[_n*_n];
    unsigned int i;
    for (i=0; i<_n*_n; i++) {
        a[i] = randnf();
        b[i] = randnf();
    }
    
    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        matrixf_mul(a,_n,_n,  b,_n,_n,  c,_n,_n);
        matrixf_mul(a,_n,_n,  b,_n,_n,  c,_n,_n);
        matrixf_mul(a,_n,_n,  b,_n,_n,  c,_n,_n);
        matrixf_mul(a,_n,_n,  b,_n,_n,  c,_n,_n);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
}

#define MATRIXF_MUL_BENCHMARK_API(N)    \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ matrixf_mul_bench(_start, _finish, _num_iterations, N); }

void benchmark_matrixf_mul_n2      MATRIXF_MUL_BENCHMARK_API(2)
void benchmark_matrixf_mul_n4      MATRIXF_MUL_BENCHMARK_API(4)
void benchmark_matrixf_mul_n8      MATRIXF_MUL_BENCHMARK_API(8)
void benchmark_matrixf_mul_n16     MATRIXF_MUL_BENCHMARK_API(16)
void benchmark_matrixf_mul_n32     MATRIXF_MUL_BENCHMARK_API(32)
void benchmark_matrixf_mul_n64     MATRIXF_MUL_BENCHMARK_API(64)

