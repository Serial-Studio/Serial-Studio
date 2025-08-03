//
// fecsoft_ldpc_test.c
//
// Test soft decoding using the sum-product
// algorithm (see sandbox/ldpc_test.c)
//
// The generator and parity check matrices can be generated from
// the 16 x 16 matrix P. Each row of P is a circular shift of the
// generator polynomial p = [ 1 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0]
// such that P^T = P and
//  G = [I(16) P    ]^T
//  H = [P^T   I(16)]

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "liquid.internal.h"

#define OUTPUT_FILENAME "fecsoft_ldpc_test.m"

int main(int argc, char*argv[])
{
    // initialize random seed
    srand(time(NULL));

    float SNRdB_min = -2.0f;
    float SNRdB_max =  6.0f;
    unsigned int num_steps = 19;
    unsigned int num_trials = 2000;
    unsigned int max_iterations = 5;

    unsigned int m = 16;    // rows in H
    unsigned int n = 32;    // cols in H

    // initial generator polynomial
    // TODO : find 'optimal' generator polynomial
    unsigned char p[] = {1,0,0,0, 0,1,0,0, 0,0,0,1, 0,0,0,0};

    // generator matrix [n x m]
    unsigned char Gs[n*m];

    // parity check matrix [m x n]
    unsigned char Hs[m*n];

    unsigned int i;
    unsigned int j;

    // init generator and parity check matrices
    for (i=0; i<m; i++) {
        for (j=0; j<m; j++) {
            // G = [I(m) P]^T
            Gs[j*m + i]       = (i==j) ? 1 : 0;
            Gs[j*m + i + m*m] = p[(i+j)%m];

            // H = [P^T I(m)]
            Hs[i*n + j + m] = (i==j) ? 1 : 0;
            Hs[i*n + j]     = p[(i+j)%m];
        }
    }

    // generate sparse binary matrices
    smatrixb G = smatrixb_create_array(Gs, n, m);
    smatrixb H = smatrixb_create_array(Hs, m, n);
    printf("G =\n"); smatrixb_print_expanded(G);
    printf("H =\n"); smatrixb_print_expanded(H);

    // derived values
    float SNRdB_step = (SNRdB_max - SNRdB_min) / (num_steps-1);

    // arrays
    unsigned char x[m];     // original message signal
    unsigned char c[n];     // transmitted codeword
    float complex y[n];     // received message with noise
    float LLR[n];           // log-likelihood ratio
    unsigned char c_hat[n]; // estimated codeword
    unsigned char x_hat[n]; // estimated message signal

    unsigned int t;
    unsigned int s;

    // data arrays
    unsigned int num_bit_errors[num_steps];
    unsigned int num_sym_errors[num_steps];

    printf("  %8s [%8s] %8s %12s %12s\n", "SNR [dB]", "trials", "# errs", "(BER)", "uncoded");
    for (s=0; s<num_steps; s++) {
        // compute SNR
        float SNRdB = SNRdB_min + s*SNRdB_step;

        // noise standard deviation
        float sigma = powf(10.0f, -SNRdB/20.0f);

        // reset error counter
        num_bit_errors[s] = 0;
        num_sym_errors[s] = 0;

        for (t=0; t<num_trials; t++) {
            // generate original message signal
            for (i=0; i<m; i++)
                x[i] = rand() % 2;

            smatrixb_vmul(G, x, c);
            //printf("x = ["); for (i=0; i<m; i++) printf(" %c", x[i] ? '1' : '0'); printf(" ]\n");
            //printf("c = ["); for (i=0; i<n; i++) printf(" %c", c[i] ? '1' : '0'); printf(" ]\n");

            // compute received signal (with noise) and log-likelihood ratio
            for (i=0; i<n; i++) {
                y[i] = c[i] ? -1.0f : 1.0f;
                y[i] += sigma*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
                LLR[i] = 1.0f* crealf(y[i]) / (sigma*sigma);
            }

            // run internal sum-product algorithm
            int parity_pass = fec_sumproduct(m, n, H, LLR, c_hat, max_iterations);

            // compute estimated transmitted message (first 12 bits of c_hat)
            for (i=0; i<m; i++)
                x_hat[i] = c_hat[i];

            // compute errors in decoded message signal
            for (i=0; i<m; i++)
                num_bit_errors[s] += (x_hat[i] == x[i]) ? 0 : 1;

            // compute number of symbol errors
            num_sym_errors[s] += parity_pass ? 0 : 1;
        }

        // print results for this SNR step
        printf("  %8.3f [%8u] %8u %12.4e %12.4e\n",
                SNRdB,
                m*num_trials,
                num_bit_errors[s], (float)(num_bit_errors[s]) / (float)(num_trials*m),
                0.5f*erfcf(1.0f/sigma));
    }

    // destroy sparse matrices
    smatrixb_destroy(H);
    smatrixb_destroy(G);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"\n\n");
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"n = %u;\n", n);
    fprintf(fid,"r = m / n;\n");
    fprintf(fid,"num_steps = %u;\n", num_steps);
    fprintf(fid,"num_trials = %u;\n", num_trials);
    fprintf(fid,"num_bit_trials = num_trials*m;\n");
    for (i=0; i<num_steps; i++) {
        fprintf(fid,"SNRdB(%4u) = %12.8f;\n",i+1, SNRdB_min + i*SNRdB_step);
        fprintf(fid,"num_bit_errors(%6u) = %u;\n", i+1, num_bit_errors[i]);
        fprintf(fid,"num_sym_errors(%6u) = %u;\n", i+1, num_sym_errors[i]);
    }
    fprintf(fid,"EbN0dB = SNRdB - 10*log10(r);\n");
    fprintf(fid,"EbN0dB_bpsk = -15:0.5:40;\n");
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(EbN0dB_bpsk, 0.5*erfc(sqrt(10.^[EbN0dB_bpsk/10]))+1e-12,'-x',\n");
    fprintf(fid,"         EbN0dB,      num_bit_errors / num_bit_trials + 1e-12,  '-x');\n");
    fprintf(fid,"axis([%f (%f-10*log10(r)) 1e-6 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"legend('uncoded','coded',1);\n");
    fprintf(fid,"xlabel('E_b/N_0 [dB]');\n");
    fprintf(fid,"ylabel('Bit Error Rate');\n");
    fprintf(fid,"title('BER vs. E_b/N_0 for (24,12) code');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

