//
// fec_ldpc_test.c
//
// Test soft demodulation of LDPC code
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEBUG_LDPC_TEST (0)

#if DEBUG_LDPC_TEST
#include <liquid.h>
#endif

//
float phi(float _x) { return -logf(tanhf(_x/2.0f + 1e-12)); }

int main(int argc, char*argv[])
{
    // parity check matrix
    unsigned char H[32] = {
        1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 0, 0,
        1, 0, 0, 1, 0, 0, 1, 0,
        0, 1, 0, 0, 1, 0, 0, 1};

    // transmitted codeword
    unsigned char c[8] = {
        1, 0, 1, 0, 1, 1, 1, 1};

    // received message
    float y[8] = {
        0.2, 0.2, -0.9, 0.6, 0.5, -1.1, -0.4, -1.2};

    // noise standard deviation
    float sigma = sqrtf(0.5f);

    unsigned int max_iterations = 10;
    unsigned int n = 8;
    unsigned int m = 4;

    // internal variables
    unsigned int num_iterations = 0;
    float Lq[m*n];
    float Lr[m*n];
    float Lc[n];
    float LQ[n];
    unsigned char c_hat[n];
    unsigned char parity[m];
    unsigned int i;
    unsigned int j;
    unsigned int ip;
    unsigned int jp;
    float alpha_prod;
    float phi_sum;
    int parity_pass;
    int continue_running = 1;

    // initialize Lq with log-likelihood values
    for (i=0; i<n; i++)
        Lc[i] = 2.0f * y[i] / (sigma*sigma);

    for (j=0; j<m; j++) {
        for (i=0; i<n; i++) {
            Lq[j*n+i] = H[j*n+i] ? Lc[i] : 0.0f;
        }
    }
#if DEBUG_LDPC_TEST
    // print Lc
    matrixf_print(Lc,1,n);
#endif

    // TODO : run multiple iterations
    while (continue_running) {
#if DEBUG_LDPC_TEST
        //
        printf("\n");
        printf("************* iteration %u ****************\n", num_iterations);
#endif

        // compute Lr
        for (i=0; i<n; i++) {
            for (j=0; j<m; j++) {
                alpha_prod = 1.0f;
                phi_sum    = 0.0f;
                for (ip=0; ip<n; ip++) {
                    if (H[j*n+ip]==1 && i != ip) {
                        float alpha = Lq[j*n+ip] > 0.0f ? 1.0f : -1.0f;
                        float beta  = fabsf(Lq[j*n+ip]);
                        phi_sum += phi(beta);
                        alpha_prod *= alpha;
                    }
                }
                Lr[j*n+i] = alpha_prod * phi(phi_sum);
            }
        }

#if DEBUG_LDPC_TEST
        // print Lq
        matrixf_print(Lq,m,n);

        // print Lr
        matrixf_print(Lr,m,n);
#endif

        // compute next iteration of Lq
        for (i=0; i<n; i++) {
            for (j=0; j<m; j++) {
                // initialize with LLR
                Lq[j*n+i] = Lc[i];

                for (jp=0; jp<m; jp++) {
                    if (H[jp*n+i]==1 && j != jp)
                        Lq[j*n+i] += Lr[jp*n+i];
                }
            }
        }

#if DEBUG_LDPC_TEST
        // print Lq
        matrixf_print(Lq,m,n);
#endif

        // compute LQ
        for (i=0; i<n; i++) {
            LQ[i] = Lc[i];  // initialize with LLR value

            for (j=0; j<m; j++) {
                if (H[j*n+i]==1)
                    LQ[i] += Lr[j*n+i];
            }
        }

#if DEBUG_LDPC_TEST
        // print LQ
        matrixf_print(LQ,1,n);
#endif

        // compute hard-decoded value
        for (i=0; i<n; i++)
            c_hat[i] = LQ[i] < 0.0f ? 1 : 0;

        // compute parity check: p = H*c_hat
        for (j=0; j<m; j++) {
            parity[j] = 0;

            // 
            for (i=0; i<n; i++)
                parity[j] += H[j*n+i] * c_hat[i];

            // math is modulo 2
            parity[j] %= 2;
        }

        // check parity
        parity_pass = 1;
        for (j=0; j<m; j++) {
            if (parity[j]) parity_pass = 0;
        }

        // print hard-decision output
        printf("%3u : c hat = [", num_iterations);
        for (i=0; i<n; i++)
            printf(" %1u", c_hat[i]);
        printf(" ],  ");

        // print parity
        printf("parity = [");
        for (j=0; j<m; j++)
            printf(" %1u", parity[j]);
        printf(" ],  ");

        printf(" (%s)\n", parity_pass ? "pass" : "FAIL");

        // update...
        num_iterations++;
        if (parity_pass || num_iterations == max_iterations)
            continue_running = 0;
    }

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

    printf("num errors: %u / %u\n", num_errors, n);

    return 0;
}

