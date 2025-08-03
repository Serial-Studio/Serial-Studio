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
#define MAX_FACTORS (40)

// print usage/help message
void usage()
{
    printf("fft_mixed_radix_test -- test mixed-radix DFTs, compare to slow DFT method\n");
    printf("options (default values in []):\n");
    printf("  h     : print usage/help\n");
    printf("  n     : fft size\n");
}

// super slow DFT, but functionally correct
void dft_run(unsigned int    _nfft,
             float complex * _x,
             float complex * _y,
             int             _dir,
             int             _flags);

// FFT mixed-radix butterfly
//  _x          :   input/output buffer pointer [size: _nfft x 1]
//  _twiddle    :   pre-computed twiddle factors [size: _nfft x 1]
//  _nfft       :   original FFT size
//  _stride     :   output stride
//  _m          :   number of FFTs to compute
//  _p          :   generic (small) FFT size
//
// NOTES : the butterfly decimates in time, storing the output as
//         contiguous samples in the same buffer.
void fftmr_bfly(float complex * _x,
                float complex * _twiddle,
                unsigned int    _nfft,
                unsigned int    _stride,
                unsigned int    _m,
                unsigned int    _p)
{
#if DEBUG
    printf("  bfly: stride=%3u, m=%3u, p=%3u\n", _stride, _m, _p);
#endif

    // create temporary buffer the size of the FFT
    float complex * x_tmp = (float complex *) malloc(_p*sizeof(float complex));

    unsigned int i;
    unsigned int k;

    unsigned int n;
    for (n=0; n<_m; n++) {
#if DEBUG
        printf("    u=%u\n", n);
#endif

        // copy input to temporary buffer
        for (i=0; i<_p; i++)
            x_tmp[i] = _x[n + i*_m];
        
        // compute DFT, applying appropriate twiddle factors
        unsigned int twiddle_base = n;
        for (i=0; i<_p; i++) {
#if DEBUG
            printf("      ----\n");
#endif
            float complex y = x_tmp[0];
            unsigned int twiddle_index = 0;
            for (k=1; k<_p; k++) {
                twiddle_index = (twiddle_index + _stride*twiddle_base) % _nfft;
#if DEBUG
                printf("      twiddle_index = %3u > %12.8f + j%12.8f, %12.8f + j%12.8f\n", twiddle_index, crealf(_twiddle[twiddle_index]), cimagf(_twiddle[twiddle_index]), crealf(x_tmp[k]), cimagf(x_tmp[k]));
#endif

                y += x_tmp[k] * _twiddle[twiddle_index];
            }
            // increment twiddle twiddle base
            twiddle_base += _m;

            // store output
            _x[n + i*_m] = y;
#if DEBUG
            printf("      y = %12.6f + j%12.6f\n", crealf(y), cimagf(y));
#endif
        }
    }

    // free temporary buffer
    free(x_tmp);
}

// FFT mixed-radix recursive function...
//  _x          :   constant input pointer [size: _nfft x 1]
//  _y          :   output pointer
//  _twiddle    :   pre-computed twiddle factors [size: _nfft x 1]
//  _nfft       :   original FFT size
//  _xoffset    :   input buffer offset
//  _xstride    :   input buffer stride
//  _m_vect     :   array of radix values [size: num_factors x 1]
//  _p_vect     :   array of DFT values [size: num_factors x 1]
void fftmr_cycle(float complex * _x,
                 float complex * _y,
                 float complex * _twiddle,
                 unsigned int    _nfft,
                 unsigned int    _xoffset,
                 unsigned int    _xstride,
                 unsigned int *  _m_vect,
                 unsigned int *  _p_vect)
{
    // de-reference factors and pop values off the top
    unsigned int m = _m_vect[0];    // radix
    unsigned int p = _p_vect[0];    // DFT size

    // increment factor pointers
    _m_vect++;
    _p_vect++;
    
#if DEBUG
    printf("fftmr_cycle:    offset=%3u, stride=%3u, p=%3u, m=%3u\n", _xoffset, _xstride, p, m);
#endif

    unsigned int i;
    if ( m == 1 ) {
        // copy data to output buffer
        for (i=0; i<p; i++)
            _y[i] = _x[_xoffset + _xstride*i];

    } else {
        // call fftmr_cycle() recursively, effectively computing
        // p DFTs each of size m samples, decimating the time
        // input by _xstride
        for (i=0; i<p; i++) {
            fftmr_cycle(_x,                     // input buffer (does not change)
                        _y + i*m,               // increment output buffer by block size
                        _twiddle,               // twiddle factors (no change)
                        _nfft,                  // original FFT size (no change)
                        _xoffset + _xstride*i,  // input offset (increased by _xstride)
                        _xstride*p,             // input stride (scaled by radix)
                        _m_vect,                // array of radix values (length reduced by one)
                        _p_vect);               // array of DFT values (length reduced by one)
        }
    }

    // run m-point DFT
    fftmr_bfly(_y, _twiddle, _nfft, _xstride, m, p);
}
                      

