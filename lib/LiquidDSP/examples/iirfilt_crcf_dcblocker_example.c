//
// iirfilt_crcf_dcblocker_example.c
//
// This example demonstrates how to create a DC-blocking recursive
// (infinite impulse response) filter.
//

#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "iirfilt_crcf_dcblocker_example.m"

int main() {
    // options
    unsigned int num_samples = 1200;    // number of samples
    float        alpha       = 0.10f;   // filter cut-off

    // design filter from prototype
    iirfilt_crcf q = iirfilt_crcf_create_dc_blocker(alpha);
    iirfilt_crcf_print(q);

    // allocate memory for data arrays
    float complex x[num_samples];   // original input
    float complex y[num_samples];   // input with DC offset
    float complex z[num_samples];   // DC-blocked result

    // generate signals
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        // original input signal
        x[i] = cexpf( (0.070f*i + 1e-4f*i*i)*_Complex_I );

        // add DC offset
        y[i] = x[i] + 2.0f*cexpf( 0.007f*_Complex_I*i );

        // run filter to try to remove DC offset
        iirfilt_crcf_execute(q, y[i], &z[i]);
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
    fprintf(fid,"num_samples=%u;\n",num_samples);
    fprintf(fid,"x=zeros(1,num_samples);\n");
    fprintf(fid,"y=zeros(1,num_samples);\n");

    // save input, output arrays
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(z[i]), cimagf(z[i]));
    }

    // plot output
    fprintf(fid,"t=0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t,real(x),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(x),'-','Color',[0 0.2 0.5],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('original input');\n");
    fprintf(fid,"  legend('real','imag','location','southwest');\n");
    fprintf(fid,"  axis([0 num_samples -3 3]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(t,real(y),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(y),'-','Color',[0 0.5 0.2],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('input with DC offset');\n");
    fprintf(fid,"  legend('real','imag','location','southwest');\n");
    fprintf(fid,"  axis([0 num_samples -3 3]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(t,real(z),'-','Color',[1 1 1]*0.5,'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(z),'-','Color',[0 0.2 0.5],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('DC-blocked output');\n");
    fprintf(fid,"  legend('real','imag','location','southwest');\n");
    fprintf(fid,"  axis([0 num_samples -3 3]);\n");
    fprintf(fid,"  grid on;\n");

    // close output file
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

