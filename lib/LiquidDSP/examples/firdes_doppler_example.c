// This example demonstrates finite impulse response Doppler filter design
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firdes_doppler_example.m"

int main()
{
    // options
    float        fd     = 0.2f;  // Normalized Doppler frequency
    float        K      = 10.0f; // Rice fading factor
    float        theta  = 0.0f;  // LoS component angle of arrival
    unsigned int h_len  = 161;   // filter length

    // generate the filter
    float h[h_len];
    liquid_firdes_doppler(h_len,fd,K,theta,h);

    // output to file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"h_len=%u;\n",h_len);
    unsigned int i;
    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%4u) = %12.4e;\n", i+1, h[i]);

    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"H=20*log10(abs(fftshift(fft(h,nfft))));\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure; plot(f,H,'Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"title(['Doppler filter design, fd:%.3f, K:%.1f, theta:%.3f, n:%u']);\n",
            fd, K, theta, h_len);
    fprintf(fid,"xlim([-0.5 0.5]);\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);
    return 0;
}

