//
// iirfilt_cccf_example.c
//
// Complex infinite impulse response filter example. Demonstrates the
// functionality of iirfilt with complex coefficients by designing a
// filter with specified parameters and then filters noise.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "iirfilt_cccf_example.m"

// print usage/help message
void usage()
{
    printf("iirfilt_cccf_example -- infinite impulse response filter example\n");
    printf("options (default values in []):\n");
    printf("  h     : print help\n");
    printf("  t     : filter type: [butter], cheby1, cheby2, ellip, bessel\n");
    printf("  b     : filter transformation: [LP], HP, BP, BS\n");
    printf("  n     : filter order, n > 0 [5]\n");
    printf("  r     : passband ripple in dB (cheby1, ellip), r > 0 [1.0]\n");
    printf("  s     : stopband attenuation in dB (cheby2, ellip), s > 0 [40.0]\n");
    printf("  f     : passband cut-off, 0 < f < 0.5 [0.2]\n");
    printf("  c     : center frequency (BP, BS cases), 0 < c < 0.5 [0.25]\n");
    printf("  o     : format [sos], tf\n");
    printf("          sos   : second-order sections form\n");
    printf("          tf    : regular transfer function form (potentially\n");
    printf("                  unstable for large orders\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int order=4;   // filter order
    float fc=0.1f;          // cutoff frequency
    float f0=0.0f;          // center frequency
    float Ap=1.0f;          // pass-band ripple
    float As=40.0f;         // stop-band attenuation
    unsigned int n=128;     // number of samples
    liquid_iirdes_filtertype ftype  = LIQUID_IIRDES_ELLIP;
    liquid_iirdes_bandtype   btype  = LIQUID_IIRDES_LOWPASS;
    liquid_iirdes_format     format = LIQUID_IIRDES_SOS;

    int dopt;
    while ((dopt = getopt(argc,argv,"ht:b:n:r:s:f:c:o:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();    return 0;
        case 't':
            if      (strcmp(optarg,"butter")==0) ftype = LIQUID_IIRDES_BUTTER;
            else if (strcmp(optarg,"cheby1")==0) ftype = LIQUID_IIRDES_CHEBY1;
            else if (strcmp(optarg,"cheby2")==0) ftype = LIQUID_IIRDES_CHEBY2;
            else if (strcmp(optarg,"ellip") ==0) ftype = LIQUID_IIRDES_ELLIP;
            else if (strcmp(optarg,"bessel")==0) ftype = LIQUID_IIRDES_BESSEL;
            else {
                fprintf(stderr,"error: iirdes_example, unknown filter type '%s'\n", optarg);
                exit(1);
            }
            break;
        case 'b':
            if      (strcmp(optarg,"LP")==0) btype = LIQUID_IIRDES_LOWPASS;
            else if (strcmp(optarg,"HP")==0) btype = LIQUID_IIRDES_HIGHPASS;
            else if (strcmp(optarg,"BP")==0) btype = LIQUID_IIRDES_BANDPASS;
            else if (strcmp(optarg,"BS")==0) btype = LIQUID_IIRDES_BANDSTOP;
            else {
                fprintf(stderr,"error: iirdes_example, unknown band type '%s'\n", optarg);
                exit(1);
            }
            break;
        case 'n': order = atoi(optarg); break;
        case 'r': Ap = atof(optarg);    break;
        case 's': As = atof(optarg);    break;
        case 'f': fc = atof(optarg);    break;
        case 'c': f0 = atof(optarg);    break;
        case 'o':
            if      (strcmp(optarg,"sos")==0) format = LIQUID_IIRDES_SOS;
            else if (strcmp(optarg,"tf") ==0) format = LIQUID_IIRDES_TF;
            else {
                fprintf(stderr,"error: iirdes_example, unknown output format '%s'\n", optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    // design filter from prototype
    iirfilt_cccf q = iirfilt_cccf_create_prototype(ftype, btype, format, order, fc, f0, Ap, As);
    iirfilt_cccf_print(q);

    unsigned int i;

    // allocate memory for data arrays
    float complex x[n];
    float complex y[n];

    // generate input signal (noisy sine wave with decaying amplitude)
    unsigned int wlen = (3*n)/4;
    for (i=0; i<n; i++) {
        // input signal (windowed noise)
        x[i]  = randnf() + _Complex_I*randnf();
        x[i] *= i < wlen ? liquid_hamming(i,wlen) : 0.0f;

        // run filter
        iirfilt_cccf_execute(q, x[i], &y[i]);
    }

    // compute two-sided frequency response
    unsigned int nfft=512;
    float complex H[nfft];
    for (i=0; i<nfft; i++) {
        float freq = (float)i / (float)nfft - 0.5f;
        iirfilt_cccf_freqresponse(q, freq, &H[i]);
    }

    // destroy filter object
    iirfilt_cccf_destroy(q);

    // 
    // plot results to output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n");
    fprintf(fid,"order=%u;\n", order);

    // save input, output arrays
    fprintf(fid,"n=%u;\n",n);
    fprintf(fid,"x=zeros(1,n);\n");
    fprintf(fid,"y=zeros(1,n);\n");
    for (i=0; i<n; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }

    // save frequency response
    fprintf(fid,"nfft=%u;\n",nfft);
    fprintf(fid,"H=zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"H(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(H[i]), cimagf(H[i]));

    // plot time-domain output
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,real(y),'-','Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"  legend('input','filtered output',1);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,imag(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(y),'-','Color',[0 0.25 0.5],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"  legend('input','filtered output',1);\n");
    fprintf(fid,"  grid on;\n");

    // plot spectral output
    fprintf(fid,"X = 20*log10(abs(fftshift(fft(x))));\n");
    fprintf(fid,"Y = 20*log10(abs(fftshift(fft(y))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot([0:(n-1)]/n-0.5, X, 'Color', [1 1 1]*0.5,\n");
    fprintf(fid,"     [0:(n-1)]/n-0.5, Y, 'Color', [0 0.25 0.50]);\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"legend('input','filtered output',1);\n");
    fprintf(fid,"grid on;\n");

    // plot ideal frequency response
    fprintf(fid,"f=[0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)));\n");
    fprintf(fid,"  axis([-0.5 0.5 -3 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  legend('Pass band (dB)',0);\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)));\n");
    fprintf(fid,"  axis([-0.5 0.5 -100 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  legend('Stop band (dB)',0);\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(f,180/pi*arg(H));\n");
    fprintf(fid,"  axis([-0.5 0.5 -180 180]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  legend('Phase (degrees)',0);\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

