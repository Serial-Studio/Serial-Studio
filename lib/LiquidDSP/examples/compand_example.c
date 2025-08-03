//
// compand_example.c
//
// This example demonstrates the interface to the compand function
// (compression, expansion).  The compander is typically used with the
// quantizer to increase the dynamic range of the converter, particularly for
// low-level signals.  The transfer function is computed (empirically) and
// printed to the screen.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "compand_example.m"

// print usage/help message
void usage()
{
    printf("compand_example\n");
    printf("  u/h   : print usage/help\n");
    printf("  v/q   : verbose/quiet, default: verbose\n");
    printf("  n     : number of samples, default: 31\n");
    printf("  m     : companding parameter: mu > 0, default: 255\n");
    printf("  r     : range > 0, default: 1.25\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int n=31;
    float mu=255.0f;
    float range = 1.25f;
    int verbose = 1;

    int dopt;
    while ((dopt = getopt(argc,argv,"uhvqn:m:r:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();              return 0;
        case 'v': verbose = 1;          break;
        case 'q': verbose = 0;          break;
        case 'n': n = atoi(optarg);     break;
        case 'm': mu = atof(optarg);    break;
        case 'r': range = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (mu < 0) {
        fprintf(stderr,"error: %s, mu must be positive\n", argv[0]);
        usage();
        exit(1);
    } else if (range <= 0) {
        fprintf(stderr,"error: %s, range must be greater than zero\n", argv[0]);
        usage();
        exit(1);
    }

    // open debug file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");

    float x = -range;
    float y, z;
    float dx = 2.0*range/(float)(n-1);
    unsigned int i;
    for (i=0; i<n; i++) {
        y = compress_mulaw(x,mu);
        z = expand_mulaw(y,mu);
        if (verbose)
            printf("%8.4f > %8.4f > %8.4f\n", x, y, z);

        fprintf(fid,"x(%3u) = %12.4e;\n", i+1, x);
        fprintf(fid,"y(%3u) = %12.4e;\n", i+1, y);
        fprintf(fid,"z(%3u) = %12.4e;\n", i+1, z);

        x += dx;
    }

    // plot results
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(x,y,'-b','LineWidth',2,x,z,'-r');\n");
    fprintf(fid,"axis square\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('x');\n");
    fprintf(fid,"ylabel('f(x)');\n");

    // close debug file
    fclose(fid);
    printf("results wrtten to %s\n", OUTPUT_FILENAME);
    printf("done.\n");
    return 0;
}

