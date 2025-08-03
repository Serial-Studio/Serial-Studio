//
// firdes_kaiser_example.c
//
// This example demonstrates finite impulse response filter design
// using a Kaiser window.
// SEE ALSO: firdespm_example.c
//

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firdes_kaiser_example.m"

// print usage/help message
void usage()
{
    printf("firdes_kaiser_example:\n");
    printf("  u/h   : print usage/help\n");
    printf("  f     : filter cutoff frequency,           0 < f < 0.5, default: 0.2\n");
    printf("  s     : filter stop-band attenuation [dB], 0 < s,       default: 60\n");
    printf("  m     : fractional sample delay,        -0.5 < m < 0.5, default: 0\n");
    printf("  n     : filter length [taps],              n > 1,       default: 55\n");
}

int main(int argc, char*argv[]) {
    // options
    float fc=0.2f;          // filter cutoff frequency
    float As=60.0f;         // stop-band attenuation [dB]
    float mu=0.0f;          // fractional timing offset
    unsigned int h_len=55;  // filter length

    int dopt;
    while ((dopt = getopt(argc,argv,"uhf:n:s:m:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();              return 0;
        case 'f': fc    = atof(optarg); break;
        case 'n': h_len = atoi(optarg); break;
        case 's': As    = atof(optarg); break;
        case 'm': mu    = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if ( fc <= 0.0f ) {
        fprintf(stderr,"error: %s, filter cutoff frequency must be greater than zero\n", argv[0]);
        exit(1);
    } else if ( h_len == 0 ) {
        fprintf(stderr,"error: %s, filter length must be greater than zero\n", argv[0]);
        exit(1);
    }

    printf("filter design parameters\n");
    printf("    cutoff frequency            :   %8.4f\n", fc);
    printf("    stop-band attenuation [dB]  :   %8.4f\n", As);
    printf("    fractional sample offset    :   %8.4f\n", mu);
    printf("    filter length               :   %u\n", h_len);

    // generate the filter
    unsigned int i;
    float h[h_len];
    liquid_firdes_kaiser(h_len,fc,As,mu,h);

    // print coefficients
    for (i=0; i<h_len; i++)
        printf("h(%4u) = %16.12f;\n", i+1, h[i]);

    // output to file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"h_len=%u;\n",h_len);
    fprintf(fid,"fc=%12.4e;\n",fc);
    fprintf(fid,"As=%12.4e;\n",As);

    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%4u) = %12.4e;\n", i+1, h[i]);

    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"H=20*log10(abs(fftshift(fft(h*2*fc,nfft))));\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure; plot(f,H,'Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"title(['Filter design/Kaiser window f_c: %f, S_L: %f, h: %u']);\n",
            fc, -As, h_len);
    fprintf(fid,"axis([-0.5 0.5 -As-40 10]);\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

