// This example demonstrates how to create an integrating recursive
// (infinite impulse response) filter.
#include <stdio.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "iirfilt_rrrf_integrator_example.m"

int main() {
    // options
    unsigned int num_samples = 1200;    // number of samples

    // allocate memory for data arrays
    float buf_0[num_samples];   // filter input
    float buf_1[num_samples];   // filter output

    // generate input signal
    unsigned int i;
    for (i=0; i<num_samples; i++)
        buf_0[i] = (i > 200 && i < 800 ? 1 : 0) + 0.1*randnf();

    // design filter from prototype
    iirfilt_rrrf q = iirfilt_rrrf_create_integrator();
    iirfilt_rrrf_print(q);

    // run filter
    iirfilt_rrrf_execute_block(q, buf_0, num_samples, buf_1);

    // destroy filter object
    iirfilt_rrrf_destroy(q);

    // plot results to output file
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
        fprintf(fid,"x(%4u) = %12.4e;\n", i+1, buf_0[i]);
        fprintf(fid,"y(%4u) = %12.4e;\n", i+1, buf_1[i]);
    }

    // plot output
    fprintf(fid,"t=0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,x,'-','Color',[0 0.2 0.5],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('original input');\n");
    fprintf(fid,"  axis([0 num_samples -1 2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,y,'-','Color',[0 0.5 0.2],'LineWidth',2);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('input with DC offset');\n");
    fprintf(fid,"  axis([0 num_samples -3 650]);\n");
    fprintf(fid,"  grid on;\n");

    // close output file
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

