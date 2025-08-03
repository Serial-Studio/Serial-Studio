//
// gasearch_knapsack_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "gasearch_knapsack_example.m"

// print usage/help message
void usage()
{
    printf("Usage: gasearch_knapsack_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  n     : number of items available, default: 1000\n");
    printf("  i     : number of iterations (generations) to run, default: 2000\n");
    printf("  c     : knapsack capacity (maximum weight), default: 20\n");
    printf("  p     : ga population size, default: 100\n");
    printf("  m     : ga mutation rate, default: 0.4\n");
}

// knapsack object structure definition
struct knapsack_s {
    unsigned int num_items; // total number of items available
    float * weight;         // weight of each item
    float * value;          // value of each item
    float capacity;         // maximum weight allowable
};

// print knapsack object
//  _bag        :   knapsack object pointer
//  _c          :   test chromosome
void  knapsack_print(struct knapsack_s * _bag,
                     chromosome _c);

// utility callback function
//  _userdata   :   knapsack object pointer
//  _c          :   test chromosome
float knapsack_utility(void * _userdata,
                       chromosome _c);

int main(int argc, char*argv[])
{
    unsigned int num_items = 1000;      // number of items available
    unsigned int num_iterations = 2000; // number of iterations to run
    float capacity = 20.0f;             // total capacity of the knapsack
    unsigned int population_size = 100; // number of chromosomes in the population
    float mutation_rate = 0.40f;        // mutation rate of the GA

    int dopt;
    while((dopt = getopt(argc,argv,"uhn:i:c:p:m:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage(); return 0;
        case 'n': num_items = atoi(optarg);         break;
        case 'i': num_iterations = atoi(optarg);    break;
        case 'c': capacity = atof(optarg);          break;
        case 'p': population_size = atoi(optarg);   break;
        case 'm': mutation_rate = atof(optarg);     break;
        default:
            exit(1);
        }
    }

    // validate input
    if (num_items == 0) {
        fprintf(stderr,"error: %s, knapsack must have at least 1 item\n", argv[0]);
        exit(1);
    } else if (capacity <= 0.0f) {
        fprintf(stderr,"error: %s, knapsack capacity must be greater than zero\n", argv[0]);
        exit(1);
    } else if (population_size <= 0) {
        fprintf(stderr,"error: %s, ga population size must be greater than zero\n", argv[0]);
        exit(1);
    } else if (mutation_rate < 0.0f || mutation_rate > 1.0f) {
        fprintf(stderr,"error: %s, ga mutation rate must be in [0,1]\n", argv[0]);
        exit(1);
    }

    unsigned int i;

    // create knapsack/items (random weight, value)
    struct knapsack_s bag;
    bag.num_items = num_items;
    bag.capacity = capacity;
    bag.weight = (float*) malloc( bag.num_items*sizeof(float) );
    bag.value  = (float*) malloc( bag.num_items*sizeof(float) );
    for (i=0; i<num_items; i++) {
        bag.weight[i] = randf();    // random number in [0,1]
        bag.value[i]  = randf();    // random number in [0,1]
    }

    // create prototype chromosome (1 bit/item)
    chromosome prototype = chromosome_create_basic(num_items, 1);
    //chromosome_init_random(prototype); // initialize to random
    chromosome_print(prototype);

    // print knapsack
    knapsack_print(&bag, prototype);

    float optimum_utility;

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    // create gasearch object
    gasearch ga = gasearch_create_advanced(&knapsack_utility,
                                             (void*)&bag,
                                             prototype,
                                             LIQUID_OPTIM_MAXIMIZE,
                                             population_size,
                                             mutation_rate);

    // execute search one iteration at a time
    fprintf(fid,"u = zeros(1,%u);\n", num_iterations);
    for (i=0; i<num_iterations; i++) {
        gasearch_evolve(ga);

        gasearch_getopt(ga, prototype, &optimum_utility);
        if (((i+1)%100)==0)
            printf("  %4u : %12.8f;\n", i+1, optimum_utility);

        fprintf(fid,"u(%3u) = %12.4e;\n", i+1, optimum_utility);
    }

    // print results
    gasearch_getopt(ga, prototype, &optimum_utility);
    knapsack_print(&bag, prototype);

    fprintf(fid,"figure;\n");
    //fprintf(fid,"semilogy(u);\n");
    fprintf(fid,"plot(u);\n");
    fprintf(fid,"xlabel('iteration');\n");
    fprintf(fid,"ylabel('utility');\n");
    fprintf(fid,"title('GA search results');\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // free allocated objects and memory
    chromosome_destroy(prototype);
    gasearch_destroy(ga);
    free(bag.value);
    free(bag.weight);

    return 0;
}


// 
// knapsack methods
//

// print knapsack object
//  _bag        :   knapsack object pointer
//  _c          :   test chromosome
void knapsack_print(struct knapsack_s * _bag,
                    chromosome _s)
{
    unsigned int i;
    printf("knapsack: %u items, capacity : %12.8f\n", _bag->num_items, _bag->capacity);
    for (i=0; i<_bag->num_items; i++) {
        printf("  %3u : %6.4f @ $%6.4f", i, _bag->weight[i], _bag->value[i]);
        unsigned int n = chromosome_value(_s, i);
        if (n != 0) printf(" *\n");
        else        printf("\n");
    }
}

// utility callback function
//  _userdata   :   knapsack object pointer
//  _c          :   test chromosome
float knapsack_utility(void * _userdata, chromosome _c)
{
    struct knapsack_s * _bag = (struct knapsack_s *) _userdata;

    // chromosome represents number of each item in knapsack
    float total_value = 0;
    float total_weight = 0;
    unsigned int i;
    for (i=0; i<_bag->num_items; i++) {
        if ( chromosome_value(_c,i) == 1 ) {
            // include this item into knapsack
            total_value  += _bag->value[i];
            total_weight += _bag->weight[i];
        }
    }

    // check for invalid solution, returning distance metric
    if (total_weight > _bag->capacity)
        return _bag->capacity - total_weight;

    // return total value of knapsack
    return total_value;
}

