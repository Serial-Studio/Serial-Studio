// Direct digital synthesizer example.  This example demonstrates
// the interface to the direct digital synthesizer.  A baseband
// pulse is generated and then efficiently up-converted
// (interpolated and mixed up) using the DDS object.  The resulting
// signal is then down-converted (mixed down and decimated) using
// the same DDS object.  Results are written to a file.
// SEE ALSO: firinterp_crcf_example.c
//           decim_crcf_example.c
//           resamp2_crcf_example.c
//           nco_example.c
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "dds_cccf_example.m"

int main() {
    // options
    float           fc          = -0.2f;    // input (output) decim (interp) frequency
    unsigned int    num_stages  = 3;        // number of halfband interp/decim stages
    unsigned int    num_samples = 64;       // number of input samples
    float           As          = 60.0f;    // DDS stop-band attenuation [dB]
    float           bw          = 0.25f;    // signal bandwidth

    // derived values
    unsigned int r=1<<num_stages;   // resampling rate (output/input)

    // create resampler
    dds_cccf q = dds_cccf_create(num_stages,fc,bw,As);
    dds_cccf_print(q);

    // open/initialize output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"n=%u;\n", num_samples);
    fprintf(fid,"r=%u;\n", r);

    float complex x[num_samples];
    float complex y[r*num_samples];
    float complex z[num_samples];

    unsigned int i;

    // generate the baseband signal (filter pulse)
    unsigned int h_len = num_samples/2;
    h_len += num_samples % 2 ? 0 : 1;
    float h[h_len];
    liquid_firdes_kaiser(h_len,bw,60.0f,0.0f,h);
    for (i=0; i<num_samples; i++)
        x[i] = i < h_len ? 2*h[i]*bw : 0.0f;

    // run interpolation (up-conversion) stage
    for (i=0; i<num_samples; i++) {
        dds_cccf_interp_execute(q, x[i], &y[r*i]);
    }

    // clear DDS object
    dds_cccf_reset(q);

    // run decimation (down-conversion) stage
    for (i=0; i<num_samples; i++) {
        dds_cccf_decim_execute(q, &y[r*i], &z[i]);
    }

    // output results
    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));

    for (i=0; i<r*num_samples; i++)
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    for (i=0; i<num_samples; i++)
        fprintf(fid,"z(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(z[i]), cimagf(z[i]));

    // print results
    fprintf(fid,"\n\n");
    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"X = 20*log10(abs(fftshift(fft(x,  nfft))));\n");
    fprintf(fid,"Y = 20*log10(abs(fftshift(fft(y/r,nfft))));\n");
    fprintf(fid,"Z = 20*log10(abs(fftshift(fft(z,  nfft))));\n");
    fprintf(fid,"plot(f,X,f,Y,f,Z);\n");
    fprintf(fid,"legend('original','up-converted','down-converted');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-0.5 0.5 -120 20]);\n");

    fprintf(fid,"\n\n");
    fprintf(fid,"t0 = 0:[n-1];\n");
    fprintf(fid,"t1 = 0:[n*r-1];\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t0,real(x),'-s','MarkerSize',3,t0,imag(x),'-s','MarkerSize',3);\n");
    fprintf(fid,"  legend('I','Q');\n");
    fprintf(fid,"  axis([0 n -0.55 0.55]);\n");
    fprintf(fid,"  ylabel('original');\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(t1,real(y),t1,imag(y));\n");
    fprintf(fid,"  axis([0 n*r -0.55 0.55]);\n");
    fprintf(fid,"  ylabel('up-converted');\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(t0,real(z),'-s','MarkerSize',3,t0,imag(z),'-s','MarkerSize',3);\n");
    fprintf(fid,"  axis([0 n -0.55 0.55]);\n");
    fprintf(fid,"  ylabel('down-converted');\n");

    fclose(fid);
    printf("results written to %s\n",OUTPUT_FILENAME);

    // clean up allocated objects
    dds_cccf_destroy(q);

    printf("done.\n");
    return 0;
}
