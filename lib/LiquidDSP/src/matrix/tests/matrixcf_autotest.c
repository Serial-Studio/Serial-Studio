/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>

#include "autotest/autotest.h"
#include "liquid.h"

// autotest data definitions
#include "src/matrix/tests/matrix_data.h"

// test matrix addition
void autotest_matrixcf_add()
{
    float tol = 1e-6f;

    // x [size: 5 x 4]
    // y [size: 5 x 4]
    // z [size: 5 x 4]
    float complex z[20];
    matrixcf_add(matrixcf_data_add_x,
                 matrixcf_data_add_y,
                 z,
                 5, 4);

    unsigned int i;
    for (i=0; i<20; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_add_z[i]), crealf(z[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_add_z[i]), cimagf(z[i]), tol );
    }
}

// test matrix augmentation
void autotest_matrixcf_aug()
{
    float tol = 1e-6f;

    // x [size: 5 x 4]
    // y [size: 5 x 3]
    // z [size: 5 x 7]
    float complex z[35];
    matrixcf_aug(matrixcf_data_aug_x, 5, 4,
                 matrixcf_data_aug_y, 5, 3,
                 z,                  5, 7);
    
    // print result
    if (liquid_autotest_verbose) {
        printf("augment:\n");
        printf("  x: ");        matrixcf_print(matrixcf_data_aug_x, 5, 4);
        printf("  y: ");        matrixcf_print(matrixcf_data_aug_y, 5, 3);
        printf("  expected: "); matrixcf_print(matrixcf_data_aug_z, 5, 7);
        printf("  z: ");        matrixcf_print(z,5,3);
    }

    unsigned int i;
    for (i=0; i<35; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_aug_z[i]), crealf(z[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_aug_z[i]), cimagf(z[i]), tol );
    }
}

// Cholesky decomposition
void autotest_matrixcf_chol()
{
    float tol = 1e-3f;  // error tolerance

    // A [size: 4 x 4]
    // L [size: 4 x 4]
    float complex L[16];

    // run decomposition
    matrixcf_chol(matrixcf_data_chol_A, 4, L);

    if (liquid_autotest_verbose) {
        printf("chol:\n");
        printf("  A: ");        matrixcf_print(matrixcf_data_chol_A, 4, 4);
        printf("  expected: "); matrixcf_print(matrixcf_data_chol_L, 4, 4);
        printf("  L: ");        matrixcf_print(L,                    4, 4);
    }

    unsigned int i;
    for (i=0; i<16; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_chol_L[i]), crealf(L[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_chol_L[i]), cimagf(L[i]), tol );
    }
}

// matrix inversion
void autotest_matrixcf_inv()
{
    float tol = 1e-6f;  // error tolerance

    // x [size: 5 x 5]
    // y [size: 5 x 5]
    float complex y[25];
    memmove(y, matrixcf_data_inv_x, 5*5*sizeof(float complex));
    matrixcf_inv(y, 5, 5);

    if (liquid_autotest_verbose) {
        printf("inv:\n");
        printf("  x: ");        matrixcf_print(matrixcf_data_inv_x, 5, 5);
        printf("  expected: "); matrixcf_print(matrixcf_data_inv_y, 5, 5);
        printf("  y: ");        matrixcf_print(y,                   5, 5);
    }

    unsigned int i;
    for (i=0; i<25; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_inv_y[i]), crealf(y[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_inv_y[i]), cimagf(y[i]), tol );
    }
}

// linsolve (solve linear system of equations)
void autotest_matrixcf_linsolve()
{
    float tol = 1e-6f;  // error tolerance

    // A [size: 5 x 5]
    // x [size: 5 x 1]
    // b [size: 5 x 1]
    float complex x[5];
    
    // run solver
    matrixcf_linsolve(matrixcf_data_linsolve_A, 5,
                      matrixcf_data_linsolve_b,
                      x, NULL);

    if (liquid_autotest_verbose) {
        printf("linsolve:\n");
        printf("  A: ");        matrixcf_print(matrixcf_data_linsolve_A, 5, 5);
        printf("  b: ");        matrixcf_print(matrixcf_data_linsolve_b, 5, 1);
        printf("  expected: "); matrixcf_print(matrixcf_data_linsolve_x, 5, 1);
        printf("  x: ");        matrixcf_print(x,                        5, 1);
    }

    unsigned int i;
    for (i=0; i<5; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_linsolve_x[i]), crealf(x[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_linsolve_x[i]), cimagf(x[i]), tol );
    }
}

