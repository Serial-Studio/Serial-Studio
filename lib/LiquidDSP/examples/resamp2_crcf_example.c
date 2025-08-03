//
// resamp2_crcf_example.c
//
// This example demonstrates the halfband resampler running as both an
// interpolator and a decimator. A narrow-band signal is first
// interpolated by a factor of 2, and then decimated. The resulting RMS
// error between the final signal and original is computed and printed
// to the screen.
//

#include <stdio.h>
#include <complex.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "resamp2_crcf_example.m"

int main() {
    unsigned int m=5;               // filter semi-length (actual length: 4*m+1)
    float bw=0.13f;                 // input signal bandwidth
    float fc=-0.597f;               // input signal carrier frequency (radians/sample)
    unsigned int num_samples=37;    // number of input samples
    float As=60.0f;                 // stop-band attenuation [dB]

    unsigned int i;

    // derived values
    unsigned int n = num_samples + 2*m + 1; // adjusted input sequence length

    // allocate memory for data arrays
    float complex x[  n];
    float complex y[2*n];
    float complex z[  n];

    // generate the baseband signal (filter pulse)
    float h[num_samples];
    liquid_firdes_kaiser(num_samples,bw,60.0f,0.0f,h);
    for (i=0; i<n; i++)
        x[i] = i < num_samples ? h[i] * cexpf(_Complex_I*fc*i) : 0.0f;

    // create/print the half-band resampler, with a specified
    // stopband attenuation level
    resamp2_crcf q = resamp2_crcf_create(m,0,As);
    resamp2_crcf_print(q);

    // run interpolation stage
    for (i=0; i<n; i++)
        resamp2_crcf_interp_execute(q, x[i], &y[2*i]);

    // clear resamp2 object
    resamp2_crcf_reset(q);
    resamp2_crcf_set_scale(q, 0.5f);

    // execute decimation stage
    for (i=0; i<n; i++)
        resamp2_crcf_decim_execute(q, &y[2*i], &z[i]);

    // clean up allocated objects
    resamp2_crcf_destroy(q);

    // compute RMS error
    float rmse = 0.0f;
    for (i=2*m; i<n; i++) {
        float e = cabsf(x[i-2*m] - z[i]);
        rmse += e*e;
    }
    rmse = sqrtf( rmse / (float)(n-2*m) );
    printf("rms error : %12.4e\n", rmse);

    // 
    // print results to file
    //
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"bw=%12.8f;\n", bw);
    fprintf(fid,"n=%u;\n", n);

    // output results
    for (i=0; i<n; i++)
        fprintf(fid,"x(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));

    for (i=0; i<2*n; i++)
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    for (i=0; i<n; i++)
        fprintf(fid,"z(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(z[i]), cimagf(z[i]));

    // print results
    fprintf(fid,"\n\n");
    fprintf(fid,"figure('position',[100 100 1200 600]);\n");
    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"X = 20*log10(abs(fftshift(fft(x*bw*2,nfft))));\n");
    fprintf(fid,"Y = 20*log10(abs(fftshift(fft(y*bw,  nfft))));\n");
    fprintf(fid,"Z = 20*log10(abs(fftshift(fft(z*bw*2,nfft))));\n");
    fprintf(fid,"subplot(3,2,[1,3,5,]);\n");
    fprintf(fid,"plot(f,X,f,Y,f,Z);\n");
    fprintf(fid,"legend('original','up-converted','down-converted');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-0.5 0.5 -120 20]);\n");

    fprintf(fid,"\n\n");
    fprintf(fid,"t0 = 0:[n-1];\n");
    fprintf(fid,"t1 = 0:[n*2-1];\n");
    fprintf(fid,"subplot(3,2,2);\n");
    fprintf(fid,"  plot(t0,real(x),t0,imag(x));\n");
    fprintf(fid,"  legend('I','Q');\n");
    fprintf(fid,"  axis([0 n -1 1]);\n");
    fprintf(fid,"  ylabel('original');\n");
    fprintf(fid,"subplot(3,2,4);\n");
    fprintf(fid,"  plot(t1,real(y),t1,imag(y));\n");
    fprintf(fid,"  axis([0 n*2 -1 1]);\n");
    fprintf(fid,"  ylabel('interpolated');\n");
    fprintf(fid,"subplot(3,2,6);\n");
    fprintf(fid,"  plot(t0,real(z),t0,imag(z));\n");
    fprintf(fid,"  axis([0 n -1 1]);\n");
    fprintf(fid,"  ylabel('interp/decim');\n");
    fclose(fid);
    printf("results written to %s\n",OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
