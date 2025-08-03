// create halfband filter using firdespm
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firdespm_halfband_example.m"

int main(int argc, char*argv[])
{
    // filter design parameters
    unsigned int m  = 4;
    float        ft = 0.4f;

    // derived values
    unsigned int h_len = 4*m + 1;
    float h[h_len];
    liquid_firdespm_halfband_ft(m, ft, h);

    // print coefficients
    unsigned int i;
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
    fprintf(fid,"figure;\n");
    fprintf(fid,"  plot(f,H,'Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('normalized frequency');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fprintf(fid,"  title('Filter design (firdespm)');\n");
    fprintf(fid,"  axis([-0.5 0.5 -180 5]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

