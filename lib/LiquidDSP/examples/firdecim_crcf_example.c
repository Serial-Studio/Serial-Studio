//
// firdecim_crcf_example.c
//
// This example demonstrates the interface to the firdecim (finite
// impulse response decimator) family of objects.
//
// SEE ALSO: firinterp_crcf_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firdecim_crcf_example.m"

// print usage/help message
void usage()
{
    printf("firdecim_crcf_example:\n");
    printf("  -h         : print usage/help\n");
    printf("  -M <decim> : decimation factor, M > 1           default: 2\n");
    printf("  -m <delay> : filter delay (symbols), m > 0,     default: 2\n");
    printf("  -s <atten> : filter stop-band attenuation [dB], default: 60\n");
    printf("  -n <num>   : number of samples (after decim),   default: 8\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int M           = 6;       // decimation factor
    unsigned int m           = 8;       // filter delay
    float        As          = 60.0f;   // filter stop-band attenuation
    unsigned int num_samples = 120;     // number of samples (after decim)

    int dopt;
    while ((dopt = getopt(argc,argv,"hM:m:s:n:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                    return 0;
        case 'M': M           = atoi(optarg); break;
        case 'm': m           = atoi(optarg); break;
        case 's': As          = atof(optarg); break;
        case 'n': num_samples = atoi(optarg); break;
        default:
            usage();
            return 1;
        }
    }

    // validate options
    if (M < 2) {
        fprintf(stderr,"error: %s, decim factor must be greater than 1\n", argv[0]);
        return 1;
    } else if (m < 1) {
        fprintf(stderr,"error: %s, filter delay must be greater than 0\n", argv[0]);
        return 1;
    } else if (As <= 0.0) {
        fprintf(stderr,"error: %s, stop-band attenuation must be greater than zero\n", argv[0]);
        return 1;
    } else if (num_samples < 1) {
        fprintf(stderr,"error: %s, must have at least one sample\n", argv[0]);
        return 1;
    }

    // data arrays
    float complex x[M*num_samples]; // number of samples before decimation
    float complex y[  num_samples]; // number of samples after decimation

    // initialize input array
    unsigned int i;
    unsigned int w_len = (unsigned int)(0.9*M*num_samples);
    float f0 = 0.017f;
    float f1 = 0.021f;
    for (i=0; i<M*num_samples; i++) {
        x[i]  = 0.6f*cexpf(_Complex_I*2*M_PI*f0*i);
        x[i] += 0.4f*cexpf(_Complex_I*2*M_PI*f1*i);
        x[i] *= (i < w_len) ? liquid_hamming(i,w_len) : 0;
    }

    // create decimator object and set scale
    firdecim_crcf decim = firdecim_crcf_create_kaiser(M, m, As);
    firdecim_crcf_set_scale(decim, 1.0f/(float)M);

    // execute decimator
    firdecim_crcf_execute_block(decim, x, num_samples, y);

    // destroy decimator object
    firdecim_crcf_destroy(decim);

    // 
    // export results to file
    //
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"M  = %u;\n", M);
    fprintf(fid,"m  = %u;\n", m);
    fprintf(fid,"num_samples=%u;\n", num_samples);

    // inputs
    for (i=0; i<M*num_samples; i++)
        fprintf(fid,"x(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));

    // outputs
    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    // plot results
    fprintf(fid,"figure('position',[100 100 600 800]);\n");
    fprintf(fid,"tx = [0:(M*num_samples-1)];\n");
    fprintf(fid,"ty = [0:(  num_samples-1)]*M - M*m;\n");
    fprintf(fid,"nfft=3*2^nextpow2(M*num_samples);\n");
    fprintf(fid,"fx = [0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"fy = fx/M;\n");
    fprintf(fid,"X=20*log10(abs(fftshift(fft(  x,nfft))));\n");
    fprintf(fid,"Y=20*log10(abs(fftshift(fft(M*y,nfft))));\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(tx,real(x),'-',ty,real(y),'s');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-M*m M*num_samples -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('Input sample index');\n");
    fprintf(fid,"  ylabel('Real');\n");
    fprintf(fid,"  legend('original','decimated','location','northeast');");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(tx,imag(x),'-',ty,imag(y),'s');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-M*m M*num_samples -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('Input sample index');\n");
    fprintf(fid,"  ylabel('Imag');\n");
    fprintf(fid,"  legend('original','decimated','location','northeast');");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(fx,X,'Color',[0.5 0.5 0.5],fy,Y,'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-0.5 0.5 -40 60]);\n");
    fprintf(fid,"  xlabel('normalized frequency');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fprintf(fid,"  legend('original/real','transformed/decimated','location','northeast');");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
