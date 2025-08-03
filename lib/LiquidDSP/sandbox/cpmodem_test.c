//
// cpmodem_test.c
//
// This example demonstrates how the use the nco/pll object
// (numerically-controlled oscillator with phase-locked loop) interface for
// tracking to a complex sinusoid.  The loop bandwidth, phase offset, and
// other parameter can be specified via the command-line interface.
//
// SEE ALSO: nco_example.c
//           nco_pll_modem_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "cpmodem_test.m"

// print usage/help message
void usage()
{
    printf("cpmodem_test [options]\n");
    printf("  h     : print usage\n");
    printf("  m     : bits per symbol, default: 2\n");
    printf("  n     : number of symbols, default: 20\n");
    printf("  p     : phase offset (radians), default: pi/4\n");
    printf("  f     : frequency offset (radians), default: 0.3\n");
}

int main(int argc, char*argv[]) {
    srand( time(NULL) );
    // parameters
    float phase_offset = M_PI / 4.0f;
    float frequency_offset = 0.1f;
    unsigned int m = 2;             // bits per symbol
    unsigned int num_symbols=20;    // number of iterations

    int dopt;
    while ((dopt = getopt(argc,argv,"uhm:n:p:f:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();    return 0;
        case 'm':   m = atoi(optarg);               break;
        case 'n':   num_symbols = atoi(optarg);     break;
        case 'p':   phase_offset = atof(optarg);    break;
        case 'f':   frequency_offset= atof(optarg); break;
        default:
            exit(1);
        }
    }

    // derived values
    unsigned int M = 1<<m;  // constellation size
    unsigned int k = 4*m;   // samples per symbol
    unsigned int num_samples = num_symbols*k;
    float nstd = 0.1f;

    // objects
    nco_crcf nco_tx = nco_crcf_create(LIQUID_VCO);

    // generate input
    float dphi[num_samples];        // transmitted instantaneous frequency
    float complex x[num_samples];   // transmitted signal
    float complex y[num_samples];   // received signal
    float dphi_hat[num_samples];    // received instantaneous frequency

    unsigned int n=0;               // sample counter
    unsigned int s;                 // symbol counter
    unsigned int i;                 // 
    for (s=0; s<num_symbols; s++) {
        unsigned int symbol = rand() % M;

        // compute frequency
        // TODO: determine appropriate scaling
        float f = (2*(float)symbol - (float)M + 1.0f) * M_PI / (float)M;

        // set frequency
        nco_crcf_set_frequency(nco_tx, f);

        // synthesize signal
        for (i=0; i<k; i++) {
            // generate complex sinusoid
            nco_crcf_cexpf(nco_tx, &x[n]);
            nco_crcf_step(nco_tx);
            
            // save frequency
            dphi[n] = f;
            n++;
        }
    }

    // add channel impairments
    for (i=0; i<num_samples; i++) {
        y[i]  = x[i] * cexpf(_Complex_I*(frequency_offset*i + phase_offset));
        y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }

    // run loop
    float complex yn = 0.0f;
    for (i=0; i<num_samples; i++) {
        float complex r = y[i] * conjf(yn);

        dphi_hat[i] = cargf(r);

        yn = y[i];
    }
    nco_crcf_destroy(nco_tx);

    // write output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"m           = %u;\n", m);
    fprintf(fid,"M           = %u;\n", M);
    fprintf(fid,"k           = %u;\n", k);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    fprintf(fid,"dphi     = zeros(1,num_samples);\n");
    fprintf(fid,"dphi_hat = zeros(1,num_samples);\n");
    for (i=0; i<n; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"dphi(%4u)     = %12.4e;\n", i+1, dphi[i]);
        fprintf(fid,"dphi_hat(%4u) = %12.4e;\n", i+1, dphi_hat[i]);
    }
    fprintf(fid,"t=0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(x),t,imag(x));\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('x(t)');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,real(y),t,imag(y));\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('y(t)');\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,dphi/(2*pi), t, dphi_hat/(2*pi));\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('instantaneous frequency');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n",OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
