//
// windowf_example.c
//
// This example demonstrates the functionality of a window buffer (also
// known as a circular or ring buffer) of floating-point values.  Values
// are written to and read from the buffer using several different
// methods.
//
// SEE ALSO: bufferf_example.c
//           wdelayf_example.c
//

#include <stdio.h>

#include "liquid.h"

int main() {
    // initialize vector of data for testing
    float v[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    float *r; // reader
    unsigned int i;

    // create window: 10 elements, initialized to 0
    // w: 0 0 0 0 0 0 0 0 0 0
    windowf w = windowf_create(10);

    // push 4 elements
    // w: 0 0 0 0 0 0 1 1 1 1
    windowf_push(w, 1);
    windowf_push(w, 1);
    windowf_push(w, 1);
    windowf_push(w, 1);

    // push 4 more elements
    // w: 0 0 1 1 1 1 9 8 7 6
    windowf_write(w, v, 4);

    // push 4 more elements
    // w: 1 1 9 8 7 6 3 3 3 3
    windowf_push(w, 3);
    windowf_push(w, 3);
    windowf_push(w, 3);
    windowf_push(w, 3);

    // read the buffer by assigning the pointer
    // appropriately
    windowf_read(w, &r);

    // manual print
    printf("manual output:\n");
    for (i=0; i<10; i++)
        printf("%6u : %f\n", i, r[i]);

    windowf_debug_print(w);

    // clean it up
    windowf_destroy(w);

    printf("done.\n");
    return 0;
}


