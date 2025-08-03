// show delay in symstreamr object
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "symstreamrcf_delay_example.m"

int main()
{
    // symstream parameters
    int          ftype       = LIQUID_FIRFILT_ARKAISER;
    float        bw          = 0.23456789f;
    unsigned int m           =  8;
    float        beta        = 0.30f;
    int          ms          = LIQUID_MODEM_QPSK;

    // create stream generator
    symstreamrcf gen = symstreamrcf_create_linear(ftype,bw,m,beta,ms);
    float delay = symstreamrcf_get_delay(gen);

    unsigned int buf_len = 100;
    float complex buf[2*buf_len];

    // write samples to buffer
    symstreamrcf_write_samples(gen, buf, buf_len);
    symstreamrcf_set_gain(gen, 0.0f);
    symstreamrcf_write_samples(gen, buf+buf_len, buf_len);

    // destroy objects
    symstreamrcf_destroy(gen);

    // measure approximate delay; index of first sample with abs > 1
    unsigned int i;
    for (i=0; i<2*buf_len; i++) {
        if (cabsf(buf[i]) > 1.0f)
            break;
    }
    printf("expected delay: %.3f, approximate delay: %u\n", delay, i);

    // export output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"n = %u; delay = %.6f;\n", buf_len, delay);
    fprintf(fid,"t = [0:(2*n-1)] - delay;\n");
    for (i=0; i<2*buf_len; i++)
        fprintf(fid,"v(%6u) = %12.4e + %12.4ej;\n", i+1, crealf(buf[i]), cimagf(buf[i]));
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t, real(v), t, imag(v));\n");
    fprintf(fid,"xlabel('Sample Index');\n");
    fprintf(fid,"ylabel('Signal Output');\n");
    fprintf(fid,"legend('real','imag');\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    printf("done.\n");
    return 0;
}

