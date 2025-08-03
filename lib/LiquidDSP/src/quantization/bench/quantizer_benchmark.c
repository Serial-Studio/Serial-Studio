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
void benchmark_quantize_adc(struct rusage *     _start,
                            struct rusage *     _finish,
                            unsigned long int * _num_iterations)
{
    unsigned long int i;

    unsigned int q = 0;
    unsigned int num_bits=8;
    float x=-0.1f;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        q ^= quantize_adc(x,num_bits);
        q ^= quantize_adc(x,num_bits);
        q ^= quantize_adc(x,num_bits);
        q ^= quantize_adc(x,num_bits);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
    *_num_iterations += q & 1; // trivial use of variable
}

// 
void benchmark_quantize_dac(struct rusage *     _start,
                            struct rusage *     _finish,
                            unsigned long int * _num_iterations)
{
    unsigned long int i;

    unsigned int q=0x0f;
    unsigned int num_bits=8;
    float x = 0;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        x += quantize_dac(q,num_bits);
        x += quantize_dac(q,num_bits);
        x += quantize_dac(q,num_bits);
        x += quantize_dac(q,num_bits);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
    *_num_iterations += x > 0; // trivial use of variable
}

