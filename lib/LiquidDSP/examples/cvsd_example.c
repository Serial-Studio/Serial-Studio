//
// cvsd_example.c
//
// Continuously-variable slope delta example, sinusoidal input.
// This example demonstrates the CVSD audio encoder interface, and
// its response to a sinusoidal input.  The output distortion
// ratio is computed, and the time-domain results are written to
// a file.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "cvsd_example.m"

// print usage/help message
void usage()
{
    printf("cvsd_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  n     : number of samples, default: 512\n");
    printf("  f     : input signal frequency, default: 0.02\n");
    printf("  b     : cvsd param: num-bits, default: 3\n");
    printf("  z     : cvsd param: zeta, default: 1.5\n");
    printf("  a     : cvsd param: alpha, default: 0.95\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int n=512;     // number of samples
    float fc = 0.02;        // input signal frequency
    unsigned int nbits=3;   // number of adjacent bits to observe
    float zeta=1.5f;        // slope adjustment multiplier
    float alpha = 0.95;     // pre-/post-filter coefficient

    int dopt;
    while ((dopt = getopt(argc,argv,"uhn:f:b:z:a:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();              return 0;
        case 'n': n = atoi(optarg);     break;
        case 'f': fc = atof(optarg);    break;
        case 'b': nbits = atoi(optarg); break;
        case 'z': zeta = atof(optarg);  break;
        case 'a': alpha = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    unsigned int i;

    // data arrays
    float x[n];             // input time series
    unsigned char b[n];     // encoded bit pattern
    float y[n];             // reconstructed time series

    // create cvsd codecs
    cvsd cvsd_encoder = cvsd_create(nbits, zeta, alpha);
    cvsd cvsd_decoder = cvsd_create(nbits, zeta, alpha);
    cvsd_print(cvsd_encoder);

    // generate input time series
    for (i=0; i<n; i++)
        x[i] = sinf(2.0f*M_PI*fc*i) * liquid_hamming(i,n);

    // encode time series
    for (i=0; i<n; i++)
        b[i] = cvsd_encode(cvsd_encoder, x[i]);

    // compute reconstructed time series, RMS error
    float rmse=0.0f;
    for (i=0; i<n; i++) {
        y[i] = cvsd_decode(cvsd_decoder, b[i]);

        printf("%1u ", b[i]);
        if ( ((i+1)%32) == 0 )
            printf("\n");

        float e = x[i]-y[i];
        rmse += e*e;
    }

    rmse = sqrtf(rmse/n);

    printf("\n");
    printf("signal/distortion: %8.2f dB\n", -20*log10f(rmse));

    // destroy cvsd objects
    cvsd_destroy(cvsd_encoder);
    cvsd_destroy(cvsd_decoder);

    // 
    // export results to file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");

    fprintf(fid,"n=%u;\n", n);
    fprintf(fid,"x=zeros(1,n);\n");
    fprintf(fid,"y=zeros(1,n);\n");

    for (i=0; i<n; i++) {
        fprintf(fid,"x(%3u) = %12.4e;\n", i+1, x[i]);
        fprintf(fid,"y(%3u) = %12.4e;\n", i+1, y[i]);
    }

    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(1:n,x,1:n,y);\n");
    fprintf(fid,"xlabel('time [sample index]');\n");
    fprintf(fid,"ylabel('signal');\n");
    fprintf(fid,"legend('audio input','cvsd output',1);\n");

    // close debug file
    fclose(fid);
    printf("results wrtten to %s\n", OUTPUT_FILENAME);
    printf("done.\n");

    return 0;
}

