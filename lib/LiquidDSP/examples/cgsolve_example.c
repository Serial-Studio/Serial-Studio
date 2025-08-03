// 
// cgsolve_example.c
//
// Solve linear system of equations A*x = b using the conjugate-
// gradient method where A is a symmetric positive-definite matrix.
// Compare speed to matrixf_linsolve() for same system.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.h"

int main() {
    // options
    unsigned int n = 8;

    unsigned int i;

    // allocate memory for arrays
    float A[n*n];
    float b[n];
    float x[n];
    float x_hat[n];

    // generate symmetric positive-definite matrix by first generating
    // lower triangular matrix L and computing A = L*L'
    float L[n*n];
    unsigned int j;
    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
#if 0
            // sparse matrix
            if (j > i)              matrix_access(L,n,n,i,j) = 0.0;
            else if (j == i)        matrix_access(L,n,n,i,j) = randnf();
            else if ((rand()%4)==0) matrix_access(L,n,n,i,j) = randnf();
            else                    matrix_access(L,n,n,i,j) = 0.0;
#else
            // full matrix
            matrix_access(L,n,n,i,j) = (j < i) ? 0.0 : randnf();
#endif
        }
    }
    matrixf_mul_transpose(L, n, n, A);

    // generate random solution
    for (i=0; i<n; i++)
        x[i] = randnf();

    // compute b
    matrixf_mul(A, n, n,
                x, n, 1,
                b, n, 1);

    // solve symmetric positive-definite system of equations
    matrixf_cgsolve(A, n, b, x_hat, NULL);
    //matrixf_linsolve(A, n, b, x_hat, NULL);

    // print results

    printf("A:\n");             matrixf_print(A,     n, n);
    printf("b:\n");             matrixf_print(b,     n, 1);
    printf("x (original):\n");  matrixf_print(x,     n, 1);
    printf("x (estimate):\n");  matrixf_print(x_hat, n, 1);

    // compute error norm
    float e = 0.0;
    for (i=0; i<n; i++)
        e += (x[i] - x_hat[i])*(x[i] - x_hat[i]);
    e = sqrt(e);
    printf("error norm: %12.4e\n", e);

    printf("done.\n");
    return 0;
}

