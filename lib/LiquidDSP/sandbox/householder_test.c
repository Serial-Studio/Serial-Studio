// 
// householder_test.c : test householder reflections
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.h"

#define DEBUG 1

int main() {
    unsigned int i;
    unsigned int j;
    unsigned int k;

#if 0
    // problem definition
    float A[40] = {
        22,  10,   2,   3,  7,
        14,   7,  10,   0,  8,
        -1,  13,  -1, -11,  3,
        -3,  -2,  13,  -2,  4,
         9,   8,   1,  -2,  4,
         9,   1,  -7,   5, -1,
         2,  -6,   6,   5,  1,
         4,   5,   0,  -2,  2};

    unsigned int m = 8;
    unsigned int n = 5;
#else
    unsigned int m=8;
    unsigned int n=5;
    float A[m*n];

    // make random matrix
    unsigned int t;
    for (t=0; t<m*n; t++)
        A[t] = randnf();

    // Hilbert matrix
    for (i=0; i<m; i++) {
        for (j=0; j<n; j++)
            matrix_access(A,m,n,i,j) = 1.0 / ((float)(i + j + 1));
    }
#endif

    matrixf_print(A,m,n);

#if 0
    // compute Householder reflections
    // arrays
    float Ah[m*n];
    float P[m*n];
    float Q[m*n];
    matrixf_householder(A,m,n,P,Ah,Q);
#endif

    // internal data structures
    float A0[m*n];  // A^(k)
    float A1[m*n];  // A^(k+1/2)
    float A2[m*n];  // A^(k+1  )
    float xi[m];
    float alphak;
    float sk;
    float yj[n];
    float betak;
    float tk;

    // initialize...
    memmove(A0,A,m*n*sizeof(float));

    // 
    for (k=0; k<n; k++) {
#if DEBUG
        printf("%u------------------------------\n", k);
#endif
        // compute sk, alphak
        sk = 0.0;
        for (i=k; i<m; i++) {
            float a0_ik = matrix_access(A0,m,n,i,k);
            sk += creal(a0_ik*conj(a0_ik));
        }
        sk = sqrt(sk);
        float a0_kk = matrix_access(A0,m,n,k,k);
        alphak = -sk * a0_kk / fabs(a0_kk); // TODO : use copysign
#if DEBUG
        printf("  sk        :   %12.8f\n", sk);
        printf("  alphak    :   %12.8f\n", alphak);
#endif

        if (sk == 0) {
            for (i=0; i<k; i++) xi[i] = 0.0;
        } else {
            // compute x_i^(k) for i<k
            for (i=0; i<k; i++)
                xi[i] = 0.0;

            // compute x_i^(k) for i=k
            float xk = sqrt( 0.5*(1 + fabs(a0_kk)/sk) );
            xi[k] = xk;

            // compute x_i^(k) for i>k
            float ck = 1.0 / (2*sk*a0_kk/fabs(a0_kk)*xk); // TODO : use copysign
            for (i=k+1; i<m; i++) {
                float a0_ik = matrix_access(A0,m,n,i,k);
                xi[i] = ck*a0_ik;
            }
        }

        // compute A^(k+1/2) = A^(k) - x^(k)*2[ x^(k)^T *A^(k) ]
        float xixiT[m*m];  // xi*xi^T
        matrixf_mul_transpose(xi,m,1,xixiT);
        float S[m*n];
        matrixf_mul(xixiT, m, m,
                    A0,    m, n,
                    S,     m, n);
        // 
        for (i=0; i<m*n; i++)
            A1[i] = A0[i] - 2.0*S[i];
#if DEBUG
        printf("xi:\n");    matrixf_print(xi,   m,1);
        //printf("xi*xi^T:\n");matrixf_print(xixiT,m,m);
        //printf("S:\n");     matrixf_print(S,    m,n);
        printf("A(k+1/2):\n");    matrixf_print(A1,   m,n);
#endif

        // compute tk, betak
        tk = 0.0;
        for (j=k+1; j<n; j++) {
            float a_kj = matrix_access(A1,m,n,k,j);
            tk += creal(a_kj*conj(a_kj));
        }
        tk = sqrt(tk);
        // TODO : check dimension size... [n > m]
        float a1_kk1 = matrix_access(A1,m,n,k,k+1);
        betak = -tk * a1_kk1 / fabs(a1_kk1);  // TODO : use copysign
#if DEBUG
        printf("  tk        :   %12.8f\n", tk);
        printf("  betak     :   %12.8f\n", betak);
        printf("  A1[k,k+1] :   %12.8f\n", a1_kk1);
#endif

        // TODO : check dimension size
        if ( (k+1)==n ) {
            fprintf(stderr,"warning: maximum dimension exceeded\n");

            // copy and exit
            memmove(A0, A1, m*n*sizeof(float));
            break;
        }

        if (tk == 0) {
            for (j=0; j<k; j++) yj[j] = 0.0;
        } else {
            // compute y_j(k) for j <= k
            for (j=0; j<=k; j++)
                yj[j] = 0.0;

            // compute y_j(k) for j = k+1
            // TODO : check dimension size... [n > m]
            float yk1 = (tk == 0) ? 0.0 : sqrt( 0.5*(1 + fabs(a1_kk1)/tk) );
            yj[k+1] = yk1;

            // compute y_j(k) for j > k+1
            float dk = 1.0 / (2*tk*a1_kk1/fabs(a1_kk1)*yk1);
#if DEBUG
            printf("dk : %12.8f\n", dk);
#endif
            for (j=k+2; j<n; j++) {
                float a1_kj = matrix_access(A1,m,n,k,j);
                yj[j] = dk*a1_kj;
            }
        }

        // compute A(k+1) = A(k+1/2) - 2*[A(k+1/2)*y(k)]*y(k)^T
        float yjyjT[n*n];
        matrixf_mul_transpose(yj,n,1,yjyjT);
        float T[m*n];
        matrixf_mul(A1,    m, n,
                    yjyjT, n, n,
                    T,     m, n);
        //
        for (j=0; j<m*n; j++)
            A2[j] = A1[j] - 2*T[j];
#if DEBUG
        printf("yj:\n");        matrixf_print(yj,   n,1);
        printf("yi*yi^T:\n");   matrixf_print(yjyjT,n,n);
        printf("T:\n");         matrixf_print(T,    m,n);
        printf("A(k+1):\n");    matrixf_print(A2,   m,n);
#endif

        // copy...
        memmove(A0, A2, m*n*sizeof(float));

        // exit prematurely
        //break;
        //exit(1);
    }

    printf("--------------\n\n");
    matrixf_print(A0, m, n);


    printf("done.\n");
    return 0;
}

