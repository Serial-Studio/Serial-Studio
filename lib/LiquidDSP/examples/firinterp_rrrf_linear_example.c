// This example demonstrates the a linear interpolator.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firinterp_rrrf_linear_example.m"

int main(int argc, char*argv[]) {
    // options
    unsigned int M          =  8;   // interpolation factor
    unsigned int num_symbols= 16;   // number of data symbols

    // generate input signal and interpolate
    float x[  num_symbols];   // input symbols
    float y[M*num_symbols];   // output samples
    unsigned int i;
    for (i=0; i<num_symbols; i++)
        x[i] = randnf();

    // create and run interpolator
    firinterp_rrrf q = firinterp_rrrf_create_linear(M);
    firinterp_rrrf_execute_block(q, x, num_symbols, y);
    firinterp_rrrf_destroy(q);

    // print results to screen
    printf("x(t) :\n");
    for (i=0; i<num_symbols; i++)
        printf("  x[%4u] = %8.4f\n", i, x[i]);

    printf("y(t) :\n");
    for (i=0; i<M*num_symbols; i++)
        printf("  y[%4u] = %8.4f%s\n", i, y[i], (i >= M) && ((i%M)==0) ? " **" : "");

    // export output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"M = %u;\n", M);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = M*num_symbols;\n");
    fprintf(fid,"x = zeros(1,num_symbols);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_symbols; i++)
        fprintf(fid,"x(%4u)=%12.4e;\n", i+1, x[i]);
    for (i=0; i<M*num_symbols; i++)
        fprintf(fid,"y(%4u)=%12.4e;\n", i+1, y[i]);
    fprintf(fid,"\n\n");
    fprintf(fid,"tx = [0:(num_symbols-1)];\n");
    fprintf(fid,"ty = [0:(num_samples-1)]/M - 1;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(ty,real(y),'-',tx,real(x),'s');\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('real');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s.\n",OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
