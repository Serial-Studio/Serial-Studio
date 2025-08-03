//
// symsync_crcf_example.c
//
// This example demonstrates the basic principles of the symbol timing
// recovery family of objects, specifically symsync_crcf. A set of random
// QPSK symbols are generated and interpolated with a timing offset. The
// resulting signal is run through the symsync_crcf object which applies a
// matched filter and recovers timing producing a clean constellation.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symsync_crcf_example.m"

// print usage/help message
void usage()
{
    printf("symsync_crcf_example [options]\n");
    printf("  h     : print help\n");
    printf("  k     : filter samples/symbol, default: 2\n");
    printf("  m     : filter delay (symbols), default: 5\n");
    printf("  b     : filter excess bandwidth, default: 0.5\n");
    printf("  B     : filter polyphase banks, default: 32\n");
    printf("  n     : number of symbols, default: 400\n");
}

int main(int argc, char*argv[]) {
    srand(time(NULL));

    // options
    unsigned int k           = 2;       // samples/symbol (input)
    unsigned int m           = 5;       // filter delay (symbols)
    float        beta        = 0.5f;    // filter excess bandwidth factor
    unsigned int num_filters = 32;      // number of filters in the bank
    unsigned int num_symbols = 400;     // number of data symbols

    int dopt;
    while ((dopt = getopt(argc,argv,"hk:m:b:B:s:n:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                        return 0;
        case 'k':   k           = atoi(optarg);     break;
        case 'm':   m           = atoi(optarg);     break;
        case 'b':   beta        = atof(optarg);     break;
        case 'B':   num_filters = atoi(optarg);     break;
        case 'n':   num_symbols = atoi(optarg);     break;
        default:
            exit(1);
        }
    }

    // validate input
    if (k < 2) {
        fprintf(stderr,"error: k (samples/symbol) must be at least 2\n");
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: m (filter delay) must be greater than 0\n");
        exit(1);
    } else if (beta <= 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: beta (excess bandwidth factor) must be in (0,1]\n");
        exit(1);
    } else if (num_filters == 0) {
        fprintf(stderr,"error: number of polyphase filters must be greater than 0\n");
        exit(1);
    } else if (num_symbols == 0) {
        fprintf(stderr,"error: number of symbols must be greater than 0\n");
        exit(1);
    }

    unsigned int i;

    // static values
    liquid_firfilt_type ftype = LIQUID_FIRFILT_ARKAISER;
    float bandwidth = 0.02f;    // loop filter bandwidth
    float dt = -0.5f;           // fractional sample offset

    // derived values
    unsigned int num_samples = k*num_symbols;

    float complex s[num_symbols];       // data symbols
    float complex x[num_samples];       // interpolated samples
    float complex y[num_symbols + 64];  // synchronized symbols

    // generate random QPSK symbols
    for (i=0; i<num_symbols; i++) {
        // random signal (QPSK)
        s[i] = (rand() % 2 ? -M_SQRT1_2 : M_SQRT1_2) +
               (rand() % 2 ? -M_SQRT1_2 : M_SQRT1_2) * _Complex_I;
    }

    // design interpolating filter with 'dt' samples of delay
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype,k,m,beta,dt);

    // run interpolator
    firinterp_crcf_execute_block(interp, s, num_symbols, x);

    // destroy interpolator
    firinterp_crcf_destroy(interp);

    // create symbol synchronizer
    symsync_crcf sync = symsync_crcf_create_rnyquist(ftype, k, m, beta, num_filters);
    
    // set bandwidth
    symsync_crcf_set_lf_bw(sync,bandwidth);

    // execute on entire block of samples
    unsigned int ny=0;
    symsync_crcf_execute(sync, x, num_samples, y, &ny);

    // destroy synchronizer
    symsync_crcf_destroy(sync);

    // print last several symbols to screen
    printf("output symbols:\n");
    printf("  ...\n");
    for (i=ny-10; i<ny; i++)
        printf("  sym_out(%2u) = %8.4f + j*%8.4f;\n", i+1, crealf(y[i]), cimagf(y[i]));

    //
    // export output file
    //

    FILE* fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"close all;\n");
    fprintf(fid,"clear all;\n");

    fprintf(fid,"ny=%u;\n",ny);

    for (i=0; i<ny; i++)
        fprintf(fid,"y(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        
    fprintf(fid,"i0 = 1:round(0.5*ny);\n");
    fprintf(fid,"i1 = round(0.5*ny):ny;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(real(y(i0)),imag(y(i0)),'x','MarkerSize',4,'Color',[1 1 1]*0.7);\n");
    fprintf(fid,"plot(real(y(i1)),imag(y(i1)),'o','MarkerSize',4,'Color',[0 0.25 0.5]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.6);\n");
    fprintf(fid,"xlabel('In-phase');\n");
    fprintf(fid,"ylabel('Quadrature');\n");
    fprintf(fid,"legend(['first 50%%'],['last 50%%'],'location','northeast');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    printf("done.\n");
    return 0;
}
