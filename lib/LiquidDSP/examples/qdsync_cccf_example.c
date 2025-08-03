// This example demonstrates the functionality of the qdsync object to
// detect and synchronize an arbitrary signal in time in the presence of noise,
// carrier frequency/phase offsets, and fractional-sample timing offsets.
// offsets.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include "liquid.h"

#define OUTPUT_FILENAME "qdsync_cccf_example.m"

// synchronization callback, return 0:continue, 1:reset
int callback(float complex * _buf,
             unsigned int    _buf_len,
             void *          _context)
{
    printf("callback got %u samples\n", _buf_len);
    unsigned int i;
    for (i=0; i<_buf_len; i++)
        fprintf((FILE*)_context, "y(end+1) = %12.8f + %12.8fj;\n", crealf(_buf[i]), cimagf(_buf[i]));
    return 0;
}

int main(int argc, char*argv[])
{
    // options
    unsigned int seq_len      =   80;   // number of sync symbols
    unsigned int k            =    2;   // samples/symbol
    unsigned int m            =    7;   // filter delay [symbols]
    float        beta         = 0.3f;   // excess bandwidth factor
    int          ftype        = LIQUID_FIRFILT_ARKAISER;
    float        nstd         = 0.01f;

    // generate synchronization sequence (QPSK symbols)
    float complex seq[seq_len];
    unsigned int i;
    for (i=0; i<seq_len ; i++) {
        seq[i] = (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 +
                 (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 * _Complex_I;
    }

    // open file for storing results
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all; y=[];\n");

    // create sync object
    qdsync_cccf q = qdsync_cccf_create_linear(seq, seq_len, ftype, k, m, beta, callback, fid);
    qdsync_cccf_print(q);

    // create interpolator
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype,k,m,beta,0);

    // run signal through sync object
    float complex buf[k];
    for (i=0; i<10*seq_len; i++) {
        // generate random symbol
        float complex s = i < seq_len ? seq[i] : (rand() & 1 ? 1.0f : -1.0f);

        // interpolate symbol
        firinterp_crcf_execute(interp, s, buf);

        // add noise
        unsigned int j;
        for (j=0; j<k; j++)
            buf[j] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // run through synchronizer
        qdsync_cccf_execute(q, buf, k);
    }
    qdsync_cccf_destroy(q);
    firinterp_crcf_destroy(interp);

    // export results
    fprintf(fid,"seq_len = %u;\n", seq_len);
    fprintf(fid,"figure('color','white','position',[100 100 800 800]);\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  plot(real(y(1:seq_len)),      imag(y(1:seq_len)),      '.','MarkerSize',6,'Color',[0 0.2 0.5]);\n");
    fprintf(fid,"  plot(real(y((seq_len+1):end)),imag(y((seq_len+1):end)),'.','MarkerSize',6,'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"legend('Preamble','Payload');\n");
    fprintf(fid,"grid on; xlabel('real'); ylabel('imag');\n");
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);
    return 0;
}
