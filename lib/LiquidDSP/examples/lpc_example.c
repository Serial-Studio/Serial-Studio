//
// lpc_example.c
//
// This example demonstrates linear prediction in liquid. An input signal
// is generated which exhibits a strong temporal correlation. The linear
// predictor generates an approximating all-pole filter which minimizes
// the squared error between the prediction and the actual output.
//

#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "lpc_example.m"

int main() {
    // options
    unsigned int n = 200;   // input sequence length
    unsigned int p = 4;     // prediction filter order

    // create low-pass filter object
    iirfilt_rrrf f = iirfilt_rrrf_create_lowpass(2, 0.05f);
    iirfilt_rrrf_print(f);

    unsigned int i;

    // allocate memory for data arrays
    float y[n];         // input signal (filtered noise)
    float a_hat[p+1];   // lpc output
    float g_hat[p+1];   // lpc output

    // generate input signal (filtered noise)
    for (i=0; i<n; i++)
        iirfilt_rrrf_execute(f, randnf(), &y[i]);

    // destroy filter object
    iirfilt_rrrf_destroy(f);

    // run linear prediction algorithm
    liquid_lpc(y,n,p,a_hat,g_hat);

    // run prediction filter
    float a_lpc[p+1];
    float b_lpc[p+1];
    for (i=0; i<p+1; i++) {
        a_lpc[i] = (i==0) ? 1.0f : 0.0f;
        b_lpc[i] = (i==0) ? 0.0f : -a_hat[i];
    }
    f = iirfilt_rrrf_create(b_lpc,p+1, a_lpc,p+1);
    iirfilt_rrrf_print(f);
    float y_hat[n];
    for (i=0; i<n; i++)
        iirfilt_rrrf_execute(f, y[i], &y_hat[i]);
    iirfilt_rrrf_destroy(f);

    // compute prediction error
    float err[n];
    for (i=0; i<n; i++)
        err[i] = y[i] - y_hat[i];

    // compute autocorrelation of prediction error
    float lag[n];
    float rxx[n];
    for (i=0; i<n; i++) {
        lag[i] = (float)i;
        rxx[i] = 0.0f;
        unsigned int j;
        for (j=i; j<n; j++)
            rxx[i] += err[j] * err[j-i];
    }
    float rxx0 = rxx[0];
    for (i=0; i<n; i++)
        rxx[i] /= rxx0;

    // print results
    for (i=0; i<p+1; i++)
        printf("  a[%3u] = %12.8f, g[%3u] = %12.8f\n", i, a_hat[i], i, g_hat[i]);

    printf("  prediction rmse = %12.8f\n", sqrtf(rxx0 / n));

    // 
    // plot results to output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n");
    fprintf(fid,"p=%u;\n", p);
    fprintf(fid,"n=%u;\n",n);

#if 0
    fprintf(fid,"b  = zeros(1,p+1);\n");
    fprintf(fid,"a  = zeros(1,p+1);\n");
    for (i=0; i<p+1; i++) {
        fprintf(fid,"b(%4u) = %12.4e;\n", i+1, b_lpc[i]);
        fprintf(fid,"a(%4u) = %12.4e;\n", i+1, a_lpc[i]);
    }
#endif

    fprintf(fid,"y      = zeros(1,n);\n");
    fprintf(fid,"y_hat  = zeros(1,n);\n");
    fprintf(fid,"lag    = zeros(1,n);\n");
    fprintf(fid,"rxx    = zeros(1,n);\n");
    for (i=0; i<n; i++) {
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"y_hat(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y_hat[i]), cimagf(y_hat[i]));
        fprintf(fid,"lag(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(lag[i]), cimagf(lag[i]));
        fprintf(fid,"rxx(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(rxx[i]), cimagf(rxx[i]));
    }

    // plot output
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(5,1,1:3);\n");
    fprintf(fid,"  hold on;\n");
    fprintf(fid,"  plot(t,y,    'LineWidth',1.5,'Color',[0 0.2 0.5]);\n");
    fprintf(fid,"  plot(t,y_hat,'LineWidth',1.5,'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  hold off;\n");
    fprintf(fid,"  ylabel('signal');\n");
    fprintf(fid,"  legend('input','LPC estimate','location','northeast');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(5,1,4);\n");
    fprintf(fid,"  plot(t,y-y_hat,'LineWidth',2,'Color',[1 1 1]*0.6);\n");
    fprintf(fid,"  ylabel('error');\n");
    fprintf(fid,"  axis([0 n -0.1 0.1]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(5,1,5);\n");
    fprintf(fid,"  plot(t,rxx,'LineWidth',2,'Color',[0.5 0 0]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('r_{xx}(e)');\n");
    fprintf(fid,"  axis([0 n -1.0 1]);\n");
    fprintf(fid,"  grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

