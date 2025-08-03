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

// Helper function to keep code base small
void iirfilt_crcf_bench(struct rusage *     _start,
                        struct rusage *     _finish,
                        unsigned long int * _num_iterations,
                        unsigned int        _order,
                        unsigned int        _format)
{
    unsigned int i;

    // scale number of iterations (trials)
    if (_format == LIQUID_IIRDES_TF) {
        // cycles/trial ~ 128 + 15.3*_order;
        *_num_iterations *= 1000;
        *_num_iterations /= (unsigned int)(128 + 15.3*_order);
    } else {
        // cycles/trial ~ 93 + 53.3*_order
        *_num_iterations *= 800;
        *_num_iterations /= (unsigned int)(93 + 53.3*_order);
    }

    // create filter object from prototype
    float fc    =  0.2f;    // filter cut-off frequency
    float f0    =  0.0f;    // filter center frequency (band-pass, band-stop)
    float Ap    =  0.1f;    // filter pass-band ripple
    float As    = 60.0f;    // filter stop-band attenuation
    iirfilt_crcf q = iirfilt_crcf_create_prototype(LIQUID_IIRDES_BUTTER,
                                                   LIQUID_IIRDES_LOWPASS,
                                                   _format,
                                                   _order,
                                                   fc, f0, Ap, As);

    // initialize input/output
    float complex x[4];
    float complex y[4];
    for (i=0; i<4; i++)
        x[i] = randnf() + _Complex_I*randnf();

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        iirfilt_crcf_execute(q, x[0], &y[0]);
        iirfilt_crcf_execute(q, x[1], &y[1]);
        iirfilt_crcf_execute(q, x[2], &y[2]);
        iirfilt_crcf_execute(q, x[3], &y[3]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // destroy filter object
    iirfilt_crcf_destroy(q);
}

#define IIRFILT_CRCF_BENCHMARK_API(N,T)     \
(   struct rusage *_start,                  \
    struct rusage *_finish,                 \
    unsigned long int *_num_iterations)     \
{ iirfilt_crcf_bench(_start, _finish, _num_iterations, N, T); }

// benchmark regular transfer function form
void benchmark_iirfilt_crcf_4        IIRFILT_CRCF_BENCHMARK_API(4,    LIQUID_IIRDES_TF)
void benchmark_iirfilt_crcf_8        IIRFILT_CRCF_BENCHMARK_API(8,    LIQUID_IIRDES_TF)
void benchmark_iirfilt_crcf_16       IIRFILT_CRCF_BENCHMARK_API(16,   LIQUID_IIRDES_TF)
void benchmark_iirfilt_crcf_32       IIRFILT_CRCF_BENCHMARK_API(32,   LIQUID_IIRDES_TF)
void benchmark_iirfilt_crcf_64       IIRFILT_CRCF_BENCHMARK_API(64,   LIQUID_IIRDES_TF)

// benchmark second-order sections form
void benchmark_iirfilt_crcf_sos_4    IIRFILT_CRCF_BENCHMARK_API(4,    LIQUID_IIRDES_SOS)
void benchmark_iirfilt_crcf_sos_8    IIRFILT_CRCF_BENCHMARK_API(8,    LIQUID_IIRDES_SOS)
void benchmark_iirfilt_crcf_sos_16   IIRFILT_CRCF_BENCHMARK_API(16,   LIQUID_IIRDES_SOS)
void benchmark_iirfilt_crcf_sos_32   IIRFILT_CRCF_BENCHMARK_API(32,   LIQUID_IIRDES_SOS)
void benchmark_iirfilt_crcf_sos_64   IIRFILT_CRCF_BENCHMARK_API(64,   LIQUID_IIRDES_SOS)

// benchmark DC-blocking filter
void benchmark_irfilt_crcf_dcblock(struct rusage *     _start,
                                   struct rusage *     _finish,
                                   unsigned long int * _num_iterations)
{
    unsigned long int i;

    // create filter object
    iirfilt_crcf q = iirfilt_crcf_create_dc_blocker(0.1f);

    // initialize input/output
    float complex x[4];
    for (i=0; i<4; i++)
        x[i] = randnf() + _Complex_I*randnf();

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        iirfilt_crcf_execute(q, x[0], &x[0]);
        iirfilt_crcf_execute(q, x[1], &x[1]);
        iirfilt_crcf_execute(q, x[2], &x[2]);
        iirfilt_crcf_execute(q, x[3], &x[3]);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // destroy filter object
    iirfilt_crcf_destroy(q);
}

