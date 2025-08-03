//
// Rice-K fading generator example.
//
// This example generates correlated circular complex Gauss random variables
// using an approximation to the ideal Doppler filter. The resulting Gauss
// random variables are converted to Rice-K random variables using a simple
// transformation. The resulting output file plots the filter's power
// spectral density, the fading power envelope as a function of time, and the
// distribution of Rice-K random variables alongside the theoretical PDF to
// demonstrate that the technique is valid.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "ricek_channel_example.m"

// print usage/help message
void usage()
{
    printf("%s [options]\n", __FILE__);
    printf("  h     : print help\n");
    printf("  n     : number of samples\n");
    printf("  f     : maximum doppler frequency (0,0.5)\n");
    printf("  K     : Rice K factor, linear, greater than zero\n");
    printf("  O     : mean power, linear (Rice-K distribution 'omega')\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int h_len = 51;        // doppler filter length
    float fd           = 0.1f;      // maximum doppler frequency
    float K            = 2.0f;      // Rice fading factor
    float omega        = 1.0f;      // mean power
    float theta        = 0.0f;      // angle of arrival
    unsigned int num_samples=1200;  // number of samples

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:f:K:O:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                    return 0;
        case 'n':   num_samples = atoi(optarg); break;
        case 'f':   fd          = atof(optarg); break;
        case 'K':   K           = atof(optarg); break;
        case 'O':   omega       = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (K < 0.0f) {
        fprintf(stderr,"error: %s, fading factor K must be greater than zero\n", argv[0]);
        exit(1);
    } else if (omega < 0.0f) {
        fprintf(stderr,"error: %s, signal power Omega must be greater than zero\n", argv[0]);
        exit(1);
    } else if (fd <= 0.0f || fd >= 0.5f) {
        fprintf(stderr,"error: %s, Doppler frequency must be in (0,0.5)\n", argv[0]);
        exit(1);
    } else if (h_len < 4) {
        fprintf(stderr,"error: %s, Doppler filter length too small\n", argv[0]);
        exit(1);
    } else if (num_samples == 0) {
        fprintf(stderr,"error: %s, number of samples must be greater than zero\n", argv[0]);
        exit(1);
    }

    unsigned int i;

    // allocate array for output samples
    float complex * y = (float complex*) malloc(num_samples*sizeof(float complex));

    // generate Doppler filter coefficients
    float h[h_len];
    liquid_firdes_doppler(h_len, fd, K, theta, h);

    // normalize filter coefficients such that output Gauss random
    // variables have unity variance
    float std = 0.0f;
    for (i=0; i<h_len; i++)
        std += h[i]*h[i];
    std = sqrtf(std);
    for (i=0; i<h_len; i++)
        h[i] /= std;

    // create Doppler filter from coefficients
    firfilt_crcf fdoppler = firfilt_crcf_create(h,h_len);

    // generate complex circular Gauss random variables
    float complex v;    // circular Gauss random variable (uncorrelated)
    float complex x;    // circular Gauss random variable (correlated w/ Doppler filter)
    float s   = sqrtf((omega*K)/(K+1.0));
    float sig = sqrtf(0.5f*omega/(K+1.0));
    for (i=0; i<num_samples; i++) {
        // generate complex Gauss random variable
        crandnf(&v);

        // push through Doppler filter
        firfilt_crcf_push(fdoppler, v);
        firfilt_crcf_execute(fdoppler, &x);

        // convert result to random variable with Rice-K distribution
        y[i] = _Complex_I*( crealf(x)*sig + s ) +
                          ( cimagf(x)*sig     );
    }

    // destroy filter object
    firfilt_crcf_destroy(fdoppler);

    // export results to file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n");
    fprintf(fid,"h_len       = %u;\n", h_len);
    fprintf(fid,"num_samples = %u;\n", num_samples);

    // save filter coefficients
    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%6u) = %12.4e;\n", i+1, h[i]);

    // save samples
    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%6u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    // plot power spectral density of filter
    fprintf(fid,"nfft = min(1024, 2^(ceil(log2(h_len))+4));\n");
    fprintf(fid,"f    = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H    = 20*log10(abs(fftshift(fft(h,nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,H);\n");
    fprintf(fid,"axis([-0.5 0.5 -80 20]);\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Filter Power Spectral Density [dB]');\n");
    fprintf(fid,"grid on;\n");

    // plot fading profile
    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(num_samples-1);\n");
    fprintf(fid,"plot(t,20*log10(abs(y)));\n");
    fprintf(fid,"xlabel('Normalized Time [t F_s]');\n");
    fprintf(fid,"ylabel('Fading Power Envelope [dB]');\n");
    fprintf(fid,"axis([0 num_samples -40 10]);\n");
    fprintf(fid,"grid on;\n");

    // plot distribution
    fprintf(fid,"[nn xx]   = hist(abs(y),15);\n");
    fprintf(fid,"bin_width = xx(2) - xx(1);\n");
    fprintf(fid,"ymax = max(abs(y));\n");
    fprintf(fid,"s    = %12.4e;\n", s);
    fprintf(fid,"sig  = %12.4e;\n", sig);
    fprintf(fid,"yp   = 1.1*ymax*[1:500]/500;\n");
    fprintf(fid,"pdf  = (yp/sig^2) .* exp(-(yp.^2+s^2)/(2*sig^2)) .* besseli(0,yp*s/sig^2);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(yp,pdf,'-', xx,nn/(num_samples*bin_width),'x');\n");
    fprintf(fid,"xlabel('Fading Magnitude');\n");
    fprintf(fid,"ylabel('Probability Density');\n");
    fprintf(fid,"legend('theory','data','location','northeast');\n");

    // close output file
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    // clean up allocated arrays
    free(y);

    printf("done.\n");
    return 0;
}

