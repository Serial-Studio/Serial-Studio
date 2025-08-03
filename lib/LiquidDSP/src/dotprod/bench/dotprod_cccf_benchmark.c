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
void dotprod_cccf_bench(struct rusage *_start,
                        struct rusage *_finish,
                        unsigned long int *_num_iterations,
                        unsigned int _n)
{
    // normalize number of iterations
    *_num_iterations = *_num_iterations * 20 / _n;
    if (*_num_iterations < 1) *_num_iterations = 1;

    float complex x[_n];
    float complex h[_n];
    float complex y[8];
    unsigned int i;
    for (i=0; i<_n; i++) {
        x[i] = randnf() + _Complex_I*randnf();
        h[i] = randnf() + _Complex_I*randnf();
    }

    // create dotprod structure;
    dotprod_cccf dp = dotprod_cccf_create(h,_n);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        dotprod_cccf_execute(dp, x, &y[0]);
        dotprod_cccf_execute(dp, x, &y[1]);
        dotprod_cccf_execute(dp, x, &y[2]);
        dotprod_cccf_execute(dp, x, &y[3]);
        dotprod_cccf_execute(dp, x, &y[4]);
        dotprod_cccf_execute(dp, x, &y[5]);
        dotprod_cccf_execute(dp, x, &y[6]);
        dotprod_cccf_execute(dp, x, &y[7]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 8;

    // clean up objects
    dotprod_cccf_destroy(dp);
}

#define DOTPROD_CCCF_BENCHMARK_API(N)   \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ dotprod_cccf_bench(_start, _finish, _num_iterations, N); }

void benchmark_dotprod_cccf_4      DOTPROD_CCCF_BENCHMARK_API(4)
void benchmark_dotprod_cccf_16     DOTPROD_CCCF_BENCHMARK_API(16)
void benchmark_dotprod_cccf_64     DOTPROD_CCCF_BENCHMARK_API(64)
void benchmark_dotprod_cccf_256    DOTPROD_CCCF_BENCHMARK_API(256)

