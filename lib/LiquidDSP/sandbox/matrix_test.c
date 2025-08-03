// 
// matrix_test.c
//
// This example tests basic matrix operations.
//

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "liquid.h"

int main() {

    float x[6] = {
        1, 2, 3,
        4, 5, 6};

    float y[9] = {
        1, 2, 3,
        4, 5, 6,
        7, 8, 9};

    float z[6];

    // compute z = x * y
    printf("z = x * y :\n");
    matrixf_mul(x,2,3,y,3,3,z,2,3);
    matrixf_print(z,2,3);

    /*
    // compute z = y * x'
    matrixf_transpose(x);
    printf("x' : \n");
    matrixf_print(x);
    matrixf_transpose(z);
    matrixf_multiply(y,x,z);
    printf("z = y * x' :\n");
    matrixf_print(z);

    matrixf_destroy(x);
    matrixf_destroy(y);
    matrixf_destroy(z);
    */

    float s[16] = {
        1,2,3,4,
        5,5,7,8,
        6,4,8,7,
        1,0,3,1};
    float s_inv[16];
    memmove(s_inv,s,16*sizeof(float));
    matrixf_inv(s_inv,4,4);

    float i4[16];
    matrixf_mul(s,4,4,s_inv,4,4,i4,4,4);

    printf("\ns:\n");
    matrixf_print(s,4,4);
    printf("\ninv(s):\n");
    matrixf_print(s_inv,4,4);
    printf("\ns*inv(s):\n");
    matrixf_print(i4,4,4);

    printf("\n");
    float det = matrixf_det(s,4,4);
    printf("det(s) = %12.8f\n", det);

#if 0
    // pivot test (matrix inversion)
    float t[32] = {
        1,2,3,4,  1,0,0,0,
        5,5,7,8,  0,1,0,0,
        6,4,8,7,  0,0,1,0,
        1,0,3,1,  0,0,0,1};

    unsigned int i;
    for (i=0; i<4; i++) {
        matrixf_pivot(t,4,8,i,i);
        matrixf_print(t,4,8);
    }

    unsigned int j;
    for (i=0; i<4; i++) {
        float v = matrix_access(t,4,8,i,i);
        for (j=0; j<8; j++)
            matrix_access(t,4,8,i,j) /= v;
    }
    matrixf_print(t,4,8);
#endif

    printf("\n");
    printf("testing L/U decomposition [Crout's method]\n");
    float L[16], U[16], P[16];
    matrixf_ludecomp_crout(s,4,4,L,U,P);
    matrixf_print(L,4,4);
    matrixf_print(U,4,4);

    printf("\n");
    printf("testing L/U decomposition [Doolittle's method]\n");
    matrixf_ludecomp_doolittle(s,4,4,L,U,P);
    matrixf_print(L,4,4);
    matrixf_print(U,4,4);

    printf("\n\n");
    float X[16] = {
       0.84382,  -2.38304,   1.43061,  -1.66604,
       3.99475,   0.88066,   4.69373,   0.44563,
       7.28072,  -2.06608,   0.67074,   9.80657,
       6.07741,  -3.93099,   1.22826,  -0.42142};
    float Y[16];
    printf("\nX:\n");
    matrixf_print(X,4,4);

    // swaprows
    memmove(Y,X,16*sizeof(float));
    matrixf_swaprows(Y,4,4,0,2);
    printf("\nmatrixf_swaprows(X,4,4,0,2):\n");
    matrixf_print(Y,4,4);

    // pivot test
    memmove(Y,X,16*sizeof(float));
    matrixf_pivot(Y,4,4,1,2);
    printf("\nmatrixf_pivot(X,4,4,1,2):\n");
    matrixf_print(Y,4,4);

    // inverse test
    memmove(Y,X,16*sizeof(float));
    matrixf_inv(Y,4,4);
    printf("\nmatrixf_inv(X,4,4):\n");
    matrixf_print(Y,4,4);

    // determinant test
    float D = matrixf_det(X,4,4);
    printf("\nmatrixf_det(X,4) = %12.8f\n", D);

    // L/U decomp (Crout's method)
    matrixf_ludecomp_crout(X,4,4,L,U,P);
    printf("\nmatrixf_ludecomp_crout(X,4,4,L,U,P)\n");
    printf("L:\n");
    matrixf_print(L,4,4);
    printf("U:\n");
    matrixf_print(U,4,4);

    // L/U decomp (Doolittle's method)
    matrixf_ludecomp_doolittle(X,4,4,L,U,P);
    printf("\nmatrixf_ludecomp_doolittle(X,4,4,L,U,P)\n");
    printf("L:\n");
    matrixf_print(L,4,4);
    printf("U:\n");
    matrixf_print(U,4,4);

    printf("\n");
    printf("testing Q/R decomposition [Gram-Schmidt]\n");
    float Q[16], R[16];
    matrixf_qrdecomp_gramschmidt(X,4,4,Q,R);
    matrixf_print(Q,4,4);
    matrixf_print(R,4,4);

    /*
    float b[4] = {
       0.91489,
       0.71789,
       1.06553,
      -0.81707};
    */

    float Xb[20] = {
       0.84382,  -2.38304,   1.43061,  -1.66604,   0.91489,
       3.99475,   0.88066,   4.69373,   0.44563,   0.71789,
       7.28072,  -2.06608,   0.67074,   9.80657,   1.06553,
       6.07741,  -3.93099,   1.22826,  -0.42142,  -0.81707};
    printf("\n[X b] =\n");
    matrixf_print(Xb,4,5);

    matrixf_gjelim(Xb,4,5);
    printf("\nmatrixf_gjelim(Xb,4,5)\n");
    matrixf_print(Xb,4,5);

    // compute a*a'
    float a[20] = {
      -0.24655,  -1.78843,   0.39477,   0.43735,  -1.08998,
      -0.42751,   0.62496,   1.43802,   0.19814,   0.78155,
      -0.35658,  -0.81875,  -1.09984,   1.87006,  -0.94191,
       0.39553,  -2.02036,   1.17393,   1.54591,   1.29663};

    printf("\na =\n");
    matrixf_print(a,4,5);

    printf("\n\n");
    printf("computing a*a'\n");
    float aaT[16];
    matrixf_mul_transpose(a,4,5,aaT);
    matrixf_print(aaT,4,4);

    printf("\n\n");
    printf("computing a'*a\n");
    float aTa[25];
    matrixf_transpose_mul(a,4,5,aTa);
    matrixf_print(aTa,5,5);

    printf("\n");
    printf("testing Gram-Schmidt\n");
    float Xgs[12] = {
        1., 2., 1.,
        0., 2., 0.,
        2., 3., 1.,
        1., 1., 0.};
    float Ugs[12];
    float Ugs_test[12] = {
        sqrtf(6.)/6.,   sqrtf(2.)/6.,   2./3.,
        0.,             2.*sqrtf(2.)/3.,-1./3.,
        sqrtf(6.)/3.,   0.,             0.,
        sqrtf(6.)/6.,   -sqrtf(2.)/6.,  -2./3.};
    matrixf_gramschmidt(Xgs,4,3,Ugs);

    printf("X:\n");
    matrixf_print(Xgs,4,3);
    printf("normalized X:\n");
    matrixf_print(Ugs,4,3);
    printf("expected:\n");
    matrixf_print(Ugs_test,4,3);


    // 
    // test Cholesky decomposition
    //

    printf("\n");
    printf("testing Cholesky decomposition\n");
    // generate  input matrix
    float complex Lp[9] = { 1.0,                   0.0,                   0.0,
                           -3.1 + 0.2*_Complex_I,  0.3,                   0.0,
                            1.7 + 0.5*_Complex_I, -0.6 - 0.3*_Complex_I,  2.9};
    float complex Ap[9];
    matrixcf_mul_transpose(Lp, 3, 3, Ap);
    float complex Lc[9];
    matrixcf_chol(Ap, 3, Lc);

    printf("Lp:\n"); matrixcf_print(Lp, 3, 3);
    printf("Ap:\n"); matrixcf_print(Ap, 3, 3);
    printf("Lc:\n"); matrixcf_print(Lc, 3, 3);

    printf("done.\n");
    return 0;
}

