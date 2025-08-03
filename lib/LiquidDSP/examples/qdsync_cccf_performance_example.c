// This example tests the performance for detecting and decoding frames
// with the qdsync_cccf object.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME  "qdsync_cccf_performance_example.m"

int main(int argc, char*argv[])
{
    // options
    unsigned int seq_len      = 1024;   // number of sync symbols
    unsigned int k            =    2;   // samples/symbol
    unsigned int m            =    7;   // filter delay [symbols]
    float        beta         = 0.3f;   // excess bandwidth factor
    int          ftype        = LIQUID_FIRFILT_ARKAISER;
    float        threshold    =   0.10f;//
    unsigned int min_trials   =   10;   //
    unsigned int max_trials   =  100;   //
    unsigned int min_missed   =    4;   //
    float        SNRdB        = -25.0f;

    // generate synchronization sequence (QPSK symbols)
    float complex seq[seq_len];
    unsigned int i;
    for (i=0; i<seq_len ; i++) {
        seq[i] = (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 +
                 (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 * _Complex_I;
    }

    // interpolate sequence
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype,k,m,beta,0);
    unsigned int num_symbols = seq_len + 2*m + 3*seq_len;
    unsigned int buf_len = num_symbols * k;
    float complex buf_0[buf_len];
    float complex buf_1[buf_len];
    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        float complex s = i < seq_len ? seq[i] : 0;

        // interpolate symbol
        firinterp_crcf_execute(interp, s, buf_0 + i*k);
    }
    firinterp_crcf_destroy(interp);

    // create sync object and run signal through
    qdsync_cccf q = qdsync_cccf_create_linear(seq, seq_len, ftype, k, m, beta, NULL, NULL);
    qdsync_cccf_set_threshold(q, threshold);

    while (SNRdB <= 5.0f) {
        float        nstd = powf(10.0f, -SNRdB/20.0f) * M_SQRT2;
        unsigned int t;
        unsigned int num_trials = 0;
        unsigned int num_missed = 0;
        while (1) {
            for (t=0; t<min_trials; t++) {
                // copy buffer and add noise
                for (i=0; i<buf_len; i++)
                    buf_1[i] = buf_0[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

                // run through synchronizer
                qdsync_cccf_reset(q);
                qdsync_cccf_execute(q, buf_1, buf_len);
                num_missed += qdsync_cccf_is_detected(q) ? 0 : 1;
            }
            num_trials += min_trials;
            if (num_missed >= min_missed)
                break;
            if (num_trials >= max_trials)
                break;
        }
        float pmd = (float)num_missed / (float)num_trials;
        printf("SNR: %8.3f dB, missed %3u / %3u (%5.1f%%)\n", SNRdB, num_missed, num_trials, pmd*100);
        if (num_missed < min_missed)
            break;
        SNRdB += 1.0f;
    }

    qdsync_cccf_destroy(q);

#if 0
    // open file for storing results
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");

    // export results
    fprintf(fid,"seq_len = %u;\n", seq_len);
    fprintf(fid,"figure('color','white','position',[100 100 800 800]);\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"legend('Preamble','Payload');\n");
    fprintf(fid,"grid on; xlabel('real'); ylabel('imag');\n");
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);
#endif
    return 0;
}

