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
// Real even/odd DFTs (discrete cosine/sine transforms)
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <getopt.h>
#include <complex.h>

#include "liquid.internal.h"

#ifdef HAVE_FFTW3_H
#  include <fftw3.h>
#endif

// print usage/help message
void usage()
{
    printf("fft_r2r_test -- test real ffts, compare to fftw3\n");
    printf("options (default values in []):\n");
    printf("  u/h   : print usage/help\n");
    printf("  n     : number of points\n");
}

int main(int argc, char*argv[])
{
#ifdef HAVE_FFTW3_H
    unsigned int n = 32;    // transform size
    unsigned int d = 4;     // number of elements to print each line

    int dopt;
    while ((dopt = getopt(argc,argv,"uhn:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();    return 0;
        case 'n': n = atoi(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if ( n < 2 ) {
        fprintf(stderr,"error: input transform size must be at least 2\n");
        exit(1);
    }

    // create and initialize data arrays
    float x[n];
    float y[n];
    unsigned int i;
    for (i=0; i<n; i++)
        x[i] = cosf(2*M_PI*i/((float)n)) * expf(-4.0f*i*i/((float)n*n));

    // create fftw plans
    fftwf_plan plan[8];
    plan[0] = fftwf_plan_r2r_1d(n, x, y, FFTW_REDFT00, FFTW_ESTIMATE);
    plan[1] = fftwf_plan_r2r_1d(n, x, y, FFTW_REDFT10, FFTW_ESTIMATE);
    plan[2] = fftwf_plan_r2r_1d(n, x, y, FFTW_REDFT01, FFTW_ESTIMATE);
    plan[3] = fftwf_plan_r2r_1d(n, x, y, FFTW_REDFT11, FFTW_ESTIMATE);

    plan[4] = fftwf_plan_r2r_1d(n, x, y, FFTW_RODFT00, FFTW_ESTIMATE);
    plan[5] = fftwf_plan_r2r_1d(n, x, y, FFTW_RODFT10, FFTW_ESTIMATE);
    plan[6] = fftwf_plan_r2r_1d(n, x, y, FFTW_RODFT01, FFTW_ESTIMATE);
    plan[7] = fftwf_plan_r2r_1d(n, x, y, FFTW_RODFT11, FFTW_ESTIMATE);

    char plan_name[][8] = {"REDFT00",
                           "REDFT10",
                           "REDFT01",
                           "REDFT11",
                           "RODFT00",
                           "RODFT10",
                           "RODFT01",
                           "RODFT11"};

    unsigned int j;
    printf("// %u-point real even/odd dft data\n", n);
    printf("float fftdata_r2r_n%u[] = {\n    ", n);
    for (j=0; j<n; j++) {
        //printf("  %16.10f%s\n", y[j], j==(n-1) ? "};" : ",");
        printf("%16.8e", x[j]);
        if ( j==n-1 )
            printf(" ");
        else if ( ((j+1)%d)==0 )
            printf(",\n    ");
        else
            printf(", ");
    }
    printf("};\n\n");

    // execute plans and print
    for (i=0; i<8; i++) {
        fftwf_execute(plan[i]);

        printf("\n");
        printf("// %s\n", plan_name[i]);
        printf("float fftdata_r2r_%s_n%u[] = {\n    ", plan_name[i], n);
        for (j=0; j<n; j++) {
            //printf("  %16.10f%s\n", y[j], j==(n-1) ? "};" : ",");
            printf("%16.8e", y[j]);
            if ( j==n-1 )
                printf(" ");
            else if ( ((j+1)%d)==0 )
                printf(",\n    ");
            else
                printf(", ");
        }
        printf("};\n\n");
    }

    // destroy plans
    for (i=0; i<8; i++)
        fftwf_destroy_plan(plan[i]);
#else
    printf("please install fftw and try again\n");
#endif

    return 0;
}

