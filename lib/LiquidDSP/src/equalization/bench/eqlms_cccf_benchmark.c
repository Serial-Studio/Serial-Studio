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

#include <stdlib.h>
#include <math.h>
#include <sys/resource.h>
#include "liquid.h"

#define EQLMS_CCCF_TRAIN_BENCH_API(N)   \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ eqlms_cccf_train_bench(_start, _finish, _num_iterations, N); }

// Helper function to keep code base small
void eqlms_cccf_train_bench(struct rusage *_start,
                            struct rusage *_finish,
                            unsigned long int *_num_iterations,
                            unsigned int _h_len)
{
    // scale number of iterations appropriately
    // log(cycles/trial) ~ 5.63 + 0.767*log(_h_len)
    *_num_iterations *= 3200;
    *_num_iterations /= (unsigned int) expf(5.63f + 0.767f*logf(_h_len));
    *_num_iterations = (*_num_iterations < 4) ? 4 : *_num_iterations;

    eqlms_cccf eq = eqlms_cccf_create(NULL,_h_len);
    
    unsigned long int i;

    // set up initial arrays to 'randomize' inputs/outputs
    float complex y[11];
    for (i=0; i<11; i++)
        y[i] = randnf() + _Complex_I*randnf();

    float complex d[13];
    for (i=0; i<13; i++)
        d[i] = randnf() + _Complex_I*randnf();

    unsigned int iy=0;
    unsigned int id=0;

    float complex z;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        eqlms_cccf_push(eq, y[iy]);     // push input into equalizer
        eqlms_cccf_execute(eq, &z);     // compute equalizer output
        eqlms_cccf_step(eq, d[id], z);  // step equalizer internals

        // update counters
        iy = (iy+1)%11;
        id = (id+1)%13;
    }
    getrusage(RUSAGE_SELF, _finish);

    eqlms_cccf_destroy(eq);
}

// 
void benchmark_eqlms_cccf_n4    EQLMS_CCCF_TRAIN_BENCH_API(4)
void benchmark_eqlms_cccf_n8    EQLMS_CCCF_TRAIN_BENCH_API(8)
void benchmark_eqlms_cccf_n16   EQLMS_CCCF_TRAIN_BENCH_API(16)
void benchmark_eqlms_cccf_n32   EQLMS_CCCF_TRAIN_BENCH_API(32)
void benchmark_eqlms_cccf_n64   EQLMS_CCCF_TRAIN_BENCH_API(64)

