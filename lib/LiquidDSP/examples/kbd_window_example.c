//
// kbd_window_example.c
//
// Kaiser-Bessel derived window example
//

#include <stdio.h>

#include "liquid.h"

#define OUTPUT_FILENAME "kbd_window_example.m"

int main() {
    // options
    unsigned int n=64;      // window length
    float beta = 20.0f;     // Kaiser beta factor

    unsigned int i;
    float w[n];
    liquid_kbd_window(n,beta,w);

    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"n=%u;\n",n);

    for (i=0; i<n; i++) {
        fprintf(fid,"w(%4u) = %12.4e;\n", i+1, w[i]);
        printf("w[%4u] = %12.4e;\n", i, w[i]);
    }   

    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"W=20*log10(abs(fftshift(fft(w/sum(w),nfft))));\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,w,'Color',[0 0.25 0.5],'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('window');\n");
    fprintf(fid,"  axis([0 n-1 -0.1 1.1]);\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(f,W,'Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('normalized frequency');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fprintf(fid,"  axis([-0.5 0.5 -140 20]);\n");
    fprintf(fid,"title(['Kaiser-Bessel derived window']);\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

