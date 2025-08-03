//
// dotprod_cccf_example.c
//
// This example demonstrates the interface to the complex floating-point
// dot product object (dotprod_cccf).
//

#include <stdio.h>
#include "liquid.h"

int main() {
    // input array
    float complex x[] = { 1 + 1 * _Complex_I,
                          2 + 1 * _Complex_I,
                          3 + 1 * _Complex_I,
                          4 + 1 * _Complex_I,
                          5 + 1 * _Complex_I};

    // coefficients array
    float complex h[] = { 1 + 1 * _Complex_I,
                         -1 + 1 * _Complex_I,
                          1 + 1 * _Complex_I,
                         -1 + 1 * _Complex_I,
                          1 + 1 * _Complex_I};

    // dot product result
    float complex y;

    // run regular dot product
    dotprod_cccf_run(x,h,5,&y);
    printf("dotprod_cccf              : %8.2f + j%8.2f\n", crealf(y), cimagf(y));

    // run structured dot product
    dotprod_cccf q = dotprod_cccf_create(x,5);
    dotprod_cccf_execute(q,h,&y);
    printf("dotprod_cccf (structured) : %8.2f + j%8.2f\n", crealf(y), cimagf(y));
    dotprod_cccf_destroy(q);

    return 0;
}


