//
// firinterp_crcf_example.c
//
// This example demonstrates the firinterp object (interpolator) interface.
// Data symbols are generated and then interpolated according to a
// finite impulse response Nyquist filter.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firinterp_crcf_example.m"

// print usage/help message
void usage()
{
    printf("firinterp_crcf_example:\n");
    printf("  -h         : print usage/help\n");
    printf("  -k <s/sym> : samples/symbol (interp factor), k > 1, default: 4\n");
    printf("  -m <delay> : filter delay (symbols), m > 0,         default: 3\n");
    printf("  -s <atten> : filter stop-band attenuation [dB],     default: 60\n");
    printf("  -n <num>   : number of data symbols,                default: 16\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int    k        = 4;       // samples/symbol
    unsigned int    m        = 3;       // filter delay
    float           As       = 60.0f;   // filter stop-band attenuation
    unsigned int    num_syms = 16;      // number of data symbols

    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:s:n:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();                          return 0;
        case 'k': k = atoi(optarg);                 break;
        case 'm': m = atoi(optarg);                 break;
        case 's': As = atof(optarg);                break;
        case 'n': num_syms = atoi(optarg);  break;
        default:
            exit(1);
        }
    }

    // validate options
    if (k < 2) {
        fprintf(stderr,"error: %s, interp factor must be greater than 1\n", argv[0]);
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: %s, filter delay must be greater than 0\n", argv[0]);
        exit(1);
    } else if (num_syms < 1) {
        fprintf(stderr,"error: %s, must have at least one data symbol\n", argv[0]);
        usage();
        return 1;
    }

    // derived values
    unsigned int num_syms_total = num_syms + 2*m;   // total symbols (w/ delay)
    unsigned int num_samples    = k*num_syms_total; // total samples

    // create interpolator from prototype
    firinterp_crcf q = firinterp_crcf_create_kaiser(k,m,As);

    // generate input signal and interpolate
    float complex x[num_syms_total];   // input symbols
    float complex y[num_samples];   // output samples
    unsigned int i;
    for (i=0; i<num_syms; i++) {
        x[i] = (rand() % 2 ? 1.0f : -1.0f) +
               (rand() % 2 ? 1.0f : -1.0f) * _Complex_I;
    }

    // pad end of sequence with zeros
    for (i=num_syms; i<num_syms_total; i++)
        x[i] = 0.0f;

    // interpolate symbols
    for (i=0; i<num_syms_total; i++)
        firinterp_crcf_execute(q, x[i], &y[k*i]);

    // destroy interpolator object
    firinterp_crcf_destroy(q);

    // print results to screen
    printf("x(t) :\n");
    for (i=0; i<num_syms_total; i++)
        printf("  x(%4u) = %8.4f + j*%8.4f;\n", i, crealf(x[i]), cimagf(x[i]));

    printf("y(t) :\n");
    for (i=0; i<num_samples; i++) {
        printf("  y(%4u) = %8.4f + j*%8.4f;", i, crealf(y[i]), cimagf(y[i]));
        if ( (i >= k*m) && ((i%k)==0))
            printf(" **\n");
        else
            printf("\n");
    }

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"num_syms_total = %u;\n", num_syms_total);
    fprintf(fid,"num_samples = k*num_syms_total;\n");
    fprintf(fid,"x = zeros(1,num_syms_total);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");

    for (i=0; i<num_syms_total; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));

    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    fprintf(fid,"\n\n");
    fprintf(fid,"tx = [0:(num_syms_total-1)];\n");
    fprintf(fid,"ty = [0:(num_samples-1)]/k - m;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"    plot(ty,real(y),'-',tx,real(x),'s');\n");
    fprintf(fid,"    xlabel('time');\n");
    fprintf(fid,"    ylabel('real');\n");
    fprintf(fid,"    grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"    plot(ty,imag(y),'-',tx,imag(x),'s');\n");
    fprintf(fid,"    xlabel('time');\n");
    fprintf(fid,"    ylabel('imag');\n");
    fprintf(fid,"    grid on;\n");

    fclose(fid);
    printf("results written to %s.\n",OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
