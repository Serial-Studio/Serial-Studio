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
// Test recursive mixed-radix FFT algorithm
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <getopt.h>
#include <complex.h>

#include "liquid.h"

#define DEBUG 0
#define DFT_FORWARD (-1)
#define DFT_REVERSE ( 1)

// print usage/help message
void usage()
{
    printf("fft_recursive_test -- test recursive mixed-radix DFTs, compare to slow DFT method\n");
    printf("options (default values in []):\n");
    printf("  h     : print help\n");
    printf("  n     : fft size, default: 30\n");
}

// recursive FFT algorithm
void fft_recursion(unsigned int    _nfft,
                   float complex * _x,
                   float complex * _y,
                   int             _dir);

// super slow DFT, but functionally correct
void dft_run(unsigned int    _nfft,
             float complex * _x,
             float complex * _y,
             int             _dir);

int main(int argc, char*argv[])
{
    // transform size
    unsigned int nfft = 30;

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:")) != EOF) {
        switch (dopt) {
        case 'h': usage();              return 0;
        case 'n': nfft = atoi(optarg);  break;
        default:
            exit(1);
        }
    }

    // validate input
    if ( nfft < 2 ) {
        fprintf(stderr,"error: %s, input transform size must be at least 2\n", argv[0]);
        exit(1);
    }

    unsigned int i;

    // create and initialize data arrays
    float complex x[nfft];
    float complex y[nfft];
    float complex y_test[nfft];
    for (i=0; i<nfft; i++) {
        //x[i] = randnf() + _Complex_I*randnf();
        x[i] = (float)i + _Complex_I*(3 - (float)i);
    }

    // compute output for testing
    printf("running dft...\n");
    dft_run(nfft, x, y_test, DFT_FORWARD);

    //
    // run Cooley-Tukey FFT
    //

    for (i=0; i<nfft; i++)
        y[i] = 0.0f;

    // run recursion
    // NOTE: this will destroy input array x
    printf("running recursion...\n");
    fft_recursion(nfft, x, y, DFT_FORWARD);

    // 
    // print results
    //
    for (i=0; i<nfft; i++) {
        printf("  y[%3u] = %12.6f + j*%12.6f (expected %12.6f + j%12.6f)\n",
            i,
            crealf(y[i]),      cimagf(y[i]),
            crealf(y_test[i]), cimagf(y_test[i]));
    }

    // compute error
    float rmse = 0.0f;
    for (i=0; i<nfft; i++) {
        float e = cabsf(y[i] - y_test[i]);
        rmse += e*e;
    }
    rmse = sqrtf(rmse / (float)nfft);
    printf("RMS error : %12.4e (%s)\n", rmse, rmse < 1e-3 ? "pass" : "FAIL");

    return 0;
}

// recursive...
void fft_recursion(unsigned int    _nfft,
                   float complex * _x,
                   float complex * _y,
                   int             _dir)
{
    // determine if _nfft is divisible by ...
    unsigned int i;
    unsigned int q = 0;
    for (i=2; i<_nfft; i++) {
        if ( (_nfft % i) == 0 ) {
            q = i;
            break;
        }
    }

    if ( q == 0 ) {
        // run slow DFT method and return
#if DEBUG
        printf("computing DFT of size %u\n", _nfft);
#endif
        dft_run(_nfft, _x, _y, _dir);
        return;
    }

    // run dual-radix method
    unsigned int p = _nfft / q;
    unsigned int k;

    // initialize twiddle factors
    float complex twiddle[_nfft];
    float d = _dir == DFT_FORWARD ? -1.0f : 1.0f;
    for (i=0; i<_nfft; i++)
        twiddle[i] = cexpf(d*_Complex_I*2*M_PI*(float)i/(float)_nfft);

    // compute 'q' DFTs of size 'p'
#if DEBUG
    printf("computing %u DFTs of size %u\n", q, p);
#endif
    for (i=0; i<q; i++) {
        // copy to temp buffer, compute FFT, return result
        float complex t0[p];
        float complex t1[p];

        // copy to temporary buffer
        for (k=0; k<p; k++)
            t0[k] = _x[q*k+i];

        // run DFT
        fft_recursion(p, t0, t1, _dir);

        // copy back to input, applying twiddle factors
        for (k=0; k<p; k++)
            _x[q*k+i] = t1[k] * twiddle[i*k];

#if DEBUG
        printf("i=%3u/%3u\n", i, q);
        for (k=0; k<p; k++)
            printf("  %12.6f %12.6f\n", crealf(_x[q*k+i]), cimagf(_x[q*k+i]));
#endif
    }

    // compute 'p' DFTs of size 'q' and transpose
#if DEBUG
    printf("computing %u DFTs of size %u\n", p, q);
#endif
    for (i=0; i<p; i++) {
        // copy to temp buffer...
        float complex t0[q];
        float complex t1[q];

        for (k=0; k<q; k++)
            t0[k] = _x[q*i+k];

        fft_recursion(q, t0, t1, _dir);

        // copy and transpose
        for (k=0; k<q; k++)
            _y[k*p+i] = t1[k];
            //_x[q*i+k] = t1[k];
        
#if DEBUG
        printf("i=%3u/%3u\n", i, p);
        for (k=0; k<q; k++)
            printf("  %12.6f %12.6f\n", crealf(_x[q*i+k]), cimagf(_x[q*i+k]));
#endif
    }
}

// super slow DFT, but functionally correct
void dft_run(unsigned int    _nfft,
             float complex * _x,
             float complex * _y,
             int             _dir)
{
    unsigned int i;
    unsigned int k;

    int d = (_dir == DFT_FORWARD) ? -1 : 1;

    for (i=0; i<_nfft; i++) {
        _y[i] = 0.0f;
        for (k=0; k<_nfft; k++) {
            float phi = 2*M_PI*d*i*k / (float)_nfft;
            _y[i] += _x[k] * cexpf(_Complex_I*phi);
        }
    }
}

