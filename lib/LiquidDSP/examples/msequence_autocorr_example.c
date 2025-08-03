// This example demonstrates the auto-correlation properties of a
// maximal-length sequence (m-sequence).  An m-sequence of a
// certain length is used to generate two binary sequences
// (buffers) which are then cross-correlated.  The resulting
// correlation produces -1 for all values except at index zero,
// where the sequences align.
//
// SEE ALSO: bsequence_example.c
//
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "msequence_autocorr_example.m"

int main(int argc, char*argv[])
{
    // options
    unsigned int m=5;   // shift register length, n=2^m - 1

    // create and initialize m-sequence
    msequence ms = msequence_create_default(m);
    msequence_print(ms);
    unsigned int n = msequence_get_length(ms);
    signed int rxx[n];  // auto-correlation

    // create and initialize first binary sequence on m-sequence
    bsequence bs1 = bsequence_create(n);
    bsequence_init_msequence(bs1, ms);

    // create and initialize second binary sequence on same m-sequence
    bsequence bs2 = bsequence_create(n);
    bsequence_init_msequence(bs2, ms);

    // when sequences are aligned, autocorrelation is equal to length
    rxx[0] = 2*bsequence_correlate(bs1, bs2) - n;

    // when sequences are misaligned, autocorrelation is equal to -1
    unsigned int i;
    for (i=0; i<n; i++) {
        // compute auto-correlation
        rxx[i] = 2*bsequence_correlate(bs1, bs2)-n;

        // circular shift the second sequence
        bsequence_circshift(bs2);
    }
    
    // p/n sequence
    signed int x[n];
    for (i=0; i<n; i++)
        x[i] = bsequence_index(bs1, i);

    // clean up memory
    bsequence_destroy(bs1);
    bsequence_destroy(bs2);
    msequence_destroy(ms);

    // print results
    for (i=0; i<n; i++)
        printf("rxx[%3u] = %3d\n", i, rxx[i]);

    //
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, cannot open output file '%s' for writing\n", argv[0], OUTPUT_FILENAME);
        exit(1);
    }

    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"n = %u;\n", n);
    fprintf(fid,"p = zeros(1,n);\n");

    for (i=0; i<n; i++) {
        fprintf(fid,"x(%6u)   = %3d;\n",    i+1, x[i]);
        fprintf(fid,"rxx(%6u) = %12.8f;\n", i+1, rxx[i] / (float)n);
    }

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"   stairs(t,x);\n");
    fprintf(fid,"   axis([-1 n -0.2 1.2]);\n");
    fprintf(fid,"   ylabel('x');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"   plot(t,rxx,t,rxx);\n");
    fprintf(fid,"   axis([-1 n -0.5 1.2]);\n");
    fprintf(fid,"   xlabel('delay (samples)');\n");
    fprintf(fid,"   ylabel('auto-correlation');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    return 0;
}

