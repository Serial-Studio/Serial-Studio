// Demonstration of rate matching with the arbitrary resampler
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liquid.h"

int main(int argc, char*argv[])
{
    // options
    float        r          = 1.618034; // true resampling rate (output/input) offset
    unsigned int num_blocks = 400;      // number of blocks to simulate
    unsigned int block_len  = 256;      // number of samples in block
    float        alpha      = 0.1f;     // loop bandwidth factor
    const char   filename[] = "resamp_crcf_rate_match_example.m";   // output filename

    // buffers
    float complex buf_0[2*block_len];   // original
    float complex buf_1[2*block_len];   // resampled
    float complex buf_2[2*block_len];   // recovered
    memset(buf_0, 0x00, block_len*sizeof(float complex));

    // create resamplers
    resamp_crcf resamp_0 = resamp_crcf_create_default(r);
    resamp_crcf resamp_1 = resamp_crcf_create_default(1);

    // run loop and print results
    unsigned int i;
    float log_error = 0.0f; // accumulated log error
    float rate      = 1.0f;
    char  buf_str[200];
    FILE * fid = fopen(filename,"w");
    fprintf(fid,"clear all; close all;\nv=[\n");
    printf("# %8s %8s %8s %12s %12s %12s\n", "blk len", "num in", "num out", "rate in", "rate out", "rate all");
    for (i=0; i<num_blocks; i++) {
        // initial offset
        unsigned int n1=0;
        resamp_crcf_execute_block(resamp_0, buf_0, block_len, buf_1, &n1);

        // compensated
        unsigned int n2=0;
        resamp_crcf_execute_block(resamp_1, buf_1, n1, buf_2, &n2);

        // adjust resampling rate
        log_error = (1-alpha)*log_error + alpha*logf((float)block_len/(float)n2);
        resamp_crcf_adjust_rate(resamp_1, expf(alpha*log_error));
        rate = resamp_crcf_get_rate(resamp_1);

        // print results
        sprintf(buf_str,"  %8u %8u %8u %12.9f %12.9f %12.9f\n", block_len, n1, n2, r, rate, r*rate);
        fprintf(fid,"%s",buf_str);
        printf("%s",buf_str);
    }
    printf("# true rate: %12.9f, recovered: %12.9f\n", r, r*rate);
    fprintf(fid,"];\nfigure; plot(v(:,6),'LineWidth',2); grid on;\n");
    fprintf(fid,"xlabel('block index'); ylabel('resampling error');\n");

    // clean up allocated objects
    resamp_crcf_destroy(resamp_0);
    resamp_crcf_destroy(resamp_1);
    return 0;
}
