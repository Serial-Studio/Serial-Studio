/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
#include <math.h>
#include <sys/resource.h>
#include "liquid.internal.h"

// frequency demodulator benchmark
void benchmark_freqdem(struct rusage *     _start,
                       struct rusage *     _finish,
                       unsigned long int * _num_iterations)
{
    // create demodulator
    float   kf  = 0.05f; // modulation index
    freqdem dem = freqdem_create(kf);

    float complex r[20];    // modulated signal
    float         m[20];    // message signal

    unsigned long int i;

    // generate modulated signal
    for (i=0; i<20; i++)
        r[i] = 0.3f*cexpf(_Complex_I*2*M_PI*i/20.0f);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        freqdem_demodulate(dem, r[ 0], &m[ 0]);
        freqdem_demodulate(dem, r[ 1], &m[ 1]);
        freqdem_demodulate(dem, r[ 2], &m[ 2]);
        freqdem_demodulate(dem, r[ 3], &m[ 3]);
        freqdem_demodulate(dem, r[ 4], &m[ 4]);
        freqdem_demodulate(dem, r[ 5], &m[ 5]);
        freqdem_demodulate(dem, r[ 6], &m[ 6]);
        freqdem_demodulate(dem, r[ 7], &m[ 7]);
        freqdem_demodulate(dem, r[ 8], &m[ 8]);
        freqdem_demodulate(dem, r[ 9], &m[ 9]);
        freqdem_demodulate(dem, r[10], &m[10]);
        freqdem_demodulate(dem, r[11], &m[11]);
        freqdem_demodulate(dem, r[12], &m[12]);
        freqdem_demodulate(dem, r[13], &m[13]);
        freqdem_demodulate(dem, r[14], &m[14]);
        freqdem_demodulate(dem, r[15], &m[15]);
        freqdem_demodulate(dem, r[16], &m[16]);
        freqdem_demodulate(dem, r[17], &m[17]);
        freqdem_demodulate(dem, r[18], &m[18]);
        freqdem_demodulate(dem, r[19], &m[19]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 20;

    // destroy demodulator
    freqdem_destroy(dem);
}


