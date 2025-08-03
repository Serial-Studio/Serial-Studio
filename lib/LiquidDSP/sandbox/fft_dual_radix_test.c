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
// Test mixed-radix FFT algorithm
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <getopt.h>
#include <complex.h>

#define DEBUG 0
#define DFT_FORWARD (-1)
#define DFT_REVERSE ( 1)

// print usage/help message
void usage()
{
    printf("fft_mixed_radix_test -- test mixed-radix DFTs, compare to slow DFT method\n");
    printf("options (default values in []):\n");
    printf("  u/h   : print usage/help\n");
    printf("  p     : stride (freq)\n");
    printf("  q     : stride (time)\n");
}

// super slow DFT, but functionally correct
void dft_run(unsigned int    _nfft,
             float complex * _x,
             float complex * _y,
             int             _dir,
             int             _flags);

int main(int argc, char*argv[]) {
    // transform size: p*q
    unsigned int p = 5;
    unsigned int q = 3;

    int dopt;
    while ((dopt = getopt(argc,argv,"uhp:q:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();          return 0;
        case 'p': p = atoi(optarg); break;
        case 'q': q = atoi(optarg); break;
        default:
            exit(1);
        }
    }

    // transform size
    unsigned int n = p*q;

    // validate input
    if ( n == 0 ) {
        fprintf(stderr,"error: input transform size must be at least 2\n");
        exit(1);
    }

    unsigned int i;
    unsigned int k;

    // create and initialize data arrays
    float complex x[n];
    float complex y[n];
    float complex y_test[n];
    for (i=0; i<n; i++) {
        //x[i] = randnf() + _Complex_I*randnf();
        x[i] = (float)i + _Complex_I*(3 - (float)i);
    }

    // compute output for testing
    dft_run(n, x, y_test, DFT_FORWARD, 0);

    //
    // run Cooley-Tukey FFT
    //

    // compute twiddle factors (roots of unity)
    float complex twiddle[n];
    for (i=0; i<n; i++)
        twiddle[i] = cexpf(-_Complex_I*2*M_PI*(float)i / (float)n);

    // temporary buffer
    float complex t[n];
    for (i=0; i<n; i++)
        t[i] = x[i];

#if DEBUG
    for (i=0; i<n; i++) {
        printf("  t[%3u] = %12.6f + j*%12.6f\n",
            i, crealf(t[i]), cimagf(t[i]));
    }
#endif

    // compute 'q' DFTs of size 'p' and multiply by twiddle factors
    printf("computing %u DFTs of size %u...\n", q, p);
    for (i=0; i<q; i++) {
#if DEBUG
        printf("  i=%3u/%3u\n", i, q);
#endif

        // for now, copy to temp buffer, compute FFT, and store result
        float complex t0[p];
        float complex t1[p];
        for (k=0; k<p; k++) t0[k] = t[q*k+i];
        dft_run(p, t0, t1, DFT_FORWARD, 0);
        for (k=0; k<p; k++) t[q*k+i] = t1[k];

#if DEBUG
        for (k=0; k<p; k++)
            printf("  %12.6f + j%12.6f > %12.6f + j%12.6f\n", crealf(t0[k]), cimagf(t0[k]), crealf(t1[k]), cimagf(t1[k]));
#endif
    }

    // multiply by twiddle factors
    // NOTE: this can be combined with previous step
    printf("multiplying twiddles...\n");
    for (i=0; i<q; i++) {
#if DEBUG
        printf("  i=%3u/%3u\n", i, q);
#endif
        for (k=0; k<p; k++) 
            t[q*k+i] *= twiddle[i*k];

#if DEBUG
        for (k=0; k<p; k++) {
            printf("  tw[%4u] = %12.8f + j%12.8f, t=%12.6f + j%12.6f\n",
                    i*k,
                    crealf(twiddle[i*k]), cimagf(twiddle[i*k]),
                    crealf(t[q*k+i]),     cimagf(t[q*k+i]));
        }
#endif
    }

    // compute 'p' DFTs of size 'q'
    printf("computing %u DFTs of size %u...\n", p, q);
    for (i=0; i<p; i++) {
#if DEBUG
        printf("  i=%3u/%3u\n", i, p);
#endif
        
        // for now, copy to temp buffer, compute FFT, and store result
        float complex t0[q];
        float complex t1[q];
        for (k=0; k<q; k++) t0[k] = t[q*i+k];
        dft_run(q, t0, t1, DFT_FORWARD, 0);
        for (k=0; k<q; k++) t[q*i+k] = t1[k];

#if DEBUG
        for (k=0; k<q; k++)
            printf("  %12.6f + j%12.6f > %12.6f + j%12.6f\n", crealf(t0[k]), cimagf(t0[k]), crealf(t1[k]), cimagf(t1[k]));
#endif
    }

    // transpose results
    for (i=0; i<p; i++) {
        for (k=0; k<q; k++) {
            y[k*p+i] = t[i*q+k];
        }
    }

    // 
    // print results
    //
    for (i=0; i<n; i++) {
        printf("  t[%3u] = %12.6f + j*%12.6f (expected %12.6f + j%12.6f)\n",
            i,
            crealf(y[i]),      cimagf(y[i]),
            crealf(y_test[i]), cimagf(y_test[i]));
    }

    // compute error
    float rmse = 0.0f;
    for (i=0; i<n; i++) {
        float e = cabsf(y[i] - y_test[i]);
        rmse += e*e;
    }
    rmse = sqrtf(rmse / (float)n);
    printf("RMS error : %12.4e (%s)\n", rmse, rmse < 1e-3 ? "pass" : "FAIL");

    return 0;
}

// super slow DFT, but functionally correct
void dft_run(unsigned int    _nfft,
             float complex * _x,
             float complex * _y,
             int             _dir,
             int             _flags)
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

