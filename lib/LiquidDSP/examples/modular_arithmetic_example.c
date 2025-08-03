//
// modular_arithmetic_example.c
//
// This example demonstrates some modular arithmetic functions.
//

#include <stdio.h>
#include "liquid.h"

int main() {
    unsigned int n=280;
    unsigned int factors[LIQUID_MAX_FACTORS];
    unsigned int num_factors=0;

    // compute factors of n
    liquid_factor(n,factors,&num_factors);
    printf("factors of %u:\n", n);
    unsigned int i;
    for (i=0; i<num_factors; i++)
        printf("%3u\n", factors[i]);
    
    // compute unique factors
    liquid_unique_factor(n,factors,&num_factors);
    printf("unique factors of %u:\n", n);
    for (i=0; i<num_factors; i++)
        printf("%3u\n", factors[i]);

    // compute Euler's totient function
    unsigned int t = liquid_totient(n);
    printf("totient(%u) = %u\n", n, t);

    printf("done.\n");
    return 0;
}

