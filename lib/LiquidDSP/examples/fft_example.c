//
// fft_example.c
//
// This example demonstrates the interface to the fast discrete Fourier
// transform (FFT).
// SEE ALSO: mdct_example.c
//           fct_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include "liquid.h"

// print usage/help message
void usage()
{
    printf("fft_example [options]\n");
    printf("  h     : print help\n");
    printf("  v/q   : verbose/quiet\n");
    printf("  n     : fft size, default: 16\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int nfft = 16; // transform size
    int method = 0;         // fft method (ignored)
    int verbose = 0;        // verbose output?

    int dopt;
    while ((dopt = getopt(argc,argv,"hvqn:")) != EOF) {
        switch (dopt) {
        case 'h': usage();              return 0;
        case 'v': verbose = 1;          break;
        case 'q': verbose = 0;          break;
        case 'n': nfft = atoi(optarg);  break;
        default:
            exit(1);
        }
    }

    // allocate memory arrays
    float complex * x = (float complex*) fft_malloc(nfft*sizeof(float complex));
    float complex * y = (float complex*) fft_malloc(nfft*sizeof(float complex));
    float complex * z = (float complex*) fft_malloc(nfft*sizeof(float complex));

    // initialize input
    unsigned int i;
    for (i=0; i<nfft; i++)
        x[i] = (float)i - _Complex_I*(float)i;

    // create fft plans
    fftplan pf = fft_create_plan(nfft, x, y, LIQUID_FFT_FORWARD,  method);
    fftplan pr = fft_create_plan(nfft, y, z, LIQUID_FFT_BACKWARD, method);

    // print fft plans
    fft_print_plan(pf);
    //fft_print_plan(pr);

    // execute fft plans
    fft_execute(pf);
    fft_execute(pr);

    // destroy fft plans
    fft_destroy_plan(pf);
    fft_destroy_plan(pr);

    // normalize inverse
    for (i=0; i<nfft; i++)
        z[i] /= (float) nfft;

    if (verbose) {
        // print results
        printf("original signal, x[n]:\n");
        for (i=0; i<nfft; i++)
            printf("  x[%3u] = %8.4f + j%8.4f\n", i, crealf(x[i]), cimagf(x[i]));
        printf("y[n] = fft( x[n] ):\n");
        for (i=0; i<nfft; i++)
            printf("  y[%3u] = %8.4f + j%8.4f\n", i, crealf(y[i]), cimagf(y[i]));
        printf("z[n] = ifft( y[n] ):\n");
        for (i=0; i<nfft; i++)
            printf("  z[%3u] = %8.4f + j%8.4f\n", i, crealf(z[i]), cimagf(z[i]));
    }

    // compute RMSE between original and result
    float rmse = 0.0f;
    for (i=0; i<nfft; i++) {
        float complex d = x[i] - z[i];
        rmse += crealf(d * conjf(d));
    }
    rmse = sqrtf( rmse / (float)nfft );
    printf("rmse = %12.4e\n", rmse);

    // free allocated memory
    fft_free(x);
    fft_free(y);
    fft_free(z);

    printf("done.\n");
    return 0;
}

