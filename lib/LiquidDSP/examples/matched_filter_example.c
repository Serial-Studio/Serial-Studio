//
// matched_filter_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "matched_filter_example.m"

// print usage/help message
void usage()
{
    printf("matched_filter_example options:\n");
    printf("  u/h   : print usage/help\n");
    printf("  t     : filter type: [rrcos], rkaiser, arkaiser, hm3, gmsk, fexp, fsech, farcsech\n");
    printf("  k     : filter samples/symbol, k >= 2, default: 2\n");
    printf("  m     : filter delay (symbols), m >= 1, default: 3\n");
    printf("  b     : filter excess bandwidth factor, 0 < b < 1, default: 0.5\n");
    printf("  n     : number of symbols, default: 16\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int k=2;   // samples/symbol
    unsigned int m=3;   // symbol delay
    float beta=0.7f;    // excess bandwidth factor
    unsigned int num_symbols=16;
    int ftype_tx = LIQUID_FIRFILT_RRC;
    int ftype_rx = LIQUID_FIRFILT_RRC;

    int dopt;
    while ((dopt = getopt(argc,argv,"uht:k:m:b:n:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();            return 0;
        case 't':
            if (strcmp(optarg,"gmsk")==0) {
                ftype_tx = LIQUID_FIRFILT_GMSKTX;
                ftype_rx = LIQUID_FIRFILT_GMSKRX;
            } else {
                ftype_tx = liquid_getopt_str2firfilt(optarg);
                ftype_rx = liquid_getopt_str2firfilt(optarg);
            }
            if (ftype_tx == LIQUID_FIRFILT_UNKNOWN) {
                fprintf(stderr,"error: %s, unknown filter type '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        case 'k':   k = atoi(optarg);           break;
        case 'm':   m = atoi(optarg);           break;
        case 'b':   beta = atof(optarg);        break;
        case 'n':   num_symbols = atoi(optarg); break;
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

    unsigned int i;

    // derived values
    unsigned int num_samples = num_symbols*k;
    unsigned int h_len  = 2*k*m+1;               // transmit/receive filter length
    unsigned int hc_len = 4*k*m+1;               // composite filter length

    // arrays
    float ht[h_len];    // transmit filter
    float hr[h_len];    // receive filter
    float hc[hc_len];   // composite filter

    // design the filter(s)
    liquid_firdes_prototype(ftype_tx, k, m, beta, 0, ht);
    liquid_firdes_prototype(ftype_rx, k, m, beta, 0, hr);

    for (i=0; i<h_len; i++) printf("ht(%3u) = %12.8f;\n", i+1, ht[i]);
    for (i=0; i<h_len; i++) printf("hr(%3u) = %12.8f;\n", i+1, hr[i]);

#if 0
    // generate receive filter coefficients (reverse of transmit)
    float hr[h_len];
    for (i=0; i<h_len; i++)
        hr[i] = ht[h_len-i-1];
#endif

    // compute composite filter response
    for (i=0; i<4*k*m+1; i++) {
        int lag = (int)i - (int)(2*k*m);
        hc[i] = liquid_filter_crosscorr(ht,h_len, hr,h_len, lag);
    }

    // compute filter inter-symbol interference
    float rxy0 = liquid_filter_crosscorr(ht,h_len, hr,h_len, 0);
    float isi_rms=0;
    for (i=1; i<2*m; i++) {
        float e = liquid_filter_crosscorr(ht,h_len, hr,h_len, i*k) / rxy0;
        isi_rms += e*e;
    }
    isi_rms = sqrtf( isi_rms / (float)(2*m-1) );
    printf("  isi (rms) : %12.8f dB\n", 20*log10f(isi_rms));

    // compute relative stop-band energy
    unsigned int nfft = 2048;
    float As = liquid_filter_energy(ht, h_len, 0.5f*(1.0f + beta)/(float)k, nfft);
    printf("  As        : %12.8f dB\n", 20*log10f(As));

    // generate signal
    float sym_in[num_symbols];
    float y[num_samples];
    float sym_out[num_symbols];

    // create interpolator and decimator
    firinterp_rrrf interp = firinterp_rrrf_create(k, ht, h_len);
    firdecim_rrrf  decim  = firdecim_rrrf_create( k, hr, h_len);

    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        sym_in[i] = (rand() % 2) ? 1.0f : -1.0f;

        // interpolate
        firinterp_rrrf_execute(interp, sym_in[i], &y[i*k]);

        // decimate
        firdecim_rrrf_execute(decim, &y[i*k], &sym_out[i]);

        // normalize output
        sym_out[i] /= k;

        printf("  %3u : %8.5f", i, sym_out[i]);
        if (i>=2*m) printf(" *\n");
        else        printf("\n");
    }

    // clean up objects
    firinterp_rrrf_destroy(interp);
    firdecim_rrrf_destroy(decim);

    //
    // export results
    //
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
        fprintf(fid," y(%3u) = %12.8f;\n", i+1, y[i]);

    for (i=0; i<h_len;  i++) fprintf(fid,"ht(%3u) = %20.8e;\n", i+1, ht[i]);
    for (i=0; i<h_len;  i++) fprintf(fid,"hr(%3u) = %20.8e;\n", i+1, hr[i]);
    for (i=0; i<hc_len; i++) fprintf(fid,"hc(%3u) = %20.8e;\n", i+1, hc[i]);

    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"Ht = 20*log10(abs(fftshift(fft(ht/k,   nfft))));\n");
    fprintf(fid,"Hr = 20*log10(abs(fftshift(fft(hr/k,   nfft))));\n");
    fprintf(fid,"Hc = 20*log10(abs(fftshift(fft(hc/k^2, nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,Hc,'-','LineWidth',2,...\n");
    fprintf(fid,"     [0.5/k],[-6],'or',...\n");
    fprintf(fid,"     [0.5/k*(1-beta) 0.5/k*(1-beta)],[-100 10],'-r',...\n");
    fprintf(fid,"     [0.5/k*(1+beta) 0.5/k*(1+beta)],[-100 10],'-r');\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD');\n");
    fprintf(fid,"axis([-0.5 0.5 -100 10]);\n");
    fprintf(fid,"grid on;\n");

    // compute composite filter
    fprintf(fid,"figure;\n");
    fprintf(fid,"hc = conv(ht,hr)/k;\n");
    fprintf(fid,"t = [(-2*k*m):(2*k*m)]/k;\n");
    fprintf(fid,"i0 = [0:k:4*k*m]+1;\n");
    fprintf(fid,"plot(t,    hc,    '-s',...\n");
    fprintf(fid,"     t(i0),hc(i0),'or');\n");
    fprintf(fid,"xlabel('symbol index');\n");
    fprintf(fid,"ylabel('matched filter response');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    
    printf("done.\n");
    return 0;
}

