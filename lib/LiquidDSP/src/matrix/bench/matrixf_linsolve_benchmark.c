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

#include <stdlib.h>
#include <sys/resource.h>
#include "liquid.h"

// Helper function to keep code base small
void matrixf_linsolve_bench(struct rusage *     _start,
                            struct rusage *     _finish,
                            unsigned long int * _num_iterations,
                            unsigned int        _n)
{
    // normalize number of iterations
    // time ~ _n ^ 2
    *_num_iterations /= _n * _n;
    if (*_num_iterations < 1) *_num_iterations = 1;

    unsigned long int i;

    float A[_n*_n];
    float b[_n];
    float x[_n];
    for (i=0; i<_n*_n; i++)
        A[i] = randnf();
    for (i=0; i<_n; i++)
        b[i] = randnf();
    
    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        matrixf_linsolve(A,_n,b,x,NULL);
        matrixf_linsolve(A,_n,b,x,NULL);
        matrixf_linsolve(A,_n,b,x,NULL);
        matrixf_linsolve(A,_n,b,x,NULL);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
}

#define MATRIXF_LINSOLVE_BENCHMARK_API(N)   \
(   struct rusage *_start,                  \
    struct rusage *_finish,                 \
    unsigned long int *_num_iterations)     \
{ matrixf_linsolve_bench(_start, _finish, _num_iterations, N); }

void benchmark_matrixf_linsolve_n2      MATRIXF_LINSOLVE_BENCHMARK_API(2)
void benchmark_matrixf_linsolve_n4      MATRIXF_LINSOLVE_BENCHMARK_API(4)
void benchmark_matrixf_linsolve_n8      MATRIXF_LINSOLVE_BENCHMARK_API(8)
void benchmark_matrixf_linsolve_n16     MATRIXF_LINSOLVE_BENCHMARK_API(16)
void benchmark_matrixf_linsolve_n32     MATRIXF_LINSOLVE_BENCHMARK_API(32)
void benchmark_matrixf_linsolve_n64     MATRIXF_LINSOLVE_BENCHMARK_API(64)

