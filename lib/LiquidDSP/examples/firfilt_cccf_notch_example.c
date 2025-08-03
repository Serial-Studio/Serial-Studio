//
// firfilt_cccf_notch_example.c
//
// This example demonstrates how to create a notching non-recursive
// (finite impulse response) filter.
//

#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firfilt_cccf_notch_example.m"

int main() {
    // options
    unsigned int num_samples = 600;     // number of samples
    unsigned int m           = 25;      // prototype filter semi-length
    float        As          = 30.0f;   // prototype filter stop-band suppression
    float        f0          = 0.15f;   // notch frequency

    // design filter from prototype
    firfilt_cccf q = firfilt_cccf_create_notch(m,As,f0);
    firfilt_cccf_print(q);

    // allocate memory for data arrays
    float complex x[num_samples];   // original input
    float complex y[num_samples];   // filtered signal

    // generate input signal
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        x[i] = cexpf(_Complex_I*2*M_PI*0.037*i) +
               cexpf(_Complex_I*2*M_PI*f0*   i);
    }

    // run notch filter
    firfilt_cccf_execute_block(q, x, num_samples, y);

    // destroy filter object
    firfilt_cccf_destroy(q);

    // 
    // plot results to output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n");
    fprintf(fid,"num_samples=%u;\n",num_samples);
    fprintf(fid,"x=zeros(1,num_samples);\n");
    fprintf(fid,"y=zeros(1,num_samples);\n");

    // save input, output arrays
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }

    // plot output
    fprintf(fid,"t=0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t,real(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(x),'-','Color',[0 0.2 0.5],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('original input');\n");
    fprintf(fid,"  legend('real','imag','location','northeast');\n");
    fprintf(fid,"  axis([0 num_samples -2.2 2.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(t,real(y),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(y),'-','Color',[0 0.5 0.2],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('notch filter output');\n");
    fprintf(fid,"  legend('real','imag','location','northeast');\n");
    fprintf(fid,"  axis([0 num_samples -2.2 2.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  nfft = 2^nextpow2(num_samples);\n");
    fprintf(fid,"  f    = [0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"  X    = 20*log10(abs(fftshift(fft(x,nfft))));\n");
    fprintf(fid,"  Y    = 20*log10(abs(fftshift(fft(y,nfft))));\n");
    fprintf(fid,"  plot(f,X,'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       f,Y,'-','Color',[0.5 0 0],  'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('PSD (dB)');\n");
    fprintf(fid,"  legend('original','filtered','location','northeast');\n");
    fprintf(fid,"  axis([-0.5 0.5 -20 60]);\n");
    fprintf(fid,"  grid on;\n");

    // close output file
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

