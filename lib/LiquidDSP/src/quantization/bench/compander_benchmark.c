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

// 
void benchmark_compress_mulaw(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations)
{
    unsigned long int i;

    float x  = -0.1f;
    float mu = 255.0f;
    float y  = 0.0f;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        y += compress_mulaw(x,mu);
        y -= compress_mulaw(x,mu);
        x += compress_mulaw(y,mu);
        x -= compress_mulaw(y,mu);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
}

// 
void benchmark_expand_mulaw(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations)
{
    unsigned long int i;

    float x  = 0.0f;
    float mu = 255.0f;
    float y  = 0.75f;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        x += expand_mulaw(y,mu);
        x -= expand_mulaw(y,mu);
        y += expand_mulaw(x,mu);
        y -= expand_mulaw(x,mu);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
}

