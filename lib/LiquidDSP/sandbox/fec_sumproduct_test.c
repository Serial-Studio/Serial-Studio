//
// fec_sumproduct_test.c
//
// Test soft decoding of LDPC codes using internal sum-product
// algorithm.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

int main(int argc, char*argv[])
{
    // parity check matrix
    unsigned char Hb[32] = {
        1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 0, 0,
        1, 0, 0, 1, 0, 0, 1, 0,
        0, 1, 0, 0, 1, 0, 0, 1};

    // convert H to sparse matrix
    smatrixb H = smatrixb_create_array(Hb, 4, 8);

    // transmitted codeword
    unsigned char c[8] = {
        1, 0, 1, 0, 1, 1, 1, 1};

    // received message
    float y[8] = {0.2, 0.2, -0.9, 0.6, 0.5, -1.1, -0.4, -1.2};
    unsigned int i;

    // noise standard deviation
    float sigma = sqrtf(0.5f);

    unsigned int m = 4;
    unsigned int n = 8;

    // convert received signal to LLR
    float LLR[n];
    for (i=0; i<n; i++)
        LLR[i] = 2.0f * y[i] / (sigma*sigma);

    unsigned int max_iterations = 10;
    unsigned char c_hat[n];

    // run internal sum-product algorithm
    int parity_pass = fec_sumproduct(m, n, H, LLR, c_hat, max_iterations);

    // compute errors and print results
    unsigned int num_errors = 0;
    for (i=0; i<n; i++)
        num_errors += (c[i] == c_hat[i]) ? 0 : 1;

    printf("\nresults:\n");

    printf("c    :");
    for (i=0; i<n; i++)
        printf(" %1u", c[i]);
    printf("\n");

    printf("c_hat:");
    for (i=0; i<n; i++)
        printf(" %1u", c_hat[i]);
    printf("\n");

    printf("parity : %s\n", parity_pass ? "pass" : "FAIL");
    printf("num errors: %u / %u\n", num_errors, n);

    return 0;
}

