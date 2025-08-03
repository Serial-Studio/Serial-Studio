//
// fftfilt_crcf_example.c
//
// Complex FFT-based finite impulse response filter example. This example
// demonstrates the functionality of firfilt by designing a low-order 
// prototype and using it to filter a noisy signal.  The filter coefficients
// are  real, but the input and output arrays are complex. The filter order
// and cutoff frequency are specified at the beginning, and the result is
// compared to the regular corresponding firfilt_crcf output.
//
// SEE ALSO: `firfilt_crcf_example.c`
//

#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "fftfilt_crcf_example.m"

int main() {
    // options
    unsigned int h_len=57;      // filter length
    float fc=0.10f;             // cutoff frequency
    float As=60.0f;             // stop-band attenuation
    unsigned int n=64;          // number of samples per block
    unsigned int num_blocks=6;  // total number of blocks

    // derived values
    unsigned int num_samples = n * num_blocks;

    // design filter
    float h[h_len];
    liquid_firdes_kaiser(h_len, fc, As, 0, h);

    // design FFT-based filter and scale to bandwidth
    fftfilt_crcf q0 = fftfilt_crcf_create(h, h_len, n);
    fftfilt_crcf_set_scale(q0, 2.0f*fc);

    // design regular FIR filter
    firfilt_crcf q1 = firfilt_crcf_create(h, h_len);
    firfilt_crcf_set_scale(q1, 2.0f*fc);

    unsigned int i;

    // allocate memory for data arrays
    float complex x[num_samples];   // input
    float complex y0[num_samples];  // output (fftfilt)
    float complex y1[num_samples];  // output (firfilt)

    // generate input signal (noise)
    for (i=0; i<num_samples; i++)
        x[i] = randnf() + _Complex_I*randnf();

    // run signal through fft-based filter in blocks
    for (i=0; i<num_blocks; i++)
        fftfilt_crcf_execute(q0, &x[i*n], &y1[i*n]);

    // run signal through regular filter one sample at a time
    for (i=0; i<num_samples; i++) {
        // run filter
        firfilt_crcf_push(q1, x[i]);
        firfilt_crcf_execute(q1, &y0[i]);
    }

    // destroy filter objects
    fftfilt_crcf_destroy(q0);
    firfilt_crcf_destroy(q1);

    // compute error norm
    float rmse = 0.0f;
    printf("  %6s : %8s : %8s (%8s), %8s : %8s (%8s)\n",
            "index",
            "re{fir}", "re{fft}", "re{err}",
            "im{fir}", "im{fft}", "im{err}");
    for (i=0; i<num_samples; i++) {
        float complex e = y0[i] - y1[i];
        printf("  %6u : %8.5f : %8.5f (%8.5f), %8.5f : %8.5f (%8.5f)\n",
                i,
                crealf(y0[i]), crealf(y1[i]), crealf(e),
                cimagf(y0[i]), cimagf(y1[i]), cimagf(e));

        // accumulate error
        rmse += crealf( e*conjf(e) );
    }
    // normalize RMS error
    rmse = sqrtf( rmse/(float)num_samples );
    printf("  rmse : %12.4e\n", rmse);

    // 
    // plot results to output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"\n");
    fprintf(fid,"h_len=%u;\n", h_len);
    fprintf(fid,"n=%u;\n",num_samples);
    fprintf(fid,"x =zeros(1,n);\n");
    fprintf(fid,"y0=zeros(1,n);\n");
    fprintf(fid,"y1=zeros(1,n);\n");

    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x( %4u) = %12.4e + j*%12.4e;\n", i+1, crealf( x[i]), cimagf( x[i]));
        fprintf(fid,"y0(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y0[i]), cimagf(y0[i]));
        fprintf(fid,"y1(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y1[i]), cimagf(y1[i]));
    }

    // plot output
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(y0),'-','Color',[0 0.5 0.2],'LineWidth',1,...\n");
    fprintf(fid,"       t,real(y1),'-','Color',[0 0.2 0.5],'LineWidth',1);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"  legend('fftfilt','firfilt','location','northeast');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,imag(y0),'-','Color',[0 0.5 0.2],'LineWidth',1,...\n");
    fprintf(fid,"       t,imag(y1),'-','Color',[0 0.2 0.5],'LineWidth',1);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"  legend('fftfilt','firfilt','location','northeast');\n");
    fprintf(fid,"  grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

