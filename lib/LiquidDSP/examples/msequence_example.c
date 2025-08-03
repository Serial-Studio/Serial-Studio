// This example demonstrates the property of maximal-length sequence
// (m-sequence) linear feedback shift registers (LFSR) where the state
// cycles through all permutations of integers from 1 to 2^m-1.
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>

#include "liquid.h"

int main(int argc, char*argv[])
{
    // create and initialize m-sequence
    msequence q = msequence_create_default(5);
    msequence_print(q);

    // cycle through values and print state
    unsigned int i;
    for (i=0; i<msequence_get_length(q); i++) {
        // verify we never hit the '1' state except on the first iteration
        if (i > 0 && msequence_get_state(q)==1) {
            printf("invalid state!\n");
            break;
        }
        printf("%u\n",msequence_get_state(q));
        msequence_advance(q);
    }

    // ensure final state is 1 (circled all the way back around)
    printf("final state (should be 1): %u\n", msequence_get_state(q));

    msequence_destroy(q);
    return 0;
}

