//
// agc_crcf_example.c
//
// Automatic gain control example demonstrating its transient
// response.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "agc_crcf_example.m"

// print usage/help message
void usage()
{
    printf("agc_example [options]\n");
    printf("  h     : print usage\n");
    printf("  n     : number of samples, n >=100, default: 2000\n");
    printf("  b     : AGC bandwidth,     b >=  0, default: 0.01\n");
}


int main(int argc, char*argv[])
{
    // options
    float        bt          = 0.01f;   // agc loop bandwidth
    float        gamma       = 0.001f;  // initial signal level
    unsigned int num_samples = 2000;    // number of samples

    int dopt;
    while((dopt = getopt(argc,argv,"hn:N:s:b:")) != EOF){
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'n': num_samples = atoi(optarg);   break;
        case 'b': bt          = atof(optarg);   break;
        default:
            exit(1);
        }
    }

    // validate input
    if (bt < 0.0f) {
        fprintf(stderr,"error: %s, bandwidth must be positive\n", argv[0]);
        exit(1);
    } else if (num_samples == 0) {
        fprintf(stderr,"error: %s, number of samples must be greater than zero\n", argv[0]);
        exit(1);
    }
    
    unsigned int i;

    // create objects
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, bt);
    //agc_crcf_set_scale(q, 0.5f);

    float complex x[num_samples];   // input
    float complex y[num_samples];   // output
    float rssi[num_samples];        // received signal strength

    // print info
    agc_crcf_print(q);

    // generate signal
    for (i=0; i<num_samples; i++)
        x[i] = gamma * cexpf(_Complex_I*2*M_PI*0.0193f*i);

    // run agc
    for (i=0; i<num_samples; i++) {
        // apply gain
        agc_crcf_execute(q, x[i], &y[i]);

        // retrieve signal level [dB]
        rssi[i] = agc_crcf_get_rssi(q);
    }

    // destroy AGC object
    agc_crcf_destroy(q);

    // 
    // export results
    //
    FILE* fid = fopen(OUTPUT_FILENAME,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, could not open '%s' for writing\n", argv[0], OUTPUT_FILENAME);
        exit(1);
    }
    fprintf(fid,"%% %s: auto-generated file\n\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"n = %u;\n", num_samples);

    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"rssi(%4u)  = %12.4e;\n", i+1, rssi[i]);
    }

    // plot results
    fprintf(fid,"\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t, real(x), '-', 'Color',[0 0.2 0.5],...\n");
    fprintf(fid,"       t, imag(x), '-', 'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('input');\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(t, real(y), '-', 'Color',[0 0.2 0.5],...\n");
    fprintf(fid,"       t, imag(y), '-', 'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('output');\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(t,rssi,'-','LineWidth',1.2,'Color',[0.5 0 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('rssi [dB]');\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

