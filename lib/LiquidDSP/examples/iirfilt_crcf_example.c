//
// iirfilt_crcf_example.c
//
// Complex infinite impulse response filter example. Demonstrates
// the functionality of iirfilt by designing a low-order
// prototype (e.g. Butterworth) and using it to filter a noisy
// signal.  The filter coefficients are real, but the input and
// output arrays are complex.  The filter order and cutoff
// frequency are specified at the beginning.
//

#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "iirfilt_crcf_example.m"

int main() {
    // options
    unsigned int order =   4;       // filter order
    float        fc    =   0.1f;    // cutoff frequency
    float        f0    =   0.0f;    // center frequency
    float        Ap    =   1.0f;    // pass-band ripple
    float        As    =  40.0f;    // stop-band attenuation
    unsigned int n     = 128;       // number of samples
    liquid_iirdes_filtertype ftype  = LIQUID_IIRDES_ELLIP;
    liquid_iirdes_bandtype   btype  = LIQUID_IIRDES_LOWPASS;
    liquid_iirdes_format     format = LIQUID_IIRDES_SOS;

    // design filter from prototype
    iirfilt_crcf q = iirfilt_crcf_create_prototype(
                        ftype, btype, format, order, fc, f0, Ap, As);
    iirfilt_crcf_print(q);

    unsigned int i;

    // allocate memory for data arrays
    float complex x[n];
    float complex y[n];

    // generate input signal (noisy sine wave with decaying amplitude)
    for (i=0; i<n; i++) {
        x[i] = cexpf((2*M_PI*0.057f*_Complex_I - 0.04f)*i);
        x[i] += 0.1f*(randnf() + _Complex_I*randnf());

        // run filter
        iirfilt_crcf_execute(q, x[i], &y[i]);
    }

    // compute response
    unsigned int nfft=512;
    float complex H[nfft];
    for (i=0; i<nfft; i++) {
        float freq = 0.5f * (float)i / (float)nfft;
        iirfilt_crcf_freqresponse(q, freq, &H[i]);
    }

    // destroy filter object
    iirfilt_crcf_destroy(q);

    // 
    // plot results to output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n");
    fprintf(fid,"order=%u;\n", order);
    fprintf(fid,"n=%u;\n",n);
    fprintf(fid,"nfft=%u;\n",nfft);
    fprintf(fid,"x=zeros(1,n);\n");
    fprintf(fid,"y=zeros(1,n);\n");
    fprintf(fid,"H=zeros(1,nfft);\n");

    for (i=0; i<n; i++) {
        //printf("%4u : %12.8f + j*%12.8f\n", i, crealf(y), cimagf(y));
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }

    for (i=0; i<nfft; i++)
        fprintf(fid,"H(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(H[i]), cimagf(H[i]));

    // plot output
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,real(y),'-','Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"  legend('input','filtered output','location','northeast');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,imag(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(y),'-','Color',[0 0.25 0.5],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"  legend('input','filtered output','location','northeast');\n");
    fprintf(fid,"  grid on;\n");

    // plot frequency response
    fprintf(fid,"f=0.5*[0:(nfft-1)]/nfft;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)),'Color',[0 0.25 0.5],'LineWidth',2);\n");
    fprintf(fid,"  axis([0 0.5 -3 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('Pass band [dB]');\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)),'Color',[0 0.25 0.5],'LineWidth',2);\n");
    fprintf(fid,"  axis([0 0.5 -100 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('Stop band [dB]');\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(f,180/pi*arg(H),'Color',[0 0.25 0.5],'LineWidth',2);\n");
    //fprintf(fid,"  axis([0 0.5 -100 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('Phase [degrees]');\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

