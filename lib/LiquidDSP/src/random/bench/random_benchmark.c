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

#include <sys/resource.h>
#include "liquid.h"

// 
// BENCHMARK: uniform
//
void benchmark_random_uniform(struct rusage *_start,
                              struct rusage *_finish,
                              unsigned long int *_num_iterations)
{
    // normalize number of iterations
    *_num_iterations *= 10;

    float x = 0.0f;
    unsigned long int i;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        x += randf();
        x += randf();
        x += randf();
        x += randf();
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
    *_num_iterations += x > 0; // trivial use of variable
}

// 
// BENCHMARK: normal
//
void benchmark_random_normal(struct rusage *_start,
                             struct rusage *_finish,
                             unsigned long int *_num_iterations)
{
    // normalize number of iterations
    *_num_iterations *= 1;

    float x = 0.0f;
    unsigned long int i;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        x += randnf();
        x += randnf();
        x += randnf();
        x += randnf();
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
    *_num_iterations += x > 0; // trivial use of variable
}

// 
// BENCHMARK: complex normal
//
void benchmark_random_complex_normal(struct rusage *_start,
                                     struct rusage *_finish,
                                     unsigned long int *_num_iterations)
{
    // normalize number of iterations
    *_num_iterations /= 2;

    float complex x = 0.0f;
    unsigned long int i;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        crandnf(&x);
        crandnf(&x);
        crandnf(&x);
        crandnf(&x);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
    *_num_iterations += crealf(x) > 0; // trivial use of variable
}

// 
// BENCHMARK: Weibull
//
void benchmark_random_weibull(struct rusage *_start,
                              struct rusage *_finish,
                              unsigned long int *_num_iterations)
{
    // normalize number of iterations
    *_num_iterations *= 2;

    float x=0.0f;
    float alpha=1.0f;
    float beta=2.0f;
    float gamma=6.0f;
    unsigned long int i;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        x += randweibf(alpha,beta,gamma);
        x += randweibf(alpha,beta,gamma);
        x += randweibf(alpha,beta,gamma);
        x += randweibf(alpha,beta,gamma);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
    *_num_iterations += x > 0; // trivial use of variable
}

// 
// BENCHMARK: Rice-K
//
void benchmark_random_ricek(struct rusage *_start,
                            struct rusage *_finish,
                            unsigned long int *_num_iterations)
{
    // normalize number of iterations
    *_num_iterations /= 3;

    float x = 0.0f;
    float K=2.0f;
    float omega=1.0f;
    unsigned long int i;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        x += randricekf(K,omega);
        x += randricekf(K,omega);
        x += randricekf(K,omega);
        x += randricekf(K,omega);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
    *_num_iterations += x > 0; // trivial use of variable
}

