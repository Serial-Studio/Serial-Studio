//
// sandbox/matched_filter_cfo_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "matched_filter_cfo_test.m"

// print usage/help message
void usage()
{
    printf("matched_filter_cfo_test options:\n");
    printf("  u/h   : print usage/help\n");
    printf("  t     : filter type: [rrcos], rkaiser, arkaiser, hM3\n");
    printf("  k     : filter samples/symbol, k >= 2, default: 2\n");
    printf("  m     : filter delay (symbols), m >= 1, default: 3\n");
    printf("  b     : filter excess bandwidth factor, 0 < b < 1, default: 0.5\n");
    printf("  n     : number of symbols, default: 16\n");
    printf("  P     : carrier phase offset, default: 0\n");
    printf("  F     : carrier frequency offset, default: 0.8\n");
    printf("  c     : compensate for carrier frequency offset in matched filter? default: no\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int k=2;       // samples/symbol
    unsigned int m=4;       // symbol delay
    float beta=0.3f;        // excess bandwidth factor
    float phi = 0.0f;       // carrier phase offset
    float dphi = 0.8f;      // carrier frequency offset
    unsigned int num_symbols=128;
    int cfo_compensation=0; // compensate for carrier offset in matched filter?
    enum {
        FILTER_RRCOS=0,
        FILTER_RKAISER,
        FILTER_ARKAISER,
        FILTER_hM3
    } ftype = 0;            // filter prototype

    int dopt;
    while ((dopt = getopt(argc,argv,"uht:k:m:b:n:P:F:c")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();            return 0;
        case 't':
            if (strcmp(optarg,"rrcos")==0) {
                ftype = FILTER_RRCOS;
            } else if (strcmp(optarg,"rkaiser")==0) {
                ftype = FILTER_RKAISER;
            } else if (strcmp(optarg,"arkaiser")==0) {
                ftype = FILTER_ARKAISER;
            } else if (strcmp(optarg,"hM3")==0) {
                ftype = FILTER_hM3;
            } else {
                fprintf(stderr,"error: %s, unknown filter type '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        case 'k':   k = atoi(optarg);           break;
        case 'm':   m = atoi(optarg);           break;
        case 'b':   beta = atof(optarg);        break;
        case 'n':   num_symbols = atoi(optarg); break;
        case 'P':   phi = atof(optarg);         break;
        case 'F':   dphi = atof(optarg);        break;
        case 'c':   cfo_compensation=1;         break;
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

    switch (ftype) {
    case FILTER_RRCOS:
        liquid_firdes_rrcos(k,m,beta,0,h);
        break;
    case FILTER_RKAISER:
        liquid_firdes_rkaiser(k,m,beta,0,h);
        break;
    case FILTER_ARKAISER:
        liquid_firdes_arkaiser(k,m,beta,0,h);
        break;
    case FILTER_hM3:
        liquid_firdes_hM3(k,m,beta,0,h);
        break;
    default:
        // should never get to this point
        fprintf(stderr,"error: %s, invalid filter type\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int num_samples = num_symbols*k;

    // generate receive filter coefficients (reverse of transmit)
    float complex gc[h_len];
    unsigned int i;
    for (i=0; i<h_len; i++)
        gc[i] = h[h_len-i-1];
    // compensate for carrier frequency offset in matched filter?
    if (cfo_compensation) {
        for (i=0; i<h_len; i++)
            gc[i] *= cexpf(_Complex_I*dphi*i);
    }

    // create interpolator and decimator
    firinterp_crcf interp = firinterp_crcf_create(k,h,h_len);
    firdecim_cccf  decim  = firdecim_cccf_create(k,gc,h_len);

    // generate signal
    float complex sym_in[num_symbols];
    float complex y[num_samples];
    float complex sym_out[num_symbols];

    for (i=0; i<h_len; i++)
        printf("h(%3u) = %12.8f;\n", i+1, h[i]);

    // interpolation
    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        sym_in[i] = cexpf(_Complex_I*( 0.25f*M_PI + 0.5f*M_PI*(rand()%4)) );

        // interpolate
        firinterp_crcf_execute(interp, sym_in[i], &y[i*k]);

#if 0
        printf("  %3u : %8.5f + j*%8.5f\n", i, crealf(sym_in[i]), cimagf(sym_in[i]));
#endif
    }

    // channel
    for (i=0; i<num_samples; i++) {
        y[i] *= cexpf(_Complex_I*(phi + dphi*i));
    }

    // decimation
    for (i=0; i<num_symbols; i++) {
        // decimate
        firdecim_cccf_execute(decim, &y[i*k], &sym_out[i]);

        // normalize output
        sym_out[i] /= k;

        // compensate for carrier offset
        sym_out[i] *= cexpf(-_Complex_I*(dphi*i*k));

        // compensate for consequental carrier phase offset
        if (!cfo_compensation)
            sym_out[i] *= cexpf(_Complex_I*dphi*m*k);

#if 0
        printf("  %3u : %8.5f + j*%8.5f", i, crealf(sym_out[i]), cimagf(sym_out[i]));
        if (i>=2*m) printf(" *\n");
        else        printf("\n");
#endif
    }

    // clean up objects
    firinterp_crcf_destroy(interp);
    firdecim_cccf_destroy(decim);


    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"beta = %12.8f;\n", beta);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = k*num_symbols;\n");

    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++)
        fprintf(fid," y(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));

    fprintf(fid,"s = zeros(1,num_symbols);\n");
    for (i=0; i<num_symbols; i++)
        fprintf(fid," s(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(real(s), imag(s), 'x');\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"xlabel('I');\n");
    fprintf(fid,"ylabel('Q');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    
    printf("done.\n");
    return 0;
}

