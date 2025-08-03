// 
// matrix_eig_test.c
//
// Test eigenvalue computation using Q/R decomposition. Eigenvalues
// will be on diagonal if they are all real.
//

#include <stdio.h>
#include <string.h>
#include "liquid.h"

int main() {
    unsigned int num_iterations=8;

#if 0
    unsigned int n=4;
    float A[16] = {
        1,2,3,4,
        5,5,7,8,
        6,4,8,7,
        1,0,3,1};
#else
    unsigned int n=3;
    float A[9] = {
        1.0f, 1.0f, 1.0f,
        1.0f, 2.0f, 1.0f,
        1.0f, 1.0f, 2.0f};
#endif

    float eig[n];
    float A0[n*n];
    float Q[n*n], R[n*n];

    memmove(A0, A, n*n*sizeof(float));

    printf("\n");
    printf("testing Q/R decomposition [Gram-Schmidt]\n");
    matrixf_qrdecomp_gramschmidt(A,n,n,Q,R);
    //matrixf_print(Q,n,n);
    //matrixf_print(R,n,n);

    unsigned int k;
    for (k=0; k<num_iterations; k++) {
        // compute Q/R decomposition
        matrixf_qrdecomp_gramschmidt(A0,n,n,Q,R);

        // compute A1 = R*Q
        matrixf_mul(R,n,n, Q,n,n, A0,n,n);

        matrixf_print(A0,n,n);
    }

    // extract eigen values
    unsigned int i;
    for (i=0; i<n; i++)
        eig[i] = matrix_access(A0,n,n,i,i);

    for (i=0; i<n; i++)
        printf("eig[%3u] = %12.8f\n", i, eig[i]);

    printf("done.\n");
    return 0;
}

