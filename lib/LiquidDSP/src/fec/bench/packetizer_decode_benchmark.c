/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
#include <math.h>
#include <sys/resource.h>

#include "liquid.internal.h"

#define PACKETIZER_DECODE_BENCH_API(N, CRC, FEC0, FEC1) \
(   struct rusage *_start,                              \
    struct rusage *_finish,                             \
    unsigned long int *_num_iterations)                 \
{ packetizer_decode_bench(_start, _finish, _num_iterations, N, CRC, FEC0, FEC1); }

// Helper function to keep code base small
void packetizer_decode_bench(struct rusage *     _start,
                             struct rusage *     _finish,
                             unsigned long int * _num_iterations,
                             unsigned int        _n,
                             crc_scheme          _crc,
                             fec_scheme          _fec0,
                             fec_scheme          _fec1)
{
    //
    unsigned int msg_dec_len = _n;
    unsigned int msg_enc_len = packetizer_compute_enc_msg_len(_n,_crc,_fec0,_fec1);

    // adjust number of iterations
    //  k-cycles/trial ~ 221 + 1.6125*msg_dec_len;
    // TODO: adjust iterations based on encoder types
    *_num_iterations *= 1000;
    *_num_iterations /= 221 + 1.6125*msg_dec_len;

    unsigned char msg_rec[msg_enc_len];
    unsigned char msg_dec[msg_dec_len];

    // initialize data
    unsigned long int i;
    for (i=0; i<msg_enc_len; i++) msg_rec[i] = rand() & 0xff;
    for (i=0; i<msg_dec_len; i++) msg_dec[i] = 0x00;

    // create packet generator
    packetizer q = packetizer_create(msg_dec_len, _crc, _fec0, _fec1);
    int crc_pass = 0;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        // decode packet
        crc_pass |= packetizer_decode(q, msg_rec, msg_dec);
        crc_pass |= packetizer_decode(q, msg_rec, msg_dec);
        crc_pass |= packetizer_decode(q, msg_rec, msg_dec);
        crc_pass |= packetizer_decode(q, msg_rec, msg_dec);

        // randomize input
        msg_rec[0] ^= crc_pass ? 1 : 0;
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;

    // clean up allocated objects
    packetizer_destroy(q);
}


//
// BENCHMARKS
//
void benchmark_packetizer_n16   PACKETIZER_DECODE_BENCH_API(16,   LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE)
void benchmark_packetizer_n32   PACKETIZER_DECODE_BENCH_API(32,   LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE)
void benchmark_packetizer_n64   PACKETIZER_DECODE_BENCH_API(64,   LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE)
void benchmark_packetizer_n128  PACKETIZER_DECODE_BENCH_API(128,  LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE)
void benchmark_packetizer_n256  PACKETIZER_DECODE_BENCH_API(256,  LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE)
void benchmark_packetizer_n512  PACKETIZER_DECODE_BENCH_API(512,  LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE)
void benchmark_packetizer_n1024 PACKETIZER_DECODE_BENCH_API(1024, LIQUID_CRC_NONE, LIQUID_FEC_NONE, LIQUID_FEC_NONE)

