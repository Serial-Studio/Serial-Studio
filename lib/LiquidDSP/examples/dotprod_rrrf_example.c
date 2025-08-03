//
// dotprod_rrrf_example.c
//
// This example demonstrates the interface to the floating-point dot
// product object (dotprod_rrrf).
//

#include <stdio.h>
#include "liquid.h"

int main() {
    // input array
    float x[] = { 1,  2,  3,  4,  5};

    // coefficients array
    float h[] = { 1, -1,  1, -1,  1};

    // dot product result
    float y;

    // run regular dot product
    dotprod_rrrf_run(x,h,5,&y);
    printf("dotprod_rrrf              : %8.2f\n", y);

    // run structured dot product
    dotprod_rrrf q = dotprod_rrrf_create(x,5);
    dotprod_rrrf_execute(q,h,&y);
    printf("dotprod_rrrf (structured) : %8.2f\n", y);
    dotprod_rrrf_print(q);
    dotprod_rrrf_destroy(q);

    return 0;
}


