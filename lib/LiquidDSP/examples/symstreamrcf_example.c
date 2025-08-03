// demonstrate arbitrary rate symstreamrcf object
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symstreamrcf_example.m"

int main()
{
    // symstream parameters
    int          ftype       = LIQUID_FIRFILT_ARKAISER;
    float        bw          =     0.23456789f;
    unsigned int m           =     9;
    float        beta        = 0.30f;
    int          ms          = LIQUID_MODEM_QPSK;

    // spectral periodogram options
    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_samples =  80000;  // number of samples
    spgramcf periodogram = spgramcf_create_default(nfft);

    // create stream generator
    symstreamrcf gen = symstreamrcf_create_linear(ftype,bw,m,beta,ms);
    symstreamrcf_set_gain(gen, sqrtf(bw));
    symstreamrcf_print(gen);

    // create buffer for storing output
    unsigned int buf_len = 1024;
    float complex buf[buf_len];

    // generate samples
    unsigned int total_samples = 0;
    while (total_samples < num_samples) {
        // write samples to buffer
        symstreamrcf_write_samples(gen, buf, buf_len);

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
    symstreamrcf_destroy(gen);
    spgramcf_destroy(periodogram);

    // export output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"f    = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H    = zeros(1,nfft);\n");
    unsigned int i;
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

