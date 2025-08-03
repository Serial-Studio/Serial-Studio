/*
 * Copyright (c) 2007 - 2018 Joseph Gaeddert
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

typedef enum {
    RESAMP2_DECIM,
    RESAMP2_INTERP
} resamp2_type;

// Helper function to keep code base small
void resamp2_crcf_bench(struct rusage *_start,
                        struct rusage *_finish,
                        unsigned long int * _num_iterations,
                        unsigned int _m,
                        resamp2_type _type)
{
    // scale number of iterations by filter length
    // NOTE: n = 4*m+1
    // cycles/trial ~ 70.5 + 7.74*_m
    *_num_iterations *= 200;
    *_num_iterations /= 70.5 + 7.74*_m;

    unsigned long int i;

    resamp2_crcf q = resamp2_crcf_create(_m,0.0f,60.0f);

    float complex x[] = {1.0f, -1.0f};
    float complex y[] = {1.0f, -1.0f};

    // start trials
    getrusage(RUSAGE_SELF, _start);
    if (_type == RESAMP2_DECIM) {

        // run decimator
        for (i=0; i<(*_num_iterations); i++) {
            resamp2_crcf_decim_execute(q,x,y);
            resamp2_crcf_decim_execute(q,x,y);
            resamp2_crcf_decim_execute(q,x,y);
            resamp2_crcf_decim_execute(q,x,y);
        }
    } else {

        // run interpolator
        for (i=0; i<(*_num_iterations); i++) {
            resamp2_crcf_interp_execute(q,x[0],y);
            resamp2_crcf_interp_execute(q,x[0],y);
            resamp2_crcf_interp_execute(q,x[0],y);
            resamp2_crcf_interp_execute(q,x[0],y);
        }
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    resamp2_crcf_destroy(q);
}

#define RESAMP2_CRCF_BENCHMARK_API(M,T) \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ resamp2_crcf_bench(_start, _finish, _num_iterations, M, T); }

//
// Decimators
//
void benchmark_resamp2_crcf_decim_m2    RESAMP2_CRCF_BENCHMARK_API(  2,RESAMP2_DECIM)
void benchmark_resamp2_crcf_decim_m4    RESAMP2_CRCF_BENCHMARK_API(  4,RESAMP2_DECIM)
void benchmark_resamp2_crcf_decim_m8    RESAMP2_CRCF_BENCHMARK_API(  8,RESAMP2_DECIM)
void benchmark_resamp2_crcf_decim_m16   RESAMP2_CRCF_BENCHMARK_API( 16,RESAMP2_DECIM)
void benchmark_resamp2_crcf_decim_m32   RESAMP2_CRCF_BENCHMARK_API( 32,RESAMP2_DECIM)
void benchmark_resamp2_crcf_decim_m64   RESAMP2_CRCF_BENCHMARK_API( 64,RESAMP2_DECIM)
void benchmark_resamp2_crcf_decim_m128  RESAMP2_CRCF_BENCHMARK_API(128,RESAMP2_DECIM)
void benchmark_resamp2_crcf_decim_m256  RESAMP2_CRCF_BENCHMARK_API(256,RESAMP2_DECIM)

// 
// Interpolators
//
void benchmark_resamp2_crcf_interp_m2   RESAMP2_CRCF_BENCHMARK_API(  2,RESAMP2_INTERP)
void benchmark_resamp2_crcf_interp_m4   RESAMP2_CRCF_BENCHMARK_API(  4,RESAMP2_INTERP)
void benchmark_resamp2_crcf_interp_m8   RESAMP2_CRCF_BENCHMARK_API(  8,RESAMP2_INTERP)
void benchmark_resamp2_crcf_interp_m16  RESAMP2_CRCF_BENCHMARK_API( 16,RESAMP2_INTERP)
void benchmark_resamp2_crcf_interp_m32  RESAMP2_CRCF_BENCHMARK_API( 32,RESAMP2_INTERP)
void benchmark_resamp2_crcf_interp_m64  RESAMP2_CRCF_BENCHMARK_API( 64,RESAMP2_INTERP)
void benchmark_resamp2_crcf_interp_m128 RESAMP2_CRCF_BENCHMARK_API(128,RESAMP2_INTERP)
void benchmark_resamp2_crcf_interp_m256 RESAMP2_CRCF_BENCHMARK_API(256,RESAMP2_INTERP)

