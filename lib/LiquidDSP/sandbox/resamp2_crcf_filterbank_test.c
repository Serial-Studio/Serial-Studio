//
// resamp2_crcf_filterbank_test.c
//
// Halfband (two-channel) filterbank example. This example demonstrates
// the analyzer/synthesizer execute() methods for the resamp2_xxxt
// family of objects.
//
// NOTE: The filterbank is not a perfect reconstruction filter; a
//       significant amount of distortion occurs in the transition band
//       of the half-band filters.
//
// SEE ALSO: resamp2_crcf_decim_example.c
//           resamp2_crcf_interp_example.c
//

#include <stdio.h>
#include <complex.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "resamp2_crcf_filterbank_example.m"

int main() {
    unsigned int m=9;               // filter semi-length
    unsigned int num_samples=128;   // number of input samples
    float As=60.0f;                 // stop-band attenuation [dB]

    // derived values
    unsigned int h_len = 4*m+1;     // half-band filter length
    unsigned int N = num_samples + h_len;

    // ensure N is even
    N += (N%2);
    unsigned int n = N/2;

    // arrays
    float complex x[N];             // input time series
    float complex y[n][2];          // output time series (channelized)
    float complex z[N];             // output time series

    // generate input sequence
    unsigned int i;
    for (i=0; i<N; i++) {
        //x[i] = randnf() * cexpf(_Complex_I*M_PI*randf());
        x[i] = cexpf(_Complex_I*2*M_PI*0.072f*i) + 0.6f*cexpf(_Complex_I*2*M_PI*0.37f*i);
        //x[i] = cexpf(_Complex_I*2*M_PI*0.072f*i);
        //x[i] += 0.6*cexpf(_Complex_I*2*M_PI*0.572f*i);
        x[i] *= (i < num_samples) ? liquid_hamming(i,num_samples) : 0.0f;
    }

    // create/print the half-band filter with a specified
    // stop-band attenuation
    resamp2_crcf q = resamp2_crcf_create(m,0.0f,As);
    resamp2_crcf_print(q);

    // run the resampler as a two-channel analysis filterbank
    for (i=0; i<n; i++)
        resamp2_crcf_analyzer_execute(q, &x[2*i], &y[i][0]);

    // clear resampler
    resamp2_crcf_reset(q);

    // run the resampler as a two-channel synthesis filterbank
    for (i=0; i<n; i++)
        resamp2_crcf_synthesizer_execute(q, &y[i][0], &z[2*i]);

    // clean up allocated objects
    resamp2_crcf_destroy(q);

    // 
    // export output file
    //
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"N = %u;\n", N);

    // save results to output file
    for (i=0; i<n; i++) {
        fprintf(fid,"x(%3u)  = %12.4e + j*%12.4e;\n", 2*i+1, crealf(x[2*i+0]), cimagf(x[2*i+0]));
        fprintf(fid,"x(%3u)  = %12.4e + j*%12.4e;\n", 2*i+2, crealf(x[2*i+1]), cimagf(x[2*i+1]));

        fprintf(fid,"y0(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i][0]), cimagf(y[i][0]));
        fprintf(fid,"y1(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i][1]), cimagf(y[i][1]));

        fprintf(fid,"z(%3u)  = %12.4e + j*%12.4e;\n", 2*i+1, crealf(z[2*i+0]), cimagf(z[2*i+0]));
        fprintf(fid,"z(%3u)  = %12.4e + j*%12.4e;\n", 2*i+2, crealf(z[2*i+1]), cimagf(z[2*i+1]));
    }

    fprintf(fid,"tx = 0:(N-1);\n");
    fprintf(fid,"ty = tx(1:2:end);\n");
    fprintf(fid,"tz = tx - 4*m + 1;\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(tx, real(x), 'LineWidth',1,'Color',[0.50 0.50 0.50],...\n");
    fprintf(fid,"       tz, real(z), 'LineWidth',1,'Color',[0.00 0.25 0.50]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"  legend('original','reconstructed',1);");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(tx, imag(x), 'LineWidth',1,'Color',[0.50 0.50 0.50],...\n");
    fprintf(fid,"       tz, imag(z), 'LineWidth',1,'Color',[0.00 0.50 0.25]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"  legend('original','reconstructed',1);");


    fprintf(fid,"nfft=512;\n");
    fprintf(fid,"g = 1 / sqrt( real(x*x') );\n");
    fprintf(fid,"X =20*log10(abs(fftshift(fft(x*g, nfft))));\n");
    fprintf(fid,"Y0=20*log10(abs(fftshift(fft(y0*g,nfft))));\n");
    fprintf(fid,"Y1=20*log10(abs(fftshift(fft(y1*g,nfft))));\n");
    fprintf(fid,"Z =20*log10(abs(fftshift(fft(z*g, nfft))));\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,X, 'LineWidth',2,'Color',[0.50 0.50 0.50],...\n");
    fprintf(fid,"     f,Z, 'LineWidth',1,'Color',[0.00 0.25 0.50]);\n");
    //fprintf(fid,"     f,Y0,'LineWidth',1,'Color',[0.00 0.25 0.50],...\n");
    //fprintf(fid,"     f,Y1,'LineWidth',1,'Color',[0.00 0.50 0.25]);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"legend('original','reconstructed',2);");
    fprintf(fid,"axis([-0.5 0.5 -80 20]);\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
