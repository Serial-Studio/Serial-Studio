/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
// benchmark sum-product algorithm
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <math.h>

#include "liquid.internal.h"

#define SUMPRODUCT_BENCH_API(M)         \
(   struct rusage *_start,              \
    struct rusage *_finish,             \
    unsigned long int *_num_iterations) \
{ sumproduct_bench(_start, _finish, _num_iterations, M); }

// generate half-rate LDPC generator and parity-check matrices
void sumproduct_generate(unsigned int    _m,
                         unsigned char * _G,
                         unsigned char * _H);

// Helper function to keep code base small
void sumproduct_bench(struct rusage *     _start,
                      struct rusage *     _finish,
                      unsigned long int * _num_iterations,
                      unsigned int        _m)
{
    // normalize number of iterations
    // seconds/trial ~ exp{ -16.90 + 2.8815*log(_m) }
    float scale = expf(-17.42f + 2.9971*logf(_m));
    *_num_iterations *= 1e-7 / scale;
    if (*_num_iterations < 1)
        *_num_iterations = 1;

    unsigned long int i;

    // derived values
    unsigned int _n = 2*_m;

    // create arrays
    unsigned char Gs[_m*_n]; // generator matrix [m x n]
    unsigned char Hs[_m*_n]; // parity check matrix [m x n]
    sumproduct_generate(_m, Gs, Hs);

    // generate sparse binary matrices
    smatrixb G = smatrixb_create_array(Gs, _n, _m);
    smatrixb H = smatrixb_create_array(Hs, _m, _n);

    // print matrices
    //printf("G:\n"); smatrixb_print_expanded(G);
    //printf("H:\n"); smatrixb_print_expanded(H);

    unsigned char x[_m];     // original message signal
    unsigned char c[_n];     // transmitted codeword
    float LLR[_n];           // log-likelihood ratio
    unsigned char c_hat[_n]; // estimated codeword

    // initialize message array
    for (i=0; i<_m; i++)
        x[i] = rand() % 2;

    // compute encoded message
    smatrixb_vmul(G, x, c);

    // print status
    //printf("x = ["); for (i=0; i<_m; i++) printf("%2u", x[i]); printf(" ];\n");
    //printf("c = ["); for (i=0; i<_n; i++) printf("%2u", c[i]); printf(" ];\n");

    // compute log-likelihood ratio (LLR)
    for (i=0; i<_n; i++)
        LLR[i] = (c[i] == 0 ? 1.0f : -1.0f) + 0.5*randnf();
    
    // start trials
    getrusage(RUSAGE_SELF, _start);
    int parity_pass;
    for (i=0; i<(*_num_iterations); i++) {
        parity_pass = fec_sumproduct(_m, _n, H, LLR, c_hat, 1); LLR[i%_m] += parity_pass ? 1 : -1;
        parity_pass = fec_sumproduct(_m, _n, H, LLR, c_hat, 1); LLR[i%_m] += parity_pass ? 1 : -1;
        parity_pass = fec_sumproduct(_m, _n, H, LLR, c_hat, 1); LLR[i%_m] += parity_pass ? 1 : -1;
        parity_pass = fec_sumproduct(_m, _n, H, LLR, c_hat, 1); LLR[i%_m] += parity_pass ? 1 : -1;
    }
    getrusage(RUSAGE_SELF, _finish);
    *_num_iterations *= 4;
}

//
// BENCHMARKS
//
void benchmark_sumproduct_m16   SUMPRODUCT_BENCH_API(16)
void benchmark_sumproduct_m32   SUMPRODUCT_BENCH_API(32)
void benchmark_sumproduct_m64   SUMPRODUCT_BENCH_API(64)
void benchmark_sumproduct_m128  SUMPRODUCT_BENCH_API(128)

// generate half-rate LDPC generator and parity-check matrices
void sumproduct_generate(unsigned int    _m,
                         unsigned char * _G,
                         unsigned char * _H)
{
    unsigned int i;
    unsigned int j;

    // derived values
    unsigned int _n = 2*_m;

    // initial generator polynomial [1 x m]
    unsigned char p[_m];

    // initialize generator polynomial (systematic)
    for (i=0; i<_m; i++)
        p[i] = 0;
    unsigned int t = 0;
    unsigned int k = 2;
    for (i=0; i<_m; i++) {
        t++;
        if (t == k) {
            t = 0;
            k *= 2;
            p[i] = 1;
        }
    }

    // initialize matrices
    for (i=0; i<_m; i++) {
        for (j=0; j<_m; j++) {
            // G = [I(m) P]^T
            _G[j*_m + i]         = (i==j) ? 1 : 0;
            _G[j*_m + i + _m*_m] = p[(i+j)%_m];

            // H = [P^T I(m)]
            _H[i*_n + j + _m] = (i==j) ? 1 : 0;
            _H[i*_n + j]      = p[(i+j)%_m];
        }
    }
}

