// generate tone at low frequency and test phase response
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "liquid.h"
#define OUTPUT_FILENAME "nco_crcf_tone_example.m"

int main()
{
    // options
    int          type        = LIQUID_NCO;      // nco type
    float        fc          = 0.000241852307f; // frequency
    unsigned int num_samples = 2400;            // number of samples to run

    // create the NCO object
    nco_crcf q = nco_crcf_create(type);
    nco_crcf_set_frequency(q, 2*M_PI*fc);
    nco_crcf_print(q);

    unsigned int i;
    float complex x[num_samples];
    for (i=0; i<num_samples; i++) {
        nco_crcf_cexpf(q, &x[i]);
        nco_crcf_step(q);
    }

    // destroy objects
    nco_crcf_destroy(q);

    // export output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"n = %u;\n", num_samples);
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"x = zeros(1,n);\n");
    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%6u) = %12.9f + %12.9fj;\n", i+1, crealf(x[i]), cimagf(x[i]));
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t, real(x), '-', 'LineWidth',1.5,...\n");
    fprintf(fid,"     t, imag(x), '-', 'LineWidth',1.5);\n");
    fprintf(fid,"xlabel('Signal');\n");
    fprintf(fid,"ylabel('Time [sample index]');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([0 n -1 1]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    return 0;
}
