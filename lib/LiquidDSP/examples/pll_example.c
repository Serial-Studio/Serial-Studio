//
// pll_example.c
// 
// Demonstrates a basic phase-locked loop to track the phase of a
// complex sinusoid.
//

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <time.h>

#include "liquid.h"

// output to octave-friendly format
#define OUTPUT_FILENAME "pll_example.m"

int main() {
    // parameters
    float phase_offset      = 0.8f;     // carrier phase offset
    float frequency_offset  = 0.01f;    // carrier frequency offset
    float wn                = 0.05f;    // pll bandwidth
    float zeta              = 0.707f;   // pll damping factor
    float K                 = 1000;     // pll loop gain
    unsigned int n          = 256;      // number of samples
    unsigned int d          = n/32;     // print every "d" lines

    //
    float theta[n];         // input phase
    float complex x[n];     // input sinusoid
    float phi[n];           // output phase
    float complex y[n];     // output sinusoid

    // generate iir loop filter object
    iirfilt_rrrf H = iirfilt_rrrf_create_pll(wn, zeta, K);
    iirfilt_rrrf_print(H);

    unsigned int i;

    // generate input
    float t=phase_offset;
    float dt = frequency_offset;
    for (i=0; i<n; i++) {
        theta[i] = t;
        x[i] = cexpf(_Complex_I*theta[i]);

        t += dt;
    }

    // run loop
    float phi_hat=0.0f;
    for (i=0; i<n; i++) {
        y[i] = cexpf(_Complex_I*phi_hat);

        // compute error
        float e = cargf(x[i]*conjf(y[i]));

        if ( (i%d)==0 )
            printf("e(%3u) = %12.8f;\n", i, e);

        // filter error
        iirfilt_rrrf_execute(H,e,&phi_hat);

        phi[i] = phi_hat;
    }

    // destroy filter
    iirfilt_rrrf_destroy(H);

    // open output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"n=%u;\n",n);

    for (i=0; i<n; i++) {
        fprintf(fid,"theta(%3u) = %16.8e;\n", i+1, theta[i]);
        fprintf(fid,"  phi(%3u) = %16.8e;\n", i+1, phi[i]);
    }
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,theta,t,phi);\n");
    fprintf(fid,"xlabel('sample index');\n");
    fprintf(fid,"ylabel('phase');\n");

    fclose(fid);

    printf("output written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
