//
// firdespm_callback_example.c
//
// This example demonstrates finite impulse response filter design
// using the Parks-McClellan algorithm with callback function for
// arbitrary response and weighting function.
//
// SEE ALSO: firdes_kaiser_example.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firdespm_callback_example.m"

// user-defined callback function defining response and weights
int callback(double   _frequency,
             void   * _userdata,
             double * _desired,
             double * _weight)
{
    // de-reference pointer as floating-point value
    unsigned int n  = *((unsigned int*)_userdata);
    double       v  = sincf(n*_frequency);
    double       fc = 1.0f / (float)n;

    // inverse sinc
    if (_frequency < fc) {
        *_desired = 1.0f / v;   // inverse of sinc
        *_weight  = 4.0f;
    } else {
        *_desired = 0.0f;       // stop-band
        *_weight  = 10*fabs(v) * exp(4.0*_frequency);
    }
    return 0;
}

int main(int argc, char*argv[])
{
    // filter design parameters
    unsigned int n     =  8;    // sinc filter length
    unsigned int h_len = 81;    // inverse sinc filter length
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    unsigned int num_bands = 2;
    float        bands[4]  = {0.00f, 0.75f/(float)n, // pass-band
                              1.05f/(float)n, 0.5f}; // stop-band

    // design filter
    float h[h_len];
    firdespm q = firdespm_create_callback(h_len,num_bands,bands,btype,callback,&n);
    firdespm_execute(q,h);
    firdespm_destroy(q);

    // print coefficients
    unsigned int i;
    for (i=0; i<h_len; i++)
        printf("h(%4u) = %16.12f;\n", i+1, h[i]);

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"n=%u;\n", n);
    fprintf(fid,"h_len=%u;\n", h_len);

    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%4u) = %20.8e;\n", i+1, h[i]);

    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"H0=20*log10(abs(fftshift(fft(ones(1,n)/n,nfft))));\n");
    fprintf(fid,"H1=20*log10(abs(fftshift(fft(h,          nfft))));\n");
    fprintf(fid,"Hc=H0+H1;\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(f,H0,'Color',[0.5 0.5 0.5],'LineWidth',1);\n");
    fprintf(fid,"plot(f,H1,'Color',[0.0 0.2 0.5],'LineWidth',1);\n");
    fprintf(fid,"plot(f,Hc,'Color',[0.0 0.5 0.2],'LineWidth',2);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"legend('sinc','inverse sinc','composite');\n");
    fprintf(fid,"title('Filter design (firdespm), inverse sinc');\n");
    fprintf(fid,"axis([-0.5 0.5 -80 20]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

