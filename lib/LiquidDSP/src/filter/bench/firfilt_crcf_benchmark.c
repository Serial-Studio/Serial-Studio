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
void firfilt_crcf_bench(struct rusage *_start,
                        struct rusage *_finish,
                        unsigned long int *_num_iterations,
                        unsigned int _n)
{
    // adjust number of iterations:
    // cycles/trial ~ 107 + 4.3*_n
    *_num_iterations *= 1000;
    *_num_iterations /= (unsigned int)(107+4.3*_n);

    // generate coefficients
    float h[_n];
    unsigned long int i;
    for (i=0; i<_n; i++)
        h[i] = randnf();

    // create filter object
    firfilt_crcf f = firfilt_crcf_create(h,_n);

    // generate input vector
    float complex x[4];
    for (i=0; i<4; i++)
        x[i] = randnf() + _Complex_I*randnf();

    // output vector
    float complex y[4];

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        firfilt_crcf_push(f, x[0]); firfilt_crcf_execute(f, &y[0]);
        firfilt_crcf_push(f, x[1]); firfilt_crcf_execute(f, &y[1]);
        firfilt_crcf_push(f, x[2]); firfilt_crcf_execute(f, &y[2]);
        firfilt_crcf_push(f, x[3]); firfilt_crcf_execute(f, &y[3]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    firfilt_crcf_destroy(f);
}

#define FIRFILT_CRCF_BENCHMARK_API(N)   \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ firfilt_crcf_bench(_start, _finish, _num_iterations, N); }

void benchmark_firfilt_crcf_4    FIRFILT_CRCF_BENCHMARK_API(4)
void benchmark_firfilt_crcf_8    FIRFILT_CRCF_BENCHMARK_API(8)
void benchmark_firfilt_crcf_16   FIRFILT_CRCF_BENCHMARK_API(16)
void benchmark_firfilt_crcf_32   FIRFILT_CRCF_BENCHMARK_API(32)
void benchmark_firfilt_crcf_64   FIRFILT_CRCF_BENCHMARK_API(64)

