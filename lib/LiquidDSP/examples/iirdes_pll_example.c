//
// iirdes_pll_example.c
//
// This example demonstrates 2nd-order IIR phase-locked loop filter
// design with a practical simulation.
//
// SEE ALSO: nco_pll_example.c
//           nco_pll_modem_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "iirdes_pll_example.m"

// print usage/help message
void usage()
{
    printf("iirdes_pll_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  b     : pll bandwidth, default: 0.01\n");
    printf("  z     : zeta (damping factor), default 0.70711\n");
    printf("  K     : loop filter gain, default: 1000\n");
    printf("  n     : number of samples, default: 512\n");
    printf("  p     : phase offset (radians), default: pi/4\n");
    printf("  f     : frequency offset (radians), default: 0.3\n");
}

int main(int argc, char*argv[]) {
    srand( time(NULL) );

    // options
    float phase_offset = M_PI / 4.0f;   // phase offset
    float frequency_offset = 0.3f;      // frequency offset
    float pll_bandwidth = 0.01f;        // PLL bandwidth
    float zeta = 1/sqrtf(2.0f);         // PLL damping factor
    float K = 1000.0f;                  // PLL loop gain
    unsigned int n=512;                 // number of iterations

    int dopt;
    while ((dopt = getopt(argc,argv,"uhb:z:K:n:p:f:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();    return 0;
        case 'b':   pll_bandwidth = atof(optarg);   break;
        case 'z':   zeta = atof(optarg);            break;
        case 'K':   K = atof(optarg);               break;
        case 'n':   n = atoi(optarg);               break;
        case 'p':   phase_offset = atof(optarg);    break;
        case 'f':   frequency_offset= atof(optarg); break;
        default:
            exit(1);
        }
    }
    unsigned int d=n/32;      // print every "d" lines

    // validate input
    if (pll_bandwidth <= 0.0f) {
        fprintf(stderr,"error: bandwidth must be greater than 0\n");
        exit(1);
    } else if (zeta <= 0.0f) {
        fprintf(stderr,"error: damping factor must be greater than 0\n");
        exit(1);
    } else if (K <= 0.0f) {
        fprintf(stderr,"error: loop gain must be greater than 0\n");
        exit(1);
    }

    // data arrays
    float complex x[n];         // input complex sinusoid
    float complex y[n];         // output complex sinusoid
    float phase_error[n];       // output phase error

    // generate PLL filter
    float b[3];
    float a[3];
    iirdes_pll_active_lag(pll_bandwidth, zeta, K, b, a);
    iirfilt_rrrf pll = iirfilt_rrrf_create(b,3,a,3);
    iirfilt_rrrf_print(pll);

    unsigned int i;
    float phi;
    for (i=0; i<n; i++) {
        phi = phase_offset + i*frequency_offset;
        x[i] = cexpf(_Complex_I*phi);
    }

    // run loop
    float theta = 0.0f;
    y[0] = 1.0f;
    for (i=0; i<n; i++) {

        // generate complex sinusoid
        y[i] = cexpf(_Complex_I*theta);

        // compute phase error
        phase_error[i] = cargf(x[i]*conjf(y[i]));

        // update pll
        iirfilt_rrrf_execute(pll, phase_error[i], &theta);

        // print phase error
        if ((i)%d == 0 || i==n-1 || i==0)
            printf("%4u : phase error = %12.8f\n", i, phase_error[i]);
    }

    // destroy filter object
    iirfilt_rrrf_destroy(pll);

    // write output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"n = %u;\n", n);
    fprintf(fid,"x = zeros(1,n);\n");
    fprintf(fid,"y = zeros(1,n);\n");
    for (i=0; i<n; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"e(%4u) = %12.4e;\n", i+1, phase_error[i]);
    }
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(x),t,real(y));\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,imag(x),t,imag(y));\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('imag');\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,e);\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('phase error');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n",OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
