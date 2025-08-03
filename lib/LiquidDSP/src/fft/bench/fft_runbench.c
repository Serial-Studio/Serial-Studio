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
// fft_runbench.c : benchmark execution program
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include "liquid.h"

// Helper function to keep code base small
void fft_runbench(struct rusage *     _start,
                  struct rusage *     _finish,
                  unsigned long int * _num_iterations,
                  unsigned int        _nfft,
                  int                 _direction)
{
    // initialize arrays, plan
    float complex * x = (float complex *) fft_malloc(_nfft*sizeof(float complex));
    float complex * y = (float complex *) fft_malloc(_nfft*sizeof(float complex));
    int _method = 0;
    fftplan q = fft_create_plan(_nfft, x, y, _direction, _method);
    
    unsigned long int i;

    // initialize input with random values
    for (i=0; i<_nfft; i++)
        x[i] = randnf() + randnf()*_Complex_I;

    // scale number of iterations to keep execution time
    // relatively linear
    *_num_iterations /= _nfft;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        fft_execute(q);
        fft_execute(q);
        fft_execute(q);
        fft_execute(q);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    fft_destroy_plan(q);
    fft_free(x);
    fft_free(y);
}

