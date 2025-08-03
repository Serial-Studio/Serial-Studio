//
// symstreamcf_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symstreamcf_example.m"

int main()
{
    // symstream parameters
    int          ftype       = LIQUID_FIRFILT_ARKAISER;
    unsigned int k           =     4;
    unsigned int m           =     9;
    float        beta        = 0.30f;
    int          ms          = LIQUID_MODEM_QPSK;

    // spectral periodogram options
    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_samples =  80000;  // number of samples

    unsigned int i;

    // create spectral periodogram
    spgramcf periodogram = spgramcf_create_default(nfft);

    unsigned int buf_len = 1024;
    float complex buf[buf_len];

    // create stream generator
    symstreamcf gen = symstreamcf_create_linear(ftype,k,m,beta,ms);
    symstreamcf_print(gen);

    unsigned int total_samples = 0;
    while (total_samples < num_samples) {
        // write samples to buffer
        symstreamcf_write_samples(gen, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(periodogram, buf, buf_len);
        
        // accumulated samples
        total_samples += buf_len;
    }
    printf("total samples: %u\n", total_samples);

    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    symstreamcf_destroy(gen);
    spgramcf_destroy(periodogram);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"f    = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H    = zeros(1,nfft);\n");
    
    for (i=0; i<nfft; i++)
        fprintf(fid,"H(%6u) = %12.4e;\n", i+1, psd[i]);

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f, H, '-', 'LineWidth',1.5);\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-0.5 0.5 -120 20]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

