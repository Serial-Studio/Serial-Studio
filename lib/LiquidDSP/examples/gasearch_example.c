// example demonstrating performance of GA search algorithm for finding basic function peak
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "gasearch_example.m"

// peak callback function; value nearest {p, p, p, ...} where p = 1/sqrt(2)
float peak_callback(void * _userdata, chromosome _c)
{
    unsigned int i, n = chromosome_get_num_traits(_c);
    float u_global = 1.0f;
    float sig      = 0.2f;
    float p        = M_SQRT1_2;
    for (i=0; i<n; i++) {
        // extract chromosome values
        float v = chromosome_valuef(_c,i);
        float e = v - p;
        float u = exp(-e*e/(2*sig*sig));
        u_global *= u;
    }
    return u_global;
}

int main() {
    unsigned int num_parameters     = 16;   // dimensionality of search (minimum 1)
    unsigned int bits_per_parameter =  6;   // parameter resolution
    unsigned int num_iterations     = 8000; // number of iterations to run
    unsigned int population_size    = 32;   // GA population size
    float        mutation_rate      = 0.2f; // GA mutation rate

    unsigned int i;
    float optimum_utility;

    // create prototype chromosome
    chromosome prototype = chromosome_create_basic(num_parameters, bits_per_parameter);

    // create gasearch object
    gasearch ga = gasearch_create_advanced(peak_callback, NULL, prototype,
                    LIQUID_OPTIM_MAXIMIZE, population_size, mutation_rate);
    gasearch_print(ga);

    // execute search at once
    //optimum_utility = gasearch_run(ga, num_iterations, -1e-6f);

    // open output file for exporting results
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    // execute search one iteration at a time
    fprintf(fid,"u = zeros(1,%u);\n", num_iterations);
    for (i=0; i<num_iterations; i++) {
        // step
        gasearch_evolve(ga);

        // get optimum utility and print results
        gasearch_getopt(ga, prototype, &optimum_utility);
        fprintf(fid,"u(%3u) = %12.4e;\n", i+1, optimum_utility);
        printf("%4u : %16.8f\n", i, optimum_utility);
    }

    // print results
    printf("\n");
    gasearch_print(ga);

    printf("optimum utility : %12.8f\n", optimum_utility);
    chromosome_printf(prototype);
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(u);\n");
    fprintf(fid,"xlabel('iteration');\n");
    fprintf(fid,"ylabel('utility');\n");
    fprintf(fid,"title('gradient search results');\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    chromosome_destroy(prototype);
    gasearch_destroy(ga);
    return 0;
}

