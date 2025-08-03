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

//
// Example of a benchmark header
//

#include <sys/resource.h>
#include <math.h>

// strings parsed by benchmarkgen.py
const char * mybench_opts[3] = {
    "opt1a opt1b",
    "opt2a opt2b opt2c",
    "opt3a opt3b opt3c"
};


void benchmark_mybench(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations)
//    unsigned int argc,
//    char *argv[])
{
    // DSP initiazation goes here

    unsigned int i;
    float x, y, theta;
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        // DSP execution goes here
        x = cosf(M_PI/2.0f);
        y = sinf(M_PI/2.0f);
        theta = atan2(y,x);
    }
    getrusage(RUSAGE_SELF, _finish);

    // DSP cleanup goes here
}


