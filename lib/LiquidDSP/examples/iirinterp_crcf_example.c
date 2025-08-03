// This example demonstrates the iirinterp object (IIR interpolator)
// interface.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "iirinterp_crcf_example.m"

// print usage/help message
void usage()
{
    printf("iirinterp_crcf_example:\n");
    printf("  h     : print help\n");
    printf("  k     : samples/symbol (interp factor), k > 1, default: 4\n");
    printf("  n     : number of input samples, default: 64\n");
}

int main(int argc, char*argv[]) {
    // options
    unsigned int k = 4;             // interpolation factor
    unsigned int num_samples = 64;  // number of input samples

    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:n:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'k': k           = atoi(optarg);   break;
        case 'n': num_samples = atoi(optarg);   break;
        default:
            exit(1);
        }
    }

    // validate options
    if (k < 2) {
        fprintf(stderr,"error: %s, interp factor must be greater than 1\n", argv[0]);
        exit(1);
    } else if (num_samples < 1) {
        fprintf(stderr,"error: %s, must have at least one data symbol\n", argv[0]);
        usage();
        return 1;
    }

    // create interpolator from prototype
    unsigned int order = 8;
    iirinterp_crcf q = iirinterp_crcf_create_default(k,order);

    // derived values
    float delay = iirinterp_crcf_groupdelay(q,0.0f);

    // generate input signal and interpolate
    float complex x[  num_samples]; // input samples
    float complex y[k*num_samples]; // output samples
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        // input signal (sinusoidal chirp)
        x[i] = cexpf(_Complex_I*(-0.17f*i + 0.9*i*i/(float)num_samples));

        // apply window
        x[i] *= (i < num_samples-5) ? liquid_hamming(i,num_samples) : 0.0f;

        // push through interpolator
        iirinterp_crcf_execute(q, x[i], &y[k*i]);
    }

    // destroy interpolator object
    iirinterp_crcf_destroy(q);

    // export output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"delay = %f;\n", delay);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"x = zeros(1,  num_samples);\n");
    fprintf(fid,"y = zeros(1,k*num_samples);\n");

    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));

    for (i=0; i<k*num_samples; i++)
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    fprintf(fid,"\n\n");
    fprintf(fid,"tx = [0:(  num_samples-1)];\n");
    fprintf(fid,"ty = [0:(k*num_samples-1)]/k - delay;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"    plot(tx,real(x),'-s','MarkerSize',3,ty,real(y),'-s','MarkerSize',1);\n");
    fprintf(fid,"    legend('input','interp','location','northeast');\n");
    fprintf(fid,"    axis([0 num_samples -1.2 1.2]);\n");
    fprintf(fid,"    xlabel('time');\n");
    fprintf(fid,"    ylabel('real');\n");
    fprintf(fid,"    grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"    plot(tx,imag(x),'-s','MarkerSize',3,ty,imag(y),'-s','MarkerSize',1);\n");
    fprintf(fid,"    legend('input','interp','location','northeast');\n");
    fprintf(fid,"    axis([0 num_samples -1.2 1.2]);\n");
    fprintf(fid,"    xlabel('time');\n");
    fprintf(fid,"    ylabel('imag');\n");
    fprintf(fid,"    grid on;\n");

    // power spectral density
    fprintf(fid,"nfft = 1024;\n");
    fprintf(fid,"fx   = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"fy   = k*fx;\n");
    fprintf(fid,"X    = 20*log10(abs(fftshift(fft(x  ,nfft))));\n");
    fprintf(fid,"Y    = 20*log10(abs(fftshift(fft(y/k,nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(fx,X,'LineWidth',2, fy,Y,'LineWidth',1);\n");
    fprintf(fid,"legend('input','interp','location','northeast');\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n",OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
