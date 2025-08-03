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
void firhilbf_decim_bench(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations,
    unsigned int _m)
{
    // normalize number of trials
    *_num_iterations *= 20;
    *_num_iterations /= liquid_nextpow2(_m+1);

    // create hilber transform object
    firhilbf q = firhilbf_create(_m,60.0f);

    float x[] = {1.0f, -1.0f};
    float complex y;
    unsigned long int i;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        firhilbf_decim_execute(q,x,&y);
        firhilbf_decim_execute(q,x,&y);
        firhilbf_decim_execute(q,x,&y);
        firhilbf_decim_execute(q,x,&y);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    firhilbf_destroy(q);
}

#define FIRHILB_DECIM_BENCHMARK_API(M)  \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ firhilbf_decim_bench(_start, _finish, _num_iterations, M); }

void benchmark_firhilbf_decim_m3    FIRHILB_DECIM_BENCHMARK_API(3)  // m=3
void benchmark_firhilbf_decim_m5    FIRHILB_DECIM_BENCHMARK_API(5)  // m=5
void benchmark_firhilbf_decim_m9    FIRHILB_DECIM_BENCHMARK_API(9)  // m=9
void benchmark_firhilbf_decim_m13   FIRHILB_DECIM_BENCHMARK_API(13) // m=13

