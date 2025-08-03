// msequence_generator_example.c
//
// This example demonstrates finding maximal-length sequence
// (m-sequence) generator polynomials of a certain length.

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "liquid.h"

void usage()
{
    printf("msequence_generator_example [options]\n");
    printf(" -h          : print usage\n");
    printf(" -d <degree> : degree of polynomial, default: 2\n");
}

int main(int argc, char *argv[])
{
    unsigned int degree = 2;

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"uhd:")) != EOF){
        switch (dopt) {
        case 'h': usage();               return 0;
        case 'd': degree = atol(optarg); break;
        default:
            exit(-1);
        }
    }

    unsigned int maxpoly = (1 << degree) - 1;
    unsigned int expected_sum = ((maxpoly + 1) / 2) * maxpoly;
    unsigned int i;
    unsigned int poly;
    for (poly = 0; poly <= maxpoly; ++poly) {
        unsigned int g = (poly << 1) + 1;
        msequence seq = msequence_create(degree, g, 1);
        unsigned int sum = 0;
        for (i = 0; i < maxpoly; ++i) {
            sum += msequence_get_state(seq);
            msequence_advance(seq);
        }
        if (sum == expected_sum) {
            printf("degree %d poly: %#06x\n", degree, g);
        }
        msequence_destroy(seq);
    }

    return 0;
}