int main(int argc, char*argv[]) {
    // transform size
    unsigned int nfft = 30;

    int dopt;
    while ((dopt = getopt(argc,argv,"uhn:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                return 0;
        case 'n':   nfft = atoi(optarg);    break;
        default:
            exit(1);
        }
    }

    // validate input
    if ( nfft == 0 ) {
        fprintf(stderr,"error: input transform size must be at least 2\n");
        exit(1);
    }

    unsigned int i;
    unsigned int k;
    
    // find 'prime' factors
    unsigned int n = nfft;
    unsigned int p[MAX_FACTORS];
    unsigned int m[MAX_FACTORS];
    unsigned int num_factors = 0;

    do {
        for (k=2; k<=n; k++) {
            if ( (n%k)==0 ) {
                n /= k;
                p[num_factors] = k;
                m[num_factors] = n;
                num_factors++;
                break;
            }
        }
    } while (n > 1 && num_factors < MAX_FACTORS);

    // NOTE: this is extremely unlikely as the worst case is
    //       nfft=2^MAX_FACTORS in which case we will probably run out
    //       of memory first
    if (num_factors == MAX_FACTORS) {
        fprintf(stderr,"error: could not factor %u with %u factors\n", nfft, MAX_FACTORS);
        exit(1);
    }

    printf("factors of %u:\n", nfft);
    for (i=0; i<num_factors; i++)
        printf("  p=%3u, m=%3u\n", p[i], m[i]);

    // create and initialize data arrays
    float complex * x      = (float complex *) malloc(nfft * sizeof(float complex));
    float complex * y      = (float complex *) malloc(nfft * sizeof(float complex));
    float complex * y_test = (float complex *) malloc(nfft * sizeof(float complex));
    if (x == NULL || y == NULL || y_test == NULL) {
        fprintf(stderr,"error: %s, not enough memory for allocation\n", argv[0]);
        exit(1);
    }
    for (i=0; i<nfft; i++) {
        //x[i] = randnf() + _Complex_I*randnf();
        x[i] = (float)i + _Complex_I*(3 - (float)i);
        y[i] = 0.0f;
    }

    // compute output for testing
    dft_run(nfft, x, y_test, DFT_FORWARD, 0);

    // compute twiddle factors (roots of unity)
    float complex * twiddle = (float complex *) malloc(nfft * sizeof(float complex));
    if (x == NULL || y == NULL || y_test == NULL) {
        fprintf(stderr,"error: %s, not enough memory for twiddle factors\n", argv[0]);
        exit(1);
    }
    for (i=0; i<nfft; i++)
        twiddle[i] = cexpf(-_Complex_I*2*M_PI*(float)i / (float)nfft);

    // call mixed-radix function
    fftmr_cycle(x, y, twiddle, nfft, 0, 1, m, p);

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

    // free allocated memory
    free(x);
    free(y);
    free(y_test);
    free(twiddle);

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

