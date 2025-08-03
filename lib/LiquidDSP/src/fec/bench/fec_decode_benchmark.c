/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
#include <string.h>
#include <sys/resource.h>

#include "liquid.internal.h"

#define FEC_DECODE_BENCH_API(FS,N, OPT) \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ fec_decode_bench(_start, _finish, _num_iterations, FS, N, OPT); }

// Helper function to keep code base small
void fec_decode_bench(
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations,
    fec_scheme _fs,
    unsigned int _n,
    void * _opts)
{
#if !LIBFEC_ENABLED
    if ( _fs == LIQUID_FEC_CONV_V27    ||
         _fs == LIQUID_FEC_CONV_V29    ||
         _fs == LIQUID_FEC_CONV_V39    ||
         _fs == LIQUID_FEC_CONV_V615   ||
         _fs == LIQUID_FEC_CONV_V27P23 ||
         _fs == LIQUID_FEC_CONV_V27P34 ||
         _fs == LIQUID_FEC_CONV_V27P45 ||
         _fs == LIQUID_FEC_CONV_V27P56 ||
         _fs == LIQUID_FEC_CONV_V27P67 ||
         _fs == LIQUID_FEC_CONV_V27P78 ||
         _fs == LIQUID_FEC_CONV_V29P23 ||
         _fs == LIQUID_FEC_CONV_V29P34 ||
         _fs == LIQUID_FEC_CONV_V29P45 ||
         _fs == LIQUID_FEC_CONV_V29P56 ||
         _fs == LIQUID_FEC_CONV_V29P67 ||
         _fs == LIQUID_FEC_CONV_V29P78 ||
         _fs == LIQUID_FEC_RS_M8)
    {
        liquid_error(LIQUID_EUMODE,"convolutional, Reed-Solomon codes unavailable (install libfec)");
        getrusage(RUSAGE_SELF, _start);
        memmove((void*)_finish,(void*)_start,sizeof(struct rusage));
        return;
    }
#endif

    // normalize number of iterations
    *_num_iterations /= _n;

    switch (_fs) {
    case LIQUID_FEC_NONE:          *_num_iterations *= 500;    break;
    case LIQUID_FEC_REP3:          *_num_iterations *= 200;    break;
    case LIQUID_FEC_REP5:          *_num_iterations *= 100;    break;
    case LIQUID_FEC_HAMMING74:     *_num_iterations *= 10;     break;
    case LIQUID_FEC_HAMMING84:     *_num_iterations *= 10;     break;
    case LIQUID_FEC_HAMMING128:    *_num_iterations *= 10;     break;
    case LIQUID_FEC_SECDED2216:    *_num_iterations *= 10;     break;
    case LIQUID_FEC_SECDED3932:    *_num_iterations *= 10;     break;
    case LIQUID_FEC_SECDED7264:    *_num_iterations *= 10;     break;
    case LIQUID_FEC_GOLAY2412:     *_num_iterations *= 2;      break;
    case LIQUID_FEC_CONV_V27:      *_num_iterations /= 5;      break;
    case LIQUID_FEC_CONV_V29:      *_num_iterations /= 10;     break;
    case LIQUID_FEC_CONV_V39:      *_num_iterations /= 100;    break;
    case LIQUID_FEC_CONV_V615:     *_num_iterations /= 200;    break;
    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:
        *_num_iterations /= 20;
        break;
    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        *_num_iterations /= 50;
        break;
    case LIQUID_FEC_RS_M8:
        *_num_iterations *= 1;
        break;
    default:;
    }
    if (*_num_iterations < 1) *_num_iterations = 1;

    // generate fec object
    fec q = fec_create(_fs,_opts);

    // create arrays
    unsigned int n_enc = fec_get_enc_msg_length(_fs,_n);
    unsigned char msg[_n];          // original message
    unsigned char msg_enc[n_enc];   // decoded message
    unsigned char msg_dec[_n];      // decoded message

    // initialize message
    unsigned long int i;
    for (i=0; i<_n; i++)
        msg[i] = rand() & 0xff;

    // encode message
    fec_encode(q,_n,msg,msg_enc);

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        fec_decode(q,_n,msg_enc,msg_dec);
        fec_decode(q,_n,msg_enc,msg_dec);
        fec_decode(q,_n,msg_enc,msg_dec);
        fec_decode(q,_n,msg_enc,msg_dec);
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // clean up objects
    fec_destroy(q);
}

//
// BENCHMARKS
//
void benchmark_fec_dec_none_n64         FEC_DECODE_BENCH_API(LIQUID_FEC_NONE,      64,  NULL)

void benchmark_fec_dec_rep3_n64         FEC_DECODE_BENCH_API(LIQUID_FEC_REP3,      64,  NULL)
void benchmark_fec_dec_rep5_n64         FEC_DECODE_BENCH_API(LIQUID_FEC_REP5,      64,  NULL)
void benchmark_fec_dec_hamming74_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_HAMMING74, 64,  NULL)
void benchmark_fec_dec_hamming84_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_HAMMING84, 64,  NULL)
void benchmark_fec_dec_hamming128_n64   FEC_DECODE_BENCH_API(LIQUID_FEC_HAMMING128,64,  NULL)

// SEC-DED block codes
void benchmark_fec_dec_secded2216_n64   FEC_DECODE_BENCH_API(LIQUID_FEC_SECDED2216,64,  NULL)
void benchmark_fec_dec_secded3932_n64   FEC_DECODE_BENCH_API(LIQUID_FEC_SECDED3932,64,  NULL)
void benchmark_fec_dec_secded7264_n64   FEC_DECODE_BENCH_API(LIQUID_FEC_SECDED7264,64,  NULL)

void benchmark_fec_dec_golay2412_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_GOLAY2412, 64,  NULL)

void benchmark_fec_dec_conv27_n64       FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V27,  64,  NULL)
void benchmark_fec_dec_conv29_n64       FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V29,  64,  NULL)
void benchmark_fec_dec_conv39_n64       FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V39,  64,  NULL)
void benchmark_fec_dec_conv615_n64      FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V615, 64,  NULL)

void benchmark_fec_dec_conv27p23_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V27P23,64, NULL)
void benchmark_fec_dec_conv27p34_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V27P34,64, NULL)
void benchmark_fec_dec_conv27p45_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V27P45,64, NULL)
void benchmark_fec_dec_conv27p56_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V27P56,64, NULL)
void benchmark_fec_dec_conv27p67_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V27P67,64, NULL)
void benchmark_fec_dec_conv27p78_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V27P78,64, NULL)

void benchmark_fec_dec_conv29p23_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V29P23,64, NULL)
void benchmark_fec_dec_conv29p34_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V29P34,64, NULL)
void benchmark_fec_dec_conv29p45_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V29P45,64, NULL)
void benchmark_fec_dec_conv29p56_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V29P56,64, NULL)
void benchmark_fec_dec_conv29p67_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V29P67,64, NULL)
void benchmark_fec_dec_conv29p78_n64    FEC_DECODE_BENCH_API(LIQUID_FEC_CONV_V29P78,64, NULL)

void benchmark_fec_dec_rs8_n64          FEC_DECODE_BENCH_API(LIQUID_FEC_RS_M8,      64,  NULL)

