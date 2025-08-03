//
// firdespm_lowpass_example.c
//
// This example demonstrates a low-pass finite impulse response filter
// design using the Parks-McClellan algorithm.
//
// SEE ALSO: firdes_kaiser_example.c

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firdespm_lowpass_example.m"

// print usage/help message
void usage()
{
    printf("firdespm_lowpass_example:\n");
    printf("  -h        : print usage/help\n");
    printf("  -n <len>  : filter length,              1 < n        default: 57\n");
    printf("  -f <freq> : filter cutoff frequency,    0 < f < 0.5, default: 0.2\n");
    printf("  -s <atten>: stop-band attenuation [dB], 0 < s,       default: 60\n");
}

int main(int argc, char*argv[]) {
    // options
    unsigned int n  =  57;      // filter cutoff frequency
    float        fc = 0.2f;     // filter cutoff frequency
    float        As = 60.0f;    // stop-band attenuation [dB]

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:f:s:")) != EOF) {
        switch (dopt) {
        case 'h': usage();           return 0;
        case 'n': n  = atoi(optarg); break;
        case 'f': fc = atof(optarg); break;
        case 's': As = atof(optarg); break;
        default: return -1;
        }
    }
    unsigned int i;
    printf("filter design parameters\n");
    printf("  length                : %12u\n",      n);
    printf("  cutoff frequency      : %12.8f Fs\n", fc);
    printf("  stop-band attenuation : %12.3f dB\n", As);

    // design the filter
    float h[n];
    firdespm_lowpass(n,fc,As,0,h);

#if 0
    // print coefficients
    for (i=0; i<n; i++)
        printf("h(%4u) = %16.12f;\n", i+1, h[i]);
#endif

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"h_len=%u;\n", n);
    fprintf(fid,"fc=%12.4e;\n",fc);
    fprintf(fid,"As=%12.4e;\n",As);

    for (i=0; i<n; i++)
        fprintf(fid,"h(%4u) = %20.8e;\n", i+1, h[i]);

    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"H=20*log10(abs(fftshift(fft(h,nfft))));\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure; plot(f,H,'Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"title(['Filter design (firdespm) f_c: %.3f, S_L: %.3f, h: %u']);\n",
            fc, -As, n);
    fprintf(fid,"axis([-0.5 0.5 -As-20 10]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

