// 
// cpfsk_psd_example.c
//
// This example demonstrates the differences in power spectral
// density (PSD) for different continuous-phase frequency-shift
// keying (CP-FSK) modems in liquid. Identical bit streams are fed
// into modulators with different pulse-shaping filters and the
// resulting PSDs are computed and logged to a file.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "cpfsk_psd_example.m"

// print usage/help message
void usage()
{
    printf("cpfsk_psd_example -- continuous-phase frequency-shift keying example\n");
    printf("options:\n");
    printf("  h     : print help\n");
    printf("  p     : bits/symbol,              default:  1\n");
    printf("  H     : modulation index,         default:  0.5\n");
    printf("  k     : samples/symbol,           default:  8\n");
    printf("  m     : filter delay (symbols),   default:  3\n");
    printf("  b     : filter roll-off,          default:  0.35\n");
    printf("  n     : number of data symbols,   default: 80\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int    bps         = 1;        // number of bits/symbol
    float           h           = 0.5f;     // modulation index (h=1/2 for MSK)
    unsigned int    k           = 8;        // filter samples/symbol
    unsigned int    m           = 7;        // filter delay (symbols)
    float           beta        = 0.35f;    // GMSK bandwidth-time factor
    unsigned int    num_symbols = 48000;    // number of data symbols
    unsigned int    nfft        = 2400;     // spectral periodogram FFT size

    int dopt;
    while ((dopt = getopt(argc,argv,"hp:H:k:m:b:n:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                    return 0;
        case 'p': bps         = atoi(optarg); break;
        case 'H': h           = atof(optarg); break;
        case 'k': k           = atoi(optarg); break;
        case 'm': m           = atoi(optarg); break;
        case 'b': beta        = atof(optarg); break;
        case 'n': num_symbols = atoi(optarg); break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int M = 1 << bps;              // constellation size

    // create modulators
    cpfskmod mod_0 = cpfskmod_create(bps, h, k, m, beta, LIQUID_CPFSK_SQUARE);
    cpfskmod mod_1 = cpfskmod_create(bps, h, k, m, beta, LIQUID_CPFSK_RCOS_FULL);
    cpfskmod mod_2 = cpfskmod_create(bps, h, k, m, beta, LIQUID_CPFSK_RCOS_PARTIAL);
    cpfskmod mod_3 = cpfskmod_create(bps, h, k, m, beta, LIQUID_CPFSK_GMSK);

    // buffers
    float complex buf_0[k];
    float complex buf_1[k];
    float complex buf_2[k];
    float complex buf_3[k];

    // create PSD estimators
    // spectral periodogram options

    // create spectral periodogram
    spgramcf spgram_0 = spgramcf_create_default(nfft);
    spgramcf spgram_1 = spgramcf_create_default(nfft);
    spgramcf spgram_2 = spgramcf_create_default(nfft);
    spgramcf spgram_3 = spgramcf_create_default(nfft);

    // estimate PSD for each symbol
    for (i=0; i<num_symbols; i++) {

        // generate random symbol
        unsigned int sym = rand() % M;

        // modulate
        cpfskmod_modulate(mod_0, sym, buf_0);
        cpfskmod_modulate(mod_1, sym, buf_1);
        cpfskmod_modulate(mod_2, sym, buf_2);
        cpfskmod_modulate(mod_3, sym, buf_3);
        
        // estimate PSD
        spgramcf_write(spgram_0, buf_0, k);
        spgramcf_write(spgram_1, buf_1, k);
        spgramcf_write(spgram_2, buf_2, k);
        spgramcf_write(spgram_3, buf_3, k);
    }
    
    // compute power spectral density estimate output
    float psd_0[nfft]; spgramcf_get_psd(spgram_0, psd_0);
    float psd_1[nfft]; spgramcf_get_psd(spgram_1, psd_1);
    float psd_2[nfft]; spgramcf_get_psd(spgram_2, psd_2);
    float psd_3[nfft]; spgramcf_get_psd(spgram_3, psd_3);

    // destroy modulators
    cpfskmod_destroy(mod_0);
    cpfskmod_destroy(mod_1);
    cpfskmod_destroy(mod_2);
    cpfskmod_destroy(mod_3);

    // destroy spgram objects
    spgramcf_destroy(spgram_0);
    spgramcf_destroy(spgram_1);
    spgramcf_destroy(spgram_2);
    spgramcf_destroy(spgram_3);

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"nfft        = %u;\n", nfft);

    // save power spectral density
    fprintf(fid,"psd_0 = zeros(1,nfft);\n");
    fprintf(fid,"psd_1 = zeros(1,nfft);\n");
    fprintf(fid,"psd_2 = zeros(1,nfft);\n");
    fprintf(fid,"psd_3 = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++) {
        fprintf(fid,"psd_0(%4u) = %12.8f;\n", i+1, psd_0[i] - 10*log10(k));
        fprintf(fid,"psd_1(%4u) = %12.8f;\n", i+1, psd_1[i] - 10*log10(k));
        fprintf(fid,"psd_2(%4u) = %12.8f;\n", i+1, psd_2[i] - 10*log10(k));
        fprintf(fid,"psd_3(%4u) = %12.8f;\n", i+1, psd_3[i] - 10*log10(k));
    }

    // plot PSD
    fprintf(fid,"figure;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  plot(f,psd_0,'LineWidth',1.5,'Color',[0.0 0.5 0.2]);\n");
    fprintf(fid,"  plot(f,psd_1,'LineWidth',1.5,'Color',[0.0 0.2 0.5]);\n");
    fprintf(fid,"  plot(f,psd_2,'LineWidth',1.5,'Color',[0.5 0.0 0.0]);\n");
    fprintf(fid,"  plot(f,psd_3,'LineWidth',1.5,'Color',[0.5 0.5 0.0]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"legend('square','rcos (full)','rcos (partial)','GMSK, BT=%.2f');\n", beta);
    fprintf(fid,"axis([-0.5 0.5 -80 10]);\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
