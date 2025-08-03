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
#include <sys/resource.h>

#include "liquid.internal.h"

#define CRC_BENCH_API(CRC,N)                \
(   struct rusage *_start,                  \
    struct rusage *_finish,                 \
    unsigned long int *_num_iterations)     \
{ crc_bench(_start, _finish, _num_iterations, CRC, N); }

// Helper function to keep code base small
void crc_bench(struct rusage *_start,
               struct rusage *_finish,
               unsigned long int *_num_iterations,
               crc_scheme _crc,
               unsigned int _n)
{
    // normalize number of iterations
    if (_crc != LIQUID_CRC_CHECKSUM)
        *_num_iterations /= 100;
    if (*_num_iterations < 1) *_num_iterations = 1;

    unsigned long int i;

    // create arrays
    unsigned char msg[_n];
    unsigned int key = 0;

    // initialize message
    for (i=0; i<_n; i++)
        msg[i] = rand() & 0xff;

    // start trials
    getrusage(RUSAGE_SELF, _start);
    for (i=0; i<(*_num_iterations); i++) {
        key ^= crc_generate_key(_crc, msg, _n);
        key ^= crc_generate_key(_crc, msg, _n);
        key ^= crc_generate_key(_crc, msg, _n);
        key ^= crc_generate_key(_crc, msg, _n);

        // randomize input
        msg[0] ^= key & 0xff;
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
}

//
// BENCHMARKS
//
void benchmark_crc_checksum_n256    CRC_BENCH_API(LIQUID_CRC_CHECKSUM,  256)

void benchmark_crc_crc8_n256        CRC_BENCH_API(LIQUID_CRC_8,         256)
void benchmark_crc_crc16_n256       CRC_BENCH_API(LIQUID_CRC_16,        256)
void benchmark_crc_crc24_n256       CRC_BENCH_API(LIQUID_CRC_24,        256)
void benchmark_crc_crc32_n256       CRC_BENCH_API(LIQUID_CRC_32,        256)

