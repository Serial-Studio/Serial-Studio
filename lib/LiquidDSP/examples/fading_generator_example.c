//
// Fading generator example
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "debug_fading_generator_example.m"

int main() {
    // options
    unsigned int h_len=51;  // doppler filter length
    float fd=0.05f;         // maximum doppler frequency
    float K=2.0f;           // Rice fading factor
    float omega=1.0f;       // mean power
    float theta=0.0f;       // angle of arrival
    unsigned int n=256;     // number of samples

    // generate filter
    unsigned int i;
    float h[h_len];
    fir_design_doppler(h_len,fd,K,theta,h);
    firfilt_rrrf fi = firfilt_rrrf_create(h,h_len);
    firfilt_rrrf fq = firfilt_rrrf_create(h,h_len);

    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");

    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%3u) = %12.8e;\n", i+1, h[i]);

    // generate complex fading envelope
    float complex x, y;
    float yi, yq;
    float s = sqrtf((omega*K)/(K+1));
    float sig = sqrtf(0.5f*omega/(K+1));
    float t=0.0f;
    for (i=0; i<h_len; i++) {
        crandnf(&x);

        firfilt_rrrf_push(fi, crealf(x));
        firfilt_rrrf_push(fq, cimagf(x));

        t += h[i]*h[i];
    }
    t = sqrtf(t);

    for (i=0; i<n; i++) {
        crandnf(&x);

        firfilt_rrrf_push(fi, crealf(x));
        firfilt_rrrf_push(fq, cimagf(x));

        // compute outputs
        firfilt_rrrf_execute(fi, &yi);
        firfilt_rrrf_execute(fq, &yq);

        yi /= t;
        yq /= t;

        y = (yi*sig + s) + _Complex_I*(yq*sig);

        printf("%4u: r:%8.5f, angle:%5.2f\n", i, cabsf(y), cargf(y));

        fprintf(fid, "y(%4u) = %12.8e + j*%12.8e;\n",i+1,crealf(y),cimagf(y));
    }

    fprintf(fid,"\n\n");
    fprintf(fid,"nfft=1024;\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H=20*log10(abs(fftshift(fft(h,nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,H,'-','LineWidth',2);\n");
    fprintf(fid,"xlabel('Normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"\n\n");

    fprintf(fid,"t=0:length(y)-1;\n");
    fprintf(fid,"R=20*log10(abs(y));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,R,'-','LineWidth',2);\n");
    fprintf(fid,"xlabel('time [samples]');\n");
    fprintf(fid,"ylabel('fading amplitude [dB]');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);

    printf("results written to %s\n", OUTPUT_FILENAME);

    // clean up objects
    firfilt_rrrf_destroy(fi);
    firfilt_rrrf_destroy(fq);

    printf("done.\n");
    return 0;
}

