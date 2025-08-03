//
// complementary_codes_example.c
// 
// This example demonstrates how to generate complementary binary
// codes in liquid.  A pair of codes is generated using the bsequence
// interface, their auto-correlations are computed, and the result is
// summed and printed to the screen.  The results are also printed to an
// output file, which plots the sequences and their auto-correlations.
//
// SEE ALSO: bsequence_example.c
//           msequence_example.c

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "complementary_codes_example.m"

// print usage/help message
void usage()
{
    printf("complementary_codes_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  n     : sequence length, 8,16,32,64,... (default: 32)\n");
}


int main(int argc, char*argv[])
{
    // options
    unsigned int n = 32;

    int dopt;
    while ((dopt = getopt(argc,argv,"uhn:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();                return 0;
        case 'n':   n = atoi(optarg);       break;
        default:
            exit(1);
        }
    }

    // validate input
    if ( (1<<liquid_nextpow2(n)) != n ) {
        fprintf(stderr,"error: %s, sequence length must be a power of 2\n", argv[0]);
        exit(1);
    }

    // create and initialize codes
    bsequence a = bsequence_create(n);
    bsequence b = bsequence_create(n);
    bsequence_create_ccodes(a, b);

    // print
    bsequence_print(a);
    bsequence_print(b);

    // generate test sequences
    bsequence ax = bsequence_create(n);
    bsequence bx = bsequence_create(n);
    bsequence_create_ccodes(ax, bx);

    // compute auto-correlations
    unsigned int i;
    int raa[n];
    int rbb[n];
    for (i=0; i<n; i++) {
        raa[i] = 2*bsequence_correlate(a,ax) - n;
        rbb[i] = 2*bsequence_correlate(b,bx) - n;

        bsequence_circshift(ax);
        bsequence_circshift(bx);

        // print to screen
        printf("  raa[%4u] = %4d, rbb[%4u] = %4d, sum = %4d\n",
                i, raa[i], i, rbb[i], raa[i] + rbb[i]);
    }

    // print results to file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, cannot open output file '%s' for writing\n", argv[0], OUTPUT_FILENAME);
        exit(1);
    }

    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"n = %u;\n", n);
    fprintf(fid,"a = zeros(1,n);\n");
    fprintf(fid,"b = zeros(1,n);\n");
    fprintf(fid,"raa = zeros(1,n);\n");
    fprintf(fid,"rbb = zeros(1,n);\n");

    for (i=0; i<n; i++) {
        fprintf(fid,"a(%6u) = %1u;\n", i+1, bsequence_index(a,n-i-1));
        fprintf(fid,"b(%6u) = %1u;\n", i+1, bsequence_index(b,n-i-1));

        fprintf(fid,"raa(%6u) = %12.8f;\n", i+1, raa[i] / (float)n);
        fprintf(fid,"rbb(%6u) = %12.8f;\n", i+1, rbb[i] / (float)n);
    }

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"   stairs(t,a);\n");
    fprintf(fid,"   axis([-1 n -0.2 1.2]);\n");
    //fprintf(fid,"   xlabel('delay (samples)');\n");
    fprintf(fid,"   ylabel('a');\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"   stairs(t,b);\n");
    fprintf(fid,"   axis([-1 n -0.2 1.2]);\n");
    //fprintf(fid,"   xlabel('delay (samples)');\n");
    fprintf(fid,"   ylabel('b');\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"   plot(t,raa,t,rbb,t,[raa+rbb]/2);\n");
    fprintf(fid,"   axis([-1 n -0.5 1.2]);\n");
    fprintf(fid,"   xlabel('delay (samples)');\n");
    fprintf(fid,"   ylabel('auto-correlation');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean up memory
    bsequence_destroy(a);
    bsequence_destroy(b);
    bsequence_destroy(ax);
    bsequence_destroy(bx);

    return 0;
}

