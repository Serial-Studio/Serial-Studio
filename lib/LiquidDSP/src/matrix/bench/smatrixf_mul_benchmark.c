/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
void smatrixf_mul_bench(struct rusage *     _start,
                        struct rusage *     _finish,
                        unsigned long int * _num_iterations,
                        unsigned int        _n)
{
    // normalize number of iterations
    // time ~ _n ^ 3
    *_num_iterations = 1 + *_num_iterations * 8192 / (_n * _n * _n + 1);

    unsigned long int i;

    // generate random matrices
    smatrixf a = smatrixf_create(_n, _n);
    smatrixf b = smatrixf_create(_n, _n);
    smatrixf c = smatrixf_create(_n, _n);

    // number of random non-zero entries
    unsigned int nnz = _n / 20 < 4 ? 4 : _n / 20;

    // initialize _a
    for (i=0; i<nnz; i++) {
        unsigned int row = rand() % _n;
        unsigned int col = rand() % _n;
        float value      = randf();
        smatrixf_set(a, row, col, value);
    }
    
    // initialize _b
    for (i=0; i<nnz; i++) {
        unsigned int row = rand() % _n;
        unsigned int col = rand() % _n;
        float value      = randf();
        smatrixf_set(b, row, col, value);
    }

    // initialize c with first multiplication
    smatrixf_mul(a,b,c);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        smatrixf_mul(a,b,c);
        smatrixf_mul(a,b,c);
        smatrixf_mul(a,b,c);
        smatrixf_mul(a,b,c);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // free smatrix objects
    smatrixf_destroy(a);
    smatrixf_destroy(b);
    smatrixf_destroy(c);
}

#define SMATRIXF_MUL_BENCHMARK_API(M)   \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ smatrixf_mul_bench(_start, _finish, _num_iterations, M); }

void benchmark_smatrixf_mul_n32     SMATRIXF_MUL_BENCHMARK_API( 32)
void benchmark_smatrixf_mul_n64     SMATRIXF_MUL_BENCHMARK_API( 64)
void benchmark_smatrixf_mul_n128    SMATRIXF_MUL_BENCHMARK_API(128)
void benchmark_smatrixf_mul_n256    SMATRIXF_MUL_BENCHMARK_API(256)
void benchmark_smatrixf_mul_n512    SMATRIXF_MUL_BENCHMARK_API(512)

