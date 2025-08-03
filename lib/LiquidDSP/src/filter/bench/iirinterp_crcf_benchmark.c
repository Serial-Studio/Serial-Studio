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
void iirinterp_crcf_bench(struct rusage *_start,
                          struct rusage *_finish,
                          unsigned long int *_num_iterations,
                          unsigned int _M,
                          unsigned int _order)
{
    // normalize number of iterations
    *_num_iterations *= 80;
    *_num_iterations /= _M*_order;
    if (*_num_iterations < 1) *_num_iterations = 1;

    // create interpolator from prototype
    liquid_iirdes_filtertype ftype  = LIQUID_IIRDES_BUTTER;
    liquid_iirdes_bandtype   btype  = LIQUID_IIRDES_LOWPASS;
    liquid_iirdes_format     format = LIQUID_IIRDES_SOS;
    float fc =  0.5f / (float)_M;   // filter cut-off frequency
    float f0 =  0.0f;
    float Ap =  0.1f;
    float As = 60.0f;
    iirinterp_crcf q = iirinterp_crcf_create_prototype(_M,ftype,btype,format,_order,fc,f0,Ap,As);

    float complex y[_M];
    // start trials
    getrusage(RUSAGE_SELF, _start);
    unsigned int i;
    for (i=0; i<(*_num_iterations); i++) {
        iirinterp_crcf_execute(q, 1.0f, y);
        iirinterp_crcf_execute(q, 1.0f, y);
        iirinterp_crcf_execute(q, 1.0f, y);
        iirinterp_crcf_execute(q, 1.0f, y);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    iirinterp_crcf_destroy(q);
}

#define IIRINTERP_CRCF_BENCHMARK_API(M,ORDER)   \
(   struct rusage *_start,                      \
    struct rusage *_finish,                     \
    unsigned long int *_num_iterations)         \
{ iirinterp_crcf_bench(_start, _finish, _num_iterations, M, ORDER); }

void benchmark_iirinterp_crcf_M2    IIRINTERP_CRCF_BENCHMARK_API(2, 5)
void benchmark_iirinterp_crcf_M4    IIRINTERP_CRCF_BENCHMARK_API(4, 5)
void benchmark_iirinterp_crcf_M8    IIRINTERP_CRCF_BENCHMARK_API(8, 5)
void benchmark_iirinterp_crcf_M16   IIRINTERP_CRCF_BENCHMARK_API(16,5)
void benchmark_iirinterp_crcf_M32   IIRINTERP_CRCF_BENCHMARK_API(32,5)