// L/U decomp (Crout)
void autotest_matrixcf_ludecomp_crout()
{
    float tol = 1e-5f;  // error tolerance

    float complex L[64];
    float complex U[64];
    float complex P[64];

    float complex LU_test[64];

    // run decomposition
    matrixcf_ludecomp_crout(matrixcf_data_ludecomp_A, 8, 8, L, U, P);

    // multiply LU
    matrixcf_mul(L,       8, 8,
                 U,       8, 8,
                 LU_test, 8, 8);

    if (liquid_autotest_verbose) {
        printf("ludecomp_crout:\n");
        printf("  A: ");  matrixcf_print(matrixcf_data_ludecomp_A, 8, 8);
        printf("  L: ");  matrixcf_print(L,                        8, 8);
        printf("  U: ");  matrixcf_print(U,                        8, 8);
        printf("  LU: "); matrixcf_print(LU_test,                  8, 8);
    }

    unsigned int r,c;
    for (r=0; r<8; r++) {
        for (c=0; c<8; c++) {
            if (r < c) {
                CONTEND_DELTA( crealf(matrix_access(L,8,8,r,c)), 0.0f, tol );
                CONTEND_DELTA( cimagf(matrix_access(L,8,8,r,c)), 0.0f, tol );
            } else if (r==c) {
                CONTEND_DELTA( crealf(matrix_access(U,8,8,r,c)), 1.0f, tol );
                CONTEND_DELTA( cimagf(matrix_access(U,8,8,r,c)), 0.0f, tol );
            } else {
                CONTEND_DELTA( crealf(matrix_access(U,8,8,r,c)), 0.0f, tol );
                CONTEND_DELTA( cimagf(matrix_access(U,8,8,r,c)), 0.0f, tol );
            }
        }
    }

    unsigned int i;
    for (i=0; i<64; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_ludecomp_A[i]), crealf(LU_test[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_ludecomp_A[i]), cimagf(LU_test[i]), tol );
    }
}

// L/U decomp (Doolittle)
void autotest_matrixcf_ludecomp_doolittle()
{
    float tol = 1e-5f;  // error tolerance

    float complex L[64];
    float complex U[64];
    float complex P[64];

    float complex LU_test[64];

    // run decomposition
    matrixcf_ludecomp_doolittle(matrixcf_data_ludecomp_A, 8, 8, L, U, P);

    // multiply LU
    matrixcf_mul(L,       8, 8,
                 U,       8, 8,
                 LU_test, 8, 8);

    if (liquid_autotest_verbose) {
        printf("ludecomp_doolittle:\n");
        printf("  A: ");  matrixcf_print(matrixcf_data_ludecomp_A, 8, 8);
        printf("  L: ");  matrixcf_print(L,                        8, 8);
        printf("  U: ");  matrixcf_print(U,                        8, 8);
        printf("  LU: "); matrixcf_print(LU_test,                  8, 8);
    }

    unsigned int r,c;
    for (r=0; r<8; r++) {
        for (c=0; c<8; c++) {
            if (r < c) {
                CONTEND_DELTA( crealf(matrix_access(L,8,8,r,c)), 0.0f, tol );
                CONTEND_DELTA( cimagf(matrix_access(L,8,8,r,c)), 0.0f, tol );
            } else if (r==c) {
                CONTEND_DELTA( crealf(matrix_access(L,8,8,r,c)), 1.0f, tol );
                CONTEND_DELTA( cimagf(matrix_access(L,8,8,r,c)), 0.0f, tol );
            } else {
                CONTEND_DELTA( crealf(matrix_access(U,8,8,r,c)), 0.0f, tol );
                CONTEND_DELTA( cimagf(matrix_access(U,8,8,r,c)), 0.0f, tol );
            }
        }
    }

    unsigned int i;
    for (i=0; i<64; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_ludecomp_A[i]), crealf(LU_test[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_ludecomp_A[i]), cimagf(LU_test[i]), tol );
    }
}

// test matrix multiplication
void autotest_matrixcf_mul()
{
    float tol = 1e-6f;

    // x [size: 5 x 4]
    // y [size: 4 x 3]
    // z [size: 5 x 3]
    float complex z[35];
    matrixcf_mul(matrixcf_data_mul_x, 5, 4,
                 matrixcf_data_mul_y, 4, 3,
                 z,                   5, 3);

    // print result
    if (liquid_autotest_verbose) {
        printf("multiplication:\n");
        printf("  x: ");        matrixcf_print(matrixcf_data_mul_x,5,4);
        printf("  y: ");        matrixcf_print(matrixcf_data_mul_y,4,3);
        printf("  expected: "); matrixcf_print(matrixcf_data_mul_z,5,3);
        printf("  z: ");        matrixcf_print(z,5,3);
    }

    unsigned int i;
    for (i=0; i<15; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_mul_z[i]), crealf(z[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_mul_z[i]), cimagf(z[i]), tol );
    }
}

