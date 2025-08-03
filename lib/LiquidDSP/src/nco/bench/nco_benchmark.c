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
#include <string.h>

#include "liquid.h"

void benchmark_nco_sincos(struct rusage *_start,
                          struct rusage *_finish,
                          unsigned long int *_num_iterations)
{
    float s, c;
    nco_crcf p = nco_crcf_create(LIQUID_NCO);
    nco_crcf_set_phase(p, 0.0f);
    nco_crcf_set_frequency(p, 0.1f);

    unsigned int i;

    // increase number of iterations for NCO
    *_num_iterations *= 100;

    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        nco_crcf_sincos(p, &s, &c);
        nco_crcf_step(p);
    }
    getrusage(RUSAGE_SELF, _finish);

    nco_crcf_destroy(p);
}

void benchmark_nco_mix_up(struct rusage *_start,
                          struct rusage *_finish,
                          unsigned long int *_num_iterations)
{
    float complex x[16],  y[16];
    memset(x, 0, 16*sizeof(float complex));

    nco_crcf p = nco_crcf_create(LIQUID_NCO);
    nco_crcf_set_phase(p, 0.0f);
    nco_crcf_set_frequency(p, 0.1f);

    unsigned int i, j;

    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        for (j=0; j<16; j++) {
            nco_crcf_mix_up(p, x[j], &y[j]);
            nco_crcf_step(p);
        }

    }
    getrusage(RUSAGE_SELF, _finish);

    *_num_iterations *= 16;
    nco_crcf_destroy(p);
}

void benchmark_nco_mix_block_up(struct rusage *_start,
                                struct rusage *_finish,
                                unsigned long int *_num_iterations)
{
    float complex x[16], y[16];
    memset(x, 0, 16*sizeof(float complex));

    nco_crcf p = nco_crcf_create(LIQUID_NCO);
    nco_crcf_set_phase(p, 0.0f);
    nco_crcf_set_frequency(p, 0.1f);

    unsigned int i;

    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        nco_crcf_mix_block_up(p, x, y, 16);
    }
    getrusage(RUSAGE_SELF, _finish);

    *_num_iterations *= 16;
    nco_crcf_destroy(p);
}

