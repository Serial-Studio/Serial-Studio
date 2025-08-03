//
// nyquist_filter_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "nyquist_filter_example.m"

// print usage/help message
void usage()
{
    printf("nyquist_filter_example options:\n");
    printf("  u/h   : print usage/help\n");
    printf("  t     : filter type: [kaiser], pm, rcos, fexp, fsech, farcsech\n");
    printf("  k     : filter samples/symbol, k >= 2, default: 2\n");
    printf("  m     : filter delay (symbols), m >= 1, default: 4\n");
    printf("  b     : filter excess bandwidth factor, 0 < b < 1, default: 0.33\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int k=2;   // samples/symbol
    unsigned int m=4;   // symbol delay
    float beta=0.33f;   // excess bandwidth factor
    int ftype = LIQUID_FIRFILT_RCOS;

    int dopt;
    while ((dopt = getopt(argc,argv,"uht:k:m:b:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':
            usage();
            return 0;
        case 't':
            if (strcmp(optarg,"kaiser")==0) {
                ftype = LIQUID_FIRFILT_KAISER;
            } else if (strcmp(optarg,"pm")==0) {
                ftype = LIQUID_FIRFILT_PM;
            } else if (strcmp(optarg,"rcos")==0) {
                ftype = LIQUID_FIRFILT_RCOS;
            } else if (strcmp(optarg,"fexp")==0) {
                ftype = LIQUID_FIRFILT_FEXP;
            } else if (strcmp(optarg,"fsech")==0) {
                ftype = LIQUID_FIRFILT_FSECH;
            } else if (strcmp(optarg,"farcsech")==0) {
                ftype = LIQUID_FIRFILT_FARCSECH;
            } else {
                fprintf(stderr,"error: %s, unknown filter type '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        case 'k':   k = atoi(optarg);           break;
        case 'm':   m = atoi(optarg);           break;
        case 'b':   beta = atof(optarg);        break;
        default:
            exit(1);
        }
    }

    if (k < 2) {
        fprintf(stderr,"error: %s, k must be at least 2\n", argv[0]);
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: %s, m must be at least 1\n", argv[0]);
        exit(1);
    } else if (beta <= 0.0f || beta >= 1.0f) {
        fprintf(stderr,"error: %s, beta must be in (0,1)\n", argv[0]);
        exit(1);
    }

    // initialize objects
    unsigned int h_len = 2*k*m+1;
    float h[h_len];

    // design the filter
    liquid_firdes_prototype(ftype,k,m,beta,0,h);

    // print the coefficients to the screen
    unsigned int i;
    for (i=0; i<h_len; i++)
        printf("h(%3u) = %12.8f\n", i+1, h[i]);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"beta = %12.8f;\n", beta);
    fprintf(fid,"h_len = 2*k*m+1;\n");

    fprintf(fid,"h = zeros(1,h_len);\n");
    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%3u) = %20.8e;\n", i+1, h[i]);
    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H = 20*log10(abs(fftshift(fft(h/k,nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,H,'-','LineWidth',2,...\n");
    fprintf(fid,"     [0.5/k],[-20*log10(2)],'or',...\n");
    fprintf(fid,"     [0.5/k*(1-beta) 0.5/k*(1-beta)],[-100 10],'-r',...\n");
    fprintf(fid,"     [0.5/k*(1+beta) 0.5/k*(1+beta)],[-100 10],'-r');\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD');\n");
    fprintf(fid,"axis([-0.5 0.5 -100 10]);\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    
    printf("done.\n");
    return 0;
}