// Q/R decomp (Gram-Schmidt)
void autotest_matrixcf_qrdecomp()
{
    float tol = 1e-4f;  // error tolerance

    float complex Q[16];
    float complex R[16];

    float complex QR_test[16];  // Q*R
    float complex QQT_test[16]; // Q*Q^T

    // run decomposition
    matrixcf_qrdecomp_gramschmidt(matrixcf_data_qrdecomp_A, 4, 4, Q, R);

    // compute Q*R
    matrixcf_mul(Q,       4, 4,
                 R,       4, 4,
                 QR_test, 4, 4);

    // compute Q*Q^T
    matrixcf_mul_transpose(Q, 4, 4, QQT_test);

    if (liquid_autotest_verbose) {
        printf("qrdecomp_gramschmidt:\n");
        printf("  A: ");          matrixcf_print(matrixcf_data_qrdecomp_A, 4, 4);
        printf("  Q: ");          matrixcf_print(Q,                        4, 4);
        printf("  R: ");          matrixcf_print(R,                        4, 4);
        printf("  Q expected: "); matrixcf_print(matrixcf_data_qrdecomp_Q, 4, 4);
        printf("  R expected: "); matrixcf_print(matrixcf_data_qrdecomp_R, 4, 4);
        printf("  QR: ");         matrixcf_print(QR_test,                  4, 4);
        printf("  QQ: ");         matrixcf_print(QQT_test,                 4, 4);
    }

    unsigned int i;

    // ensure Q*R = A
    for (i=0; i<16; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_qrdecomp_A[i]), crealf(QR_test[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_qrdecomp_A[i]), cimagf(QR_test[i]), tol );
    }

    // ensure Q*Q^T = I(4)
    float complex I4[16];
    matrixcf_eye(I4,4);
    for (i=0; i<16; i++)
        CONTEND_DELTA( QQT_test[i], I4[i], tol );

    // ensure Q and R are correct
    for (i=0; i<16; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_qrdecomp_Q[i]), crealf(Q[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_qrdecomp_Q[i]), cimagf(Q[i]), tol );

        CONTEND_DELTA( crealf(matrixcf_data_qrdecomp_R[i]), crealf(R[i]), tol );
        CONTEND_DELTA( cimagf(matrixcf_data_qrdecomp_R[i]), cimagf(R[i]), tol );
    }
}

// transpose/multiply
void autotest_matrixcf_transmul()
{
    float tol = 1e-4f;  // error tolerance

    float complex xxT[25];  // [size: 5 x 5]
    float complex xxH[25];  // [size: 5 x 5]
    float complex xTx[16];  // [size: 4 x 4]
    float complex xHx[16];  // [size: 4 x 4]

    // run matrix multiplications
    matrixcf_mul_transpose(matrixcf_data_transmul_x, 5, 4, xxT);
    matrixcf_mul_hermitian(matrixcf_data_transmul_x, 5, 4, xxH);
    matrixcf_transpose_mul(matrixcf_data_transmul_x, 5, 4, xTx);
    matrixcf_hermitian_mul(matrixcf_data_transmul_x, 5, 4, xHx);

    if (liquid_autotest_verbose) {
        printf("transmul:\n");
        printf("  x: ");            matrixcf_print(matrixcf_data_transmul_x,  5,4);
        printf("\n");
        printf("  xxT: ");          matrixcf_print(xxT,                      5,5);
        printf("  xxT expected: "); matrixcf_print(matrixcf_data_transmul_xxT,5,5);
        printf("\n");
        printf("  xxH: ");          matrixcf_print(xxH,                      5,5);
        printf("  xxH expected: "); matrixcf_print(matrixcf_data_transmul_xxH,5,5);
        printf("\n");
        printf("  xTx: ");          matrixcf_print(xTx,                      4,4);
        printf("  xTx expected: "); matrixcf_print(matrixcf_data_transmul_xTx,4,4);
        printf("\n");
        printf("  xHx: ");          matrixcf_print(xHx,                      4,4);
        printf("  xHx expected: "); matrixcf_print(matrixcf_data_transmul_xHx,4,4);
        printf("\n");
    }

    // run tests
    unsigned int i;

    for (i=0; i<25; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_transmul_xxT[i]), crealf(xxT[i]), tol);
        CONTEND_DELTA( cimagf(matrixcf_data_transmul_xxT[i]), cimagf(xxT[i]), tol);
    }

    for (i=0; i<25; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_transmul_xxH[i]), crealf(xxH[i]), tol);
        CONTEND_DELTA( cimagf(matrixcf_data_transmul_xxH[i]), cimagf(xxH[i]), tol);
    }

    for (i=0; i<16; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_transmul_xTx[i]), crealf(xTx[i]), tol);
        CONTEND_DELTA( cimagf(matrixcf_data_transmul_xTx[i]), cimagf(xTx[i]), tol);
    }

    for (i=0; i<16; i++) {
        CONTEND_DELTA( crealf(matrixcf_data_transmul_xHx[i]), crealf(xHx[i]), tol);
        CONTEND_DELTA( cimagf(matrixcf_data_transmul_xHx[i]), cimagf(xHx[i]), tol);
    }
}



