//
// firfilt_crcf_example.c
//
// Complex finite impulse response filter example. Demonstrates the 
// functionality of firfilt by designing a low-order prototype and using it 
// to filter a noisy signal.  The filter coefficients are real, but the 
// input and output arrays are complex. The filter order and cutoff 
// frequency are specified at the beginning.
//

#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firfilt_crcf_example.m"

int main() {
    // options
    unsigned int h_len=65;  // filter length
    float fc=0.1f;          // cutoff frequency
    float As=60.0f;         // stop-band attenuation
    unsigned int n=240;     // number of samples

    // design filter from prototype and scale to bandwidth
    firfilt_crcf q = firfilt_crcf_create_kaiser(h_len, fc, As, 0.0f);
    firfilt_crcf_set_scale(q, 2.0f*fc);
    firfilt_crcf_print(q);

    unsigned int i;

    // allocate memory for data arrays
    float complex x[n];
    float complex y[n];

    // generate input signal (sine wave with decaying amplitude)
    unsigned int wlen = (unsigned int)roundf(0.75*n);
    for (i=0; i<n; i++) {
        // generate input signal
        x[i] = 0.7f*cexpf(2*M_PI*0.057f*_Complex_I*i) +
               0.3f*cexpf(2*M_PI*0.357f*_Complex_I*i);

        // apply window
        x[i] *= i < wlen ? liquid_hamming(i,wlen) : 0;

        // run filter
        firfilt_crcf_push(q, x[i]);
        firfilt_crcf_execute(q, &y[i]);
    }

    // compute response
    unsigned int nfft = 1024;
    float complex H[nfft];
    for (i=0; i<nfft; i++) {
        float freq = ((float)i - 0.5f*(float)nfft) / (float)nfft;
        firfilt_crcf_freqresponse(q, freq, &H[i]);
    }

    // destroy filter object
    firfilt_crcf_destroy(q);

    // 
    // plot results to output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n");
    fprintf(fid,"h_len=%u;\n", h_len);
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
    fprintf(fid,"tx=0:(n-1);\n");
    fprintf(fid,"ty=tx - (h_len-1)/2;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(tx,real(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       ty,real(y),'-','Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"  legend('input','filtered output','location','northeast');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(tx,imag(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       ty,imag(y),'-','Color',[0 0.25 0.5],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"  legend('input','filtered output','location','northeast');\n");
    fprintf(fid,"  grid on;\n");

    // plot frequency response
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"g = 1/sum(abs(x));\n");
    fprintf(fid,"X = 20*log10(abs(fftshift(fft(x*g,nfft))));\n");
    fprintf(fid,"Y = 20*log10(abs(fftshift(fft(y*g,nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,X,'Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"     f,Y,'Color',[0 0.5 0.25],'LineWidth',2,...\n");
    fprintf(fid,"     f,20*log10(abs(H)),'Color',[0 0.25 0.5],'LineWidth',1);\n");
    fprintf(fid,"axis([-0.5 0.5 -120 20]);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"legend('input','output','filter','location','northeast');\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

