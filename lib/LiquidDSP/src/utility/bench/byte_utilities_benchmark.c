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

#include <stdlib.h>
#include <sys/resource.h>
#include "liquid.h"

void benchmark_count_ones(struct rusage     * _start,
                          struct rusage     * _finish,
                          unsigned long int * _num_iterations)
{
    // allocate buffer of bytes and initialize
    unsigned int i, j;
    unsigned int   buf_len = 1024;
    unsigned int * buf     = (unsigned int *) malloc(buf_len*sizeof(unsigned int));
    for (i=0; i<buf_len; i++)
        buf[i] = i & 0xff;

    // start trials
    unsigned int c = 0;
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        c &= 0xffff;
        for (j=0; j<buf_len; j++)
            c += liquid_count_ones(buf[j]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4*buf_len;
    *_num_iterations += c & 1; // trivial use of variable

    // clean allocated memory
    free(buf);
}

