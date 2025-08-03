/*
 * Copyright (c) 2007 - 2018 Joseph Gaeddert
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
void bpresync_cccf_bench(struct rusage *     _start,
                         struct rusage *     _finish,
                         unsigned long int * _num_iterations,
                         unsigned int        _n,
                         unsigned int        _m)
{
    // adjust number of iterations
    *_num_iterations *= 4;
    *_num_iterations /= _n;
    *_num_iterations /= _m;

    // generate sequence (random)
    float complex h[_n];
    unsigned long int i;
    for (i=0; i<_n; i++) {
        h[i] = (rand() % 2 ? 1.0f : -1.0f) +
               (rand() % 2 ? 1.0f : -1.0f)*_Complex_I;
    }

    // generate synchronizer
    bpresync_cccf q = bpresync_cccf_create(h, _n, 0.1f, _m);

    // input sequence (random)
    float complex x[7];
    for (i=0; i<7; i++) {
        x[i] = (rand() % 2 ? 1.0f : -1.0f) +
               (rand() % 2 ? 1.0f : -1.0f)*_Complex_I;
    }

    float complex rxy;
    float dphi_hat;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        // push input sequence through synchronizer
        bpresync_cccf_push(q, x[0]);  bpresync_cccf_execute(q, &rxy, &dphi_hat);
        bpresync_cccf_push(q, x[1]);  bpresync_cccf_execute(q, &rxy, &dphi_hat);
        bpresync_cccf_push(q, x[2]);  bpresync_cccf_execute(q, &rxy, &dphi_hat);
        bpresync_cccf_push(q, x[3]);  bpresync_cccf_execute(q, &rxy, &dphi_hat);
        bpresync_cccf_push(q, x[4]);  bpresync_cccf_execute(q, &rxy, &dphi_hat);
        bpresync_cccf_push(q, x[5]);  bpresync_cccf_execute(q, &rxy, &dphi_hat);
        bpresync_cccf_push(q, x[6]);  bpresync_cccf_execute(q, &rxy, &dphi_hat);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 7;

    // clean up allocated objects
    bpresync_cccf_destroy(q);
}

#define BPRESYNC_CCCF_BENCHMARK_API(N,M)    \
(   struct rusage *     _start,             \
    struct rusage *     _finish,            \
    unsigned long int * _num_iterations)    \
{ bpresync_cccf_bench(_start, _finish, _num_iterations, N, M); }

void benchmark_bpresync_cccf_16   BPRESYNC_CCCF_BENCHMARK_API(16,   6);
void benchmark_bpresync_cccf_32   BPRESYNC_CCCF_BENCHMARK_API(32,   6);
void benchmark_bpresync_cccf_64   BPRESYNC_CCCF_BENCHMARK_API(64,   6);
void benchmark_bpresync_cccf_128  BPRESYNC_CCCF_BENCHMARK_API(128,  6);
void benchmark_bpresync_cccf_256  BPRESYNC_CCCF_BENCHMARK_API(256,  6);

