//
// shadowing_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "shadowing_test.m"

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // properties
    unsigned int num_samples = 80e3;
    float        sigma       = 1.0f;
    float        fd          = 0.02f;
    unsigned int nfft        = 2400;    // spectral periodogram FFT size

    // generate Doppler filter
#if 1
    float alpha = fd;
    float a[2] = {1.0f, alpha-1.0f};
    float b[2] = {alpha, 0};
    iirfilt_rrrf filter = iirfilt_rrrf_create(b,2,a,2);
#else
    iirfilt_rrrf filter = iirfilt_rrrf_create_lowpass(11,fd);
#endif

    unsigned int i;

    // output channel gain
    float G[num_samples];

    // generate samples
    for (i=0; i<num_samples; i++) {
        iirfilt_rrrf_execute(filter, randnf()*sigma, &G[i]);
        G[i] /= fd * 6.9f;
    }

    // destroy filter
    iirfilt_rrrf_destroy(filter);

    // estimate power spectral density output
    float psd[nfft];
    spgramf_estimate_psd(nfft, G, num_samples, psd);

    //
    // export output file
    //

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"close all;\nclear all;\n\n");

    fprintf(fid,"num_samples=%u;\n",num_samples);

    for (i=0; i<num_samples; i++)
        fprintf(fid,"G(%3u) = %12.8f;\n", i+1, G[i]);

    fprintf(fid,"g = 10.^[G/20];\n");

    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"phi(%6u) = %12.4e;\n", i+1, psd[i]);

    fprintf(fid,"figure('Color','white','position',[500 500 800 800]);\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"plot(G,'-','LineWidth',1,'Color',[0 0.2 0.5]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('In-phase');\n");
    fprintf(fid,"  ylabel('Gain [dB]');\n");
    fprintf(fid,"  axis([0 5000 -2 2]);\n");
    fprintf(fid,"  title('');\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  hist(G,50);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(f,phi,'LineWidth',2,'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  axis([-0.5 0.5 -40 40]);\n");
    fprintf(fid,"  grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    printf("done.\n");
    return 0;
}
