/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
#include <string.h>
#include <sys/resource.h>

#include "liquid.internal.h"

// Helper function to keep code base small
void qdetector_cccf_bench(struct rusage *     _start,
                          struct rusage *     _finish,
                          unsigned long int * _num_iterations,
                          unsigned int        _n)
{
    // adjust number of iterations
    *_num_iterations *= 4;
    *_num_iterations /= _n;

    // generate sequence (random)
    float complex h[_n];
    unsigned long int i;
    for (i=0; i<_n; i++) {
        h[i] = (rand() % 2 ? 1.0f : -1.0f) +
               (rand() % 2 ? 1.0f : -1.0f)*_Complex_I;
    }

    // generate synchronizer
    int          ftype        = LIQUID_FIRFILT_ARKAISER;
    unsigned int k            =    2;   // samples/symbol
    unsigned int m            =    7;   // filter delay [symbols]
    float        beta         = 0.3f;   // excess bandwidth factor
    float        threshold    = 0.5f;   // threshold for detection
    float        range        = 0.05f;  // carrier offset search range [radians/sample]
    qdetector_cccf q = qdetector_cccf_create_linear(h, _n, ftype, k, m, beta);
    qdetector_cccf_set_threshold(q,threshold);
    qdetector_cccf_set_range    (q, range);

    // input sequence (random)
    float complex x[7];
    for (i=0; i<7; i++) {
        x[i] = (rand() % 2 ? 1.0f : -1.0f) +
               (rand() % 2 ? 1.0f : -1.0f)*_Complex_I;
    }

    // start trials
    getrusage(RUSAGE_SELF, _start);
    int detected = 0;
    for (i=0; i<(*_num_iterations); i++) {
        // push input sequence through synchronizer
        detected ^= qdetector_cccf_execute(q, x[0]) != NULL;
        detected ^= qdetector_cccf_execute(q, x[1]) != NULL;
        detected ^= qdetector_cccf_execute(q, x[2]) != NULL;
        detected ^= qdetector_cccf_execute(q, x[3]) != NULL;
        detected ^= qdetector_cccf_execute(q, x[4]) != NULL;
        detected ^= qdetector_cccf_execute(q, x[5]) != NULL;
        detected ^= qdetector_cccf_execute(q, x[6]) != NULL;

        // randomize input
        x[0] += detected > 2 ? -1e-3f : 1e-3f;
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 7;

    // clean up allocated objects
    qdetector_cccf_destroy(q);
}

#define QDETECTOR_CCCF_BENCHMARK_API(N)      \
(   struct rusage *     _start,             \
    struct rusage *     _finish,            \
    unsigned long int * _num_iterations)    \
{ qdetector_cccf_bench(_start, _finish, _num_iterations, N); }

void benchmark_qdetector_cccf_16   QDETECTOR_CCCF_BENCHMARK_API(16);
void benchmark_qdetector_cccf_32   QDETECTOR_CCCF_BENCHMARK_API(32);
void benchmark_qdetector_cccf_64   QDETECTOR_CCCF_BENCHMARK_API(64);
void benchmark_qdetector_cccf_128  QDETECTOR_CCCF_BENCHMARK_API(128);
void benchmark_qdetector_cccf_256  QDETECTOR_CCCF_BENCHMARK_API(256);
void benchmark_qdetector_cccf_512  QDETECTOR_CCCF_BENCHMARK_API(512);
void benchmark_qdetector_cccf_1024 QDETECTOR_CCCF_BENCHMARK_API(1024);
void benchmark_qdetector_cccf_2048 QDETECTOR_CCCF_BENCHMARK_API(2048);
void benchmark_qdetector_cccf_4096 QDETECTOR_CCCF_BENCHMARK_API(4096);
void benchmark_qdetector_cccf_8192 QDETECTOR_CCCF_BENCHMARK_API(8192);

