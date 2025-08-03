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
// Test recursive FFT plan
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <getopt.h>
#include <complex.h>

#include "liquid.internal.h"

// print usage/help message
void usage()
{
    printf("fft_recursive_plan_test -- test planning FFTs\n");
    printf("options (default values in []):\n");
    printf("  h     : print help\n");
    printf("  n     : input fft size\n");
}

// print fft plan
//  _nfft   :   input fft size
//  _level  :   level
void liquid_fft_plan(unsigned int _nfft,
                     unsigned int _level);

int main(int argc, char*argv[])
{
    // options
    unsigned int nfft = 124;

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
    if ( nfft == 0 ) {
        fprintf(stderr,"error: input transform size must be at least 2\n");
        exit(1);
    }

    // print the FFT plan
    liquid_fft_plan(nfft, 0);

    printf("done.\n");
    return 0;
}

// print fft plan
//  _nfft   :   input fft size
//  _level  :   level
void liquid_fft_plan(unsigned int _nfft,
                     unsigned int _level)
{
    if (_nfft == 0) {
        // invalid length
        fprintf(stderr,"error: liquid_fft_estimate_method(), fft size must be > 0\n");
        exit(1);
    }

    // print appropriate spacing
    unsigned int i;
    for (i=0; i<_level; i++)
        printf("    ");

    // print fft size
    printf("%u, ", _nfft);

    if (_nfft < 8) {
        // use simple DFT codelet
        printf("codelet\n");
        return;

    } else if (fft_is_radix2(_nfft)) {
        // transform is of the form 2^m
        printf("radix-2\n");
        return;

    } else if (liquid_is_prime(_nfft)) {
        // compute prime factors of _nfft-1
        unsigned int factors[LIQUID_MAX_FACTORS];
        unsigned int num_factors=0;
        liquid_factor(_nfft-1,factors,&num_factors);
        
        if (num_factors > 2) {
            // use Rader's algorithm (type I) for size _nfft-1
            printf("Rader's algorithm, Type I > %u\n", _nfft-1);
            liquid_fft_plan(_nfft-1, _level+1);
            return;

        } else {
            // use Rader's algorithm (type II) for radix-2
            // nfft_prime = 2 ^ nextpow2(2*nfft - 4)
            unsigned int nfft_prime = 1 << liquid_nextpow2(2*_nfft-4);
            printf("Rader's algorithm, Type II > %u\n", nfft_prime);
            liquid_fft_plan(nfft_prime, _level+1);
            return;
        }
    }

    // use Cooley-Tukey algorithm
    printf("Cooley-Tukey\n");

    // compute prime factors of _nfft
    unsigned int factors[LIQUID_MAX_FACTORS];
    unsigned int num_factors=0;
    liquid_factor(_nfft,factors,&num_factors);

    for (i=0; i<num_factors; i++)
        liquid_fft_plan(factors[i], _level+1);
}

