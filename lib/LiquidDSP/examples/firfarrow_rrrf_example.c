//
// firfarrow_rrrf_example.c
//
// Demonstrates the functionality of the finite impulse response Farrow
// filter for arbitrary fractional sample group delay.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firfarrow_rrrf_example.m"

// print usage/help message
void usage()
{
    printf("firfarrow_rrrf_example [options]\n");
    printf("  h     : print help\n");
    printf("  t     : fractional sample offset, t in [-0.5,0.5], default: 0.2\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int h_len       = 19;      // filter length
    unsigned int p           = 5;       // polynomial order
    float        fc          = 0.45f;   // filter cutoff
    float        As          = 60.0f;   // stop-band attenuation [dB]
    float        mu          = 0.1f;    // fractional sample delay
    unsigned int num_samples = 60;      // number of samples to evaluate

    int dopt;
    while ((dopt = getopt(argc,argv,"ht:")) != EOF) {
        switch (dopt) {
        case 'h': usage();              return 0;
        case 't': mu = atof(optarg);    break;
        default:
            exit(1);
        }
    }

    // data arrays
    float x[num_samples];   // input data array
    float y[num_samples];   // output data array

    // create and initialize Farrow filter object
    firfarrow_rrrf f = firfarrow_rrrf_create(h_len, p, fc, As);
    firfarrow_rrrf_set_delay(f, mu);

    // create low-pass filter for input signal
    iirfilt_rrrf lowpass = iirfilt_rrrf_create_lowpass(3, 0.1f);

    // push input through filter
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        // generate input (filtered noise)
        iirfilt_rrrf_execute(lowpass, randnf(), &x[i]);

        // push result through Farrow filter to add slight delay
        firfarrow_rrrf_push(f, x[i]);
        firfarrow_rrrf_execute(f, &y[i]);
    }

    // destroy Farrow and low-pass filter objects
    firfarrow_rrrf_destroy(f);
    iirfilt_rrrf_destroy(lowpass);

    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"h_len = %u;\n", h_len);
    fprintf(fid,"mu = %f;\n", mu);
    fprintf(fid,"num_samples = %u;\n", num_samples);

    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%4u) = %12.4e; y(%4u) = %12.4e;\n", i+1, x[i], i+1, y[i]);

    // plot the results
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"tx = 0:(num_samples-1);     %% input time scale\n");
    fprintf(fid,"ty = tx - (h_len-1)/2 + mu; %% output time scale\n");
    fprintf(fid,"plot(tx, x,'-s','MarkerSize',3, ...\n");
    fprintf(fid,"     ty, y,'-x','MarkerSize',3);\n");
    fprintf(fid,"legend('input','output',0);\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

