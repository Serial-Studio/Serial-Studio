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

// frequency modulator benchmark
void benchmark_freqmod(struct rusage *     _start,
                       struct rusage *     _finish,
                       unsigned long int * _num_iterations)
{
    // create modulator
    float   kf  = 0.05f; // modulation index
    freqmod mod = freqmod_create(kf);

    float         m[20];    // message signal
    float complex r[20];    // modulated signal

    unsigned long int i;

    // generate message signal (sum of sines)
    for (i=0; i<20; i++) {
        m[i] = 0.3f*cosf(2*M_PI*1*i/20.0f + 0.0f) +
               0.2f*cosf(2*M_PI*2*i/20.0f + 0.4f) +
               0.4f*cosf(2*M_PI*3*i/20.0f + 1.7f);
    }

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        freqmod_modulate(mod, m[ 0], &r[ 0]);
        freqmod_modulate(mod, m[ 1], &r[ 1]);
        freqmod_modulate(mod, m[ 2], &r[ 2]);
        freqmod_modulate(mod, m[ 3], &r[ 3]);
        freqmod_modulate(mod, m[ 4], &r[ 4]);
        freqmod_modulate(mod, m[ 5], &r[ 5]);
        freqmod_modulate(mod, m[ 6], &r[ 6]);
        freqmod_modulate(mod, m[ 7], &r[ 7]);
        freqmod_modulate(mod, m[ 8], &r[ 8]);
        freqmod_modulate(mod, m[ 9], &r[ 9]);
        freqmod_modulate(mod, m[10], &r[10]);
        freqmod_modulate(mod, m[11], &r[11]);
        freqmod_modulate(mod, m[12], &r[12]);
        freqmod_modulate(mod, m[13], &r[13]);
        freqmod_modulate(mod, m[14], &r[14]);
        freqmod_modulate(mod, m[15], &r[15]);
        freqmod_modulate(mod, m[16], &r[16]);
        freqmod_modulate(mod, m[17], &r[17]);
        freqmod_modulate(mod, m[18], &r[18]);
        freqmod_modulate(mod, m[19], &r[19]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 20;

    // destroy modulator
    freqmod_destroy(mod);
}


