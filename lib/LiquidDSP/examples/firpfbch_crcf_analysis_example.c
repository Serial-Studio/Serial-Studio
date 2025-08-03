//
// firpfbch_crcf_analysis_example.c
//
// Example of the analysis channelizer filterbank. The input signal is
// comprised of several signals spanning different frequency bands. The
// channelizer downconverts each to baseband (maximally decimated), and
// the resulting spectrum of each is plotted.
//

#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firpfbch_crcf_analysis_example.m"

int main() {
    // options
    unsigned int num_channels =  8;     // number of channels
    unsigned int m            =  4;     // filter delay
    float        As           = 60;     // stop-band attenuation
    unsigned int num_frames   = 25;     // number of frames

    //
    unsigned int i;
    unsigned int k;

    // derived values
    unsigned int num_samples = num_frames * num_channels;

    // data arrays
    float complex x[num_samples];  // time-domain input  [size: num_samples  x 1         ]
    float complex y[num_samples];  // channelized output [size: num_channels x num_frames]

    // initialize input with zeros
    for (i=0; i<num_samples; i++)
        x[i] = 0.0f;

    // generate input signal(s)
    unsigned int num_signals = 4;
    float fc[4] = {0.0f,   0.25f,  0.375f, -0.375f}; // center frequencies
    float bw[4] = {0.035f, 0.035f, 0.035f,  0.035f}; // bandwidths
    unsigned int pulse_len = 137;
    float pulse[pulse_len];
    for (i=0; i<num_signals; i++) {
        // create pulse
        liquid_firdes_kaiser(pulse_len, bw[i], 50.0f, 0.0f, pulse);

        // add pulse to input signal with carrier offset
        for (k=0; k<pulse_len; k++)
            x[k] += pulse[k] * cexpf(_Complex_I*2*M_PI*fc[i]*k) * bw[i];
    }

    // create prototype filter
    unsigned int h_len = 2*num_channels*m + 1;
    float h[h_len];
    liquid_firdes_kaiser(h_len, 0.5f/(float)num_channels, As, 0.0f, h);

#if 0
    // create filterbank channelizer object using internal method for filter
    firpfbch_crcf q = firpfbch_crcf_create_kaiser(LIQUID_ANALYZER, num_channels, m, As);
#else
    // create filterbank channelizer object using external filter coefficients
    firpfbch_crcf q = firpfbch_crcf_create(LIQUID_ANALYZER, num_channels, 2*m, h);
#endif

    // channelize input data
    for (i=0; i<num_frames; i++) {
        // execute analysis filter bank
        firpfbch_crcf_analyzer_execute(q, &x[i*num_channels], &y[i*num_channels]);
    }

    // destroy channelizer object
    firpfbch_crcf_destroy(q);
    
    // 
    // export results to file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"num_channels = %u;\n", num_channels);
    fprintf(fid,"m            = %u;\n", m);
    fprintf(fid,"num_frames   = %u;\n", num_frames);
    fprintf(fid,"h_len        = 2*num_channels*m+1;\n");
    fprintf(fid,"num_samples  = num_frames*num_channels;\n");

    fprintf(fid,"h = zeros(1,h_len);\n");
    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(num_channels, num_frames);\n");

    // save prototype filter
    for (i=0; i<h_len; i++)
        fprintf(fid,"  h(%6u) = %12.4e;\n", i+1, h[i]);

    // save input signal
    for (i=0; i<num_samples; i++)
        fprintf(fid,"  x(%6u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));

    // save channelized output signals
    for (i=0; i<num_frames; i++) {
        for (k=0; k<num_channels; k++) {
            float complex v = y[i*num_channels + k];
            fprintf(fid,"  y(%3u,%6u) = %12.4e + 1i*%12.4e;\n", k+1, i+1, crealf(v), cimagf(v));
        }
    }

    // plot results
    fprintf(fid,"\n");
    fprintf(fid,"nfft = 1024;\n"); // TODO: use nextpow2
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H = 20*log10(abs(fftshift(fft(h/num_channels,nfft))));\n");
    fprintf(fid,"X = 20*log10(abs(fftshift(fft(x,nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(f, H, 'Color', [0 0.5 0.25], 'LineWidth', 2);\n");
    fprintf(fid,"  axis([-0.5 0.5 -100 10]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('Prototype Filter PSD');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(f, X, 'Color', [0 0.25 0.5], 'LineWidth', 2);\n");
    fprintf(fid,"  axis([-0.5 0.5 -100 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('Input PSD');\n");

    // compute the PSD of each output and plot results on a square grid
    fprintf(fid,"n = ceil(sqrt(num_channels));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"for i=1:num_channels,\n");
    fprintf(fid,"  Y = 20*log10(abs(fftshift(fft(y(i,:),nfft))));\n");
    fprintf(fid,"  subplot(n,n,i);\n");
    fprintf(fid,"  plot(f, Y, 'Color', [0.25 0 0.25], 'LineWidth', 1.5);\n");
    fprintf(fid,"  axis([-0.5 0.5 -120 20]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  title(num2str(i-1));\n");
    fprintf(fid,"end;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

