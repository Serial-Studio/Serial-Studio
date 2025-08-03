//
// firdespm_example.c
//
// This example demonstrates finite impulse response filter design
// using the Parks-McClellan algorithm.
//
// SEE ALSO: firdes_kaiser_example.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firdespm_example.m"

int main(int argc, char*argv[])
{
    // filter design parameters
    unsigned int h_len = 91;
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    unsigned int num_bands = 4;
    float bands[8]   = {0.00f, 0.10f,
                        0.12f, 0.18f,
                        0.20f, 0.30f,
                        0.31f, 0.50f};

    float des[4]     = {1.0f, 0.0f, 0.1f, 0.0f};
    float weights[4] = {1.0f, 4.0f, 8.0f, 4.0f};
    liquid_firdespm_wtype wtype[4] = {LIQUID_FIRDESPM_FLATWEIGHT,
                                      LIQUID_FIRDESPM_FLATWEIGHT,
                                      LIQUID_FIRDESPM_FLATWEIGHT,
                                      LIQUID_FIRDESPM_EXPWEIGHT};

    unsigned int i;
    float h[h_len];
    firdespm_run(h_len,num_bands,bands,des,weights,wtype,btype,h);

    // print coefficients
    for (i=0; i<h_len; i++)
        printf("h(%4u) = %16.12f;\n", i+1, h[i]);

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"h_len=%u;\n", h_len);

    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%4u) = %20.8e;\n", i+1, h[i]);

    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"H=20*log10(abs(fftshift(fft(h,nfft))));\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure; plot(f,H,'Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"title('Filter design (firdespm)');\n");
    fprintf(fid,"axis([-0.5 0.5 -60 5]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

