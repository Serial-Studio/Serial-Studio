// Demonstrate fdelay object to add arbitrary fractional delays.
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "liquid.h"
#define OUTPUT_FILENAME "fdelay_rrrf_example.m"

int main() {
    // options
    unsigned int nmax       = 200;  // maximum delay
    unsigned int m          =  12;  // filter semi-length
    unsigned int npfb       =  10;  // fractional delay resolution
    unsigned int num_samples= 240;  // number of samples to run
    float        delay      =27.8;  // requested delay

    // create delay object and set delay
    fdelay_rrrf q = fdelay_rrrf_create(nmax, m, npfb);
    fdelay_rrrf_set_delay(q, delay);
    fdelay_rrrf_print(q);

    // generate impulse and propagate through object
    float x[num_samples];
    float y[num_samples];
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        // generate input
        x[i] = i==0 ? 1.0f : 0.0f;

        // run filter
        fdelay_rrrf_push(q, x[i]);
        fdelay_rrrf_execute(q, &y[i]);
    }

    // destroy filter object
    fdelay_rrrf_destroy(q);

    // estimate delay; assumes input is impulse and uses phase at
    // single point of frequency estimate evaluation
    float fc = 0.1f / (float)num_samples; // sufficiently small
    float complex v = 0.0f;
    for (i=0; i<num_samples; i++)
        v += y[i] * cexpf(_Complex_I*2*M_PI*fc*i);
    float delay_est = cargf(v) / (2*M_PI*fc) - (float)m;
    printf("fdelay = %g (%g)\n", delay_est, delay);

    // plot results to output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"delay=%f; m=%u; n=%u;\n",delay,m,num_samples);
    fprintf(fid,"x=zeros(1,n);\n");
    fprintf(fid,"y=zeros(1,n);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.4e;\n", i+1, x[i]);
        fprintf(fid,"y(%4u) = %12.4e;\n", i+1, y[i]);
    }
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,x,'-x','Color',[0 0.3 0.5],'LineWidth',1);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('input');\n");
    fprintf(fid,"  xlim([-10 10]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t-m,y,'-x','Color',[0 0.3 0.5],'LineWidth',1,...\n");
    fprintf(fid,"       [delay delay],[-0.1 1.1],'-r');\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('output');\n");
    fprintf(fid,"  xlim([-3 3]+delay); ylim([-0.2 1.2]);\n");
    fprintf(fid,"  grid on;\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    return 0;
}

