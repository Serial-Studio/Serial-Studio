// Demonstrate partitioning rresamp work in separate blocks
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME "rresamp_crcf_partition_example.m"

int main(int argc, char*argv[])
{
    // options
    unsigned int    P   =  4;       // output rate (interpolation factor)
    unsigned int    Q   =  5;       // input rate (decimation factor)
    unsigned int    m   =  8;       // resampling filter semi-length (filter delay)
    float           bw  = 0.5f;     // resampling filter bandwidth
    float           As  = 60.0f;    // resampling filter stop-band attenuation [dB]
    unsigned int    n   = 10;       // block size

    // create two identical resampler objects
    rresamp_crcf q0 = rresamp_crcf_create_kaiser(P,Q,m,bw,As);
    rresamp_crcf q1 = rresamp_crcf_create_kaiser(P,Q,m,bw,As);

    // full input, output buffers
    float complex buf_in   [2*Q*n]; // input buffer
    float complex buf_out_0[2*P*n]; // output, normal resampling operation
    float complex buf_out_1[2*P*n]; // output, partitioned into 2 blocks

    // generate input signal (pulse)
    unsigned int i;
    for (i=0; i<2*Q*n; i++)
        buf_in[i] = liquid_hamming(i,2*Q*n) * cexpf(_Complex_I*2*M_PI*0.037f*i);

    // run resampler normally in one large block (2*Q*n inputs, 2*P*n outputs)
    rresamp_crcf_execute_block(q0, buf_in, 2*n, buf_out_0);

    // reset and run with separate resamplers (e.g. in two threads)
    rresamp_crcf_reset(q0);
    // first block runs as normal
    rresamp_crcf_execute_block(q0, buf_in, n, buf_out_1);
    // initialize second block with Q*m samples to account for delay
    for (i=0; i<m; i++)
        rresamp_crcf_write(q1, buf_in + Q*n - (m-i)*Q);
    // run remainder of second block as normal
    rresamp_crcf_execute_block(q1, buf_in + Q*n, n, buf_out_1 + P*n);

    // clean up allocated objects
    rresamp_crcf_destroy(q0);
    rresamp_crcf_destroy(q1);

    // compute RMS error between output buffers
    float rmse = 0.0f;
    for (i=0; i<2*P*n; i++) {
        float complex err = buf_out_0[i] - buf_out_1[i];
        rmse += crealf( err * conjf(err) );
    }
    rmse = sqrtf( rmse / (float)(2*P*n) );
    printf("rmse=%.3g\n", rmse);

    // export results to file for plotting
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"P = %u; Q = %u; m = %u; n= %u;\n", P, Q, m, n);
    fprintf(fid,"x = zeros(1,2*Q*n);\n");
    fprintf(fid,"y = zeros(1,2*P*n);\n");
    fprintf(fid,"z = zeros(1,2*P*n);\n");
    fprintf(fid,"r = P/Q;\n");
    for (i=0; i<2*Q*n; i++)
        fprintf(fid,"x(%3u) = %12.8f + 1i*%12.8f;\n", i+1, crealf(buf_in[i]), cimagf(buf_in[i]));
    for (i=0; i<2*P*n; i++)
        fprintf(fid,"y(%3u) = %12.8f + 1i*%12.8f;\n", i+1, crealf(buf_out_0[i]), cimagf(buf_out_0[i]));
    for (i=0; i<2*P*n; i++)
        fprintf(fid,"z(%3u) = %12.8f + 1i*%12.8f;\n", i+1, crealf(buf_out_1[i]), cimagf(buf_out_1[i]));
    fprintf(fid,"\n\n");
    fprintf(fid,"%% plot time-domain result\n");
    fprintf(fid,"tx=0:(2*Q*n-1);\n");
    fprintf(fid,"ty=[(0:(2*P*n-1))]/r-m;\n");
    fprintf(fid,"figure('Color','white','position',[500 500 800 600]);\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(tx,real(x),        '-','LineWidth',2,'Color',[0.5 0.5 0.5],...\n");
    fprintf(fid,"       ty,real(y)*sqrt(r),'o','LineWidth',2,'Color',[0.5 0.5 0.5],'MarkerSize',3,...\n");
    fprintf(fid,"       ty,real(z)*sqrt(r),'o','LineWidth',2,'Color',[0.0 0.2 0.5],'MarkerSize',1);\n");
    fprintf(fid,"  legend('original','resampled (normal)','resampled (partitions)','location','northeast');");
    fprintf(fid,"  xlabel('Input Sample Index');\n");
    fprintf(fid,"  ylabel('Real Signal');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  title('Comparison of Normal and Partitioned rresamp, RMSE=%.3g');\n", rmse);
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(tx,imag(x),        '-','LineWidth',2,'Color',[0.5 0.5 0.5],...\n");
    fprintf(fid,"       ty,imag(y)*sqrt(r),'o','LineWidth',2,'Color',[0.5 0.5 0.5],'MarkerSize',3,...\n");
    fprintf(fid,"       ty,imag(z)*sqrt(r),'o','LineWidth',2,'Color',[0.0 0.5 0.2],'MarkerSize',1);\n");
    fprintf(fid,"  legend('original','resampled (normal)','resampled (partitions)','location','northeast');");
    fprintf(fid,"  xlabel('Input Sample Index');\n");
    fprintf(fid,"  ylabel('Real Signal');\n");
    fprintf(fid,"  grid on;\n");
    fclose(fid);
    printf("results written to %s\n",OUTPUT_FILENAME);
    return 0;
}
