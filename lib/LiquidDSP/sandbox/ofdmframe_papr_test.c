// Test OFDM frame's peak-to-average power ratio (PAPR).
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid.h"

#define OUTPUT_FILENAME "ofdmframe_papr_test.m"

void usage()
{
    printf("Usage: ofdmframe_papr_test [OPTION]\n");
    printf("  h     : print help\n");
    printf("  M     : number of subcarriers (must be even), default: 64\n");
    printf("  C     : cyclic prefix length, default: 16\n");
    printf("  T     : taper length, default: 0\n");
    printf("  N     : number of data symbols, default: 1000\n");
}

// compute peak-to-average power ratio
//  _x  :   input time series
//  _n  :   number of samples
float ofdmframe_PAPR(float complex * _x,
                     unsigned int    _n);

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // options
    unsigned int M           = 64;   // number of subcarriers
    unsigned int cp_len      = 16;   // cyclic prefix length
    unsigned int taper_len   = 0;    // taper length
    unsigned int num_symbols = 1000; // number of data symbols
    modulation_scheme ms = LIQUID_MODEM_QPSK;

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hM:C:T:N:")) != EOF){
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'M': M           = atoi(optarg);   break;
        case 'C': cp_len      = atoi(optarg);   break;
        case 'T': taper_len   = atoi(optarg);   break;
        case 'N': num_symbols = atoi(optarg);   break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int frame_len = M + cp_len;

    // initialize subcarrier allocation
    unsigned char p[M];
    ofdmframe_init_default_sctype(M, p);

    // create frame generator
    ofdmframegen fg = ofdmframegen_create(M, cp_len, taper_len, p);
    ofdmframegen_print(fg);

    modemcf mod = modemcf_create(ms);

    float complex X[M];             // channelized symbols
    float complex buffer[frame_len];// output time series
    float * PAPR = (float*) malloc(num_symbols*sizeof(float));

    // histogram display
    float xmin = -1.29f + 0.70f*log2f(M);
    float xmax =  8.97f + 0.34f*log2f(M);
    unsigned int num_bins = 30;
    float bin_width = (xmax - xmin) / (num_bins);
    unsigned int hist[num_bins];
    for (i=0; i<num_bins; i++)
        hist[i] = 0;

    // modulate data symbols
    unsigned int j;
    unsigned int s;
    float PAPR_min  = 0.0f;
    float PAPR_max  = 0.0f;
    float PAPR_mean = 0.0f;
    for (i=0; i<num_symbols; i++) {
        // data symbol
        for (j=0; j<M; j++) {
            s = modemcf_gen_rand_sym(mod);
            modemcf_modulate(mod,s,&X[j]);
        }
        // generate symbol
        ofdmframegen_writesymbol(fg, X, buffer);
#if 0
        // preamble
        if      (i==0) ofdmframegen_write_S0a(fg, buffer); // S0 symbol (first)
        else if (i==1) ofdmframegen_write_S0b(fg, buffer); // S0 symbol (second)
        else if (i==2) ofdmframegen_write_S1( fg, buffer); // S1 symbol
#endif

        // compute PAPR
        PAPR[i] = ofdmframe_PAPR(buffer, frame_len);

        // compute bin index
        unsigned int index;
        float ihat = num_bins * (PAPR[i] - xmin) / (xmax - xmin);
        if (ihat < 0.0f) index = 0;
        else             index = (unsigned int)ihat;
        
        if (index >= num_bins)
            index = num_bins-1;

        hist[index]++;

        // save min/max PAPR values
        if (i==0 || PAPR[i] < PAPR_min) PAPR_min = PAPR[i];
        if (i==0 || PAPR[i] > PAPR_max) PAPR_max = PAPR[i];
        PAPR_mean += PAPR[i];
    }
    PAPR_mean /= (float)num_symbols;

    // destroy objects
    ofdmframegen_destroy(fg);
    modemcf_destroy(mod);

    // print results to screen
    // find max(hist)
    unsigned int hist_max = 0;
    for (i=0; i<num_bins; i++)
        hist_max = hist[i] > hist_max ? hist[i] : hist_max;

    printf("%8s : %6s [%6s]\n", "PAPR[dB]", "count", "prob.");
    for (i=0; i<num_bins; i++) {
        printf("%8.2f : %6u [%6.4f]", xmin + i*bin_width, hist[i], (float)hist[i] / (float)num_symbols);

        unsigned int k;
        unsigned int n = round(60 * (float)hist[i] / (float)hist_max);
        for (k=0; k<n; k++)
            printf("#");
        printf("\n");
    }
    printf("\n");
    printf("min  PAPR: %12.4f dB\n", PAPR_min);
    printf("max  PAPR: %12.4f dB\n", PAPR_max);
    printf("mean PAPR: %12.4f dB\n", PAPR_mean);

    // 
    // export output file
    //

    // count subcarrier types
    unsigned int M_data  = 0;
    unsigned int M_pilot = 0;
    unsigned int M_null  = 0;
    ofdmframe_validate_sctype(p, M, &M_null, &M_pilot, &M_data);

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"M           = %u;\n", M);
    fprintf(fid,"M_data      = %u;\n", M_data);
    fprintf(fid,"M_pilot     = %u;\n", M_pilot);
    fprintf(fid,"M_null      = %u;\n", M_null);
    fprintf(fid,"cp_len      = %u;\n", cp_len);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);

    fprintf(fid,"PAPR = zeros(1,num_symbols);\n");
    for (i=0; i<num_symbols; i++)
        fprintf(fid,"  PAPR(%6u) = %12.4e;\n", i+1, PAPR[i]);

    fprintf(fid,"\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hist(PAPR,25);\n");

    fprintf(fid,"\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"cdf = [(1:num_symbols)-0.5]/num_symbols;\n");
    fprintf(fid,"semilogy(sort(PAPR),1-cdf);\n");
    fprintf(fid,"xlabel('PAPR [dB]');\n");
    fprintf(fid,"ylabel('Complementary CDF');\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    // free allocated array
    free(PAPR);

    printf("done.\n");
    return 0;
}

// compute peak-to-average power ratio
//  _x  :   input time series
//  _n  :   number of samples
float ofdmframe_PAPR(float complex * _x,
                     unsigned int    _n)
{
    float e;
    float e_mean = 0.0f;
    float e_max  = 0.0f;

    unsigned int i;
    for (i=0; i<_n; i++) {
        // compute |_x[i]|^2
        e = crealf( _x[i] * conjf(_x[i]) );

        e_mean += e;
        e_max   = (e > e_max) ? e : e_max;
    }

    e_mean = e_mean / (float)_n;

    return 10*log10f(e_max / e_mean);
}

