//
// gradsearch_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "gradsearch_example.m"

// print usage/help message
void usage()
{
    printf("%s [options]\n", __FILE__);
    printf("  h     : print help\n");
    printf("  n     : number of parameters, default: 6\n");
    printf("  t     : number of iterations, default: 2000\n");
    printf("  u     : utility function: {rosenbrock, invgauss, multimodal, spiral}\n");
}

int main(int argc, char*argv[])
{
    unsigned int num_parameters = 6;    // dimensionality of search (minimum 2)
    unsigned int num_iterations = 2000; // number of iterations to run
    utility_function func = liquid_rosenbrock;

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:t:u:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                        return 0;
        case 'n':   num_parameters = atoi(optarg);  break;
        case 't':   num_iterations = atoi(optarg);  break;
        case 'u':
            if      (strcmp(optarg,"rosenbrock")==0) func = liquid_rosenbrock;
            else if (strcmp(optarg,"invgauss")==0)   func = liquid_invgauss;
            else if (strcmp(optarg,"multimodal")==0) func = liquid_multimodal;
            else if (strcmp(optarg,"spiral")==0)     func = liquid_spiral;
            else {
                fprintf(stderr,"error: %s, unknown/unsupported utility '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    float optimum_vect[num_parameters];
    unsigned int i;
    for (i=0; i<num_parameters; i++)
        optimum_vect[i] = 0.1f*(float)i;

    float optimum_utility;

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    // create gradsearch object
    gradsearch gs = gradsearch_create(NULL,
                                      optimum_vect,
                                      num_parameters,
                                      func,
                                      LIQUID_OPTIM_MINIMIZE);

    // execute search
    //optimum_utility = gradsearch_run(gs, num_iterations, -1e-6f);

    // execute search one iteration at a time
    fprintf(fid,"u = zeros(1,%u);\n", num_iterations);
    unsigned int d=1;
    for (i=0; i<num_iterations; i++) {
        optimum_utility = func(NULL,optimum_vect,num_parameters);
        fprintf(fid,"u(%3u) = %12.4e;\n", i+1, optimum_utility);

        gradsearch_step(gs);

        if (((i+1)%d)==0 || i==0) {
            printf("%5u: ", i+1);
            gradsearch_print(gs);

            if ((i+1)==10*d) d*=10;
        }
    }

    // print results
    printf("\n");
    printf("%5u: ", num_iterations);
    gradsearch_print(gs);

    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(u);\n");
    fprintf(fid,"xlabel('iteration');\n");
    fprintf(fid,"ylabel('utility');\n");
    fprintf(fid,"title('gradient search results');\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // test results, optimum at [1, 1, 1, ... 1];

    gradsearch_destroy(gs);

    return 0;
}
