//
// resamp2_crcf_interp_recreate_test.c
//
// Halfband interpolator (complex), recreate() example
//

#include <stdio.h>
#include <complex.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "resamp2_crcf_interp_recreate_test.m"

int main() {
    unsigned int m0=5;
    unsigned int m1=4;
    unsigned int h0_len = 4*m0+1; // initial filter length
    unsigned int h1_len = 4*m1+1; // filter length (after recreate)
    float fc=0.10f;
    unsigned int N=32;
    float As=60.0f;

    resamp2_cccf f = resamp2_cccf_create(h0_len,0.0f,As);

    resamp2_cccf_print(f);

    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"h0_len = %u;\n", h0_len);
    fprintf(fid,"h1_len = %u;\n", h1_len);
    fprintf(fid,"N      = %u;\n", N);

    unsigned int i;
    float theta=0.0f, dtheta=2*M_PI*fc;
    unsigned int ix=0, iy=0;
    float complex x, y[2];
    for (i=0; i<N/2; i++) {
        x = cexpf(_Complex_I*theta);
        theta += dtheta;

        resamp2_cccf_interp_execute(f, x, y);

        ix++;
        iy++;
        fprintf(fid,"x(%3u) = %8.4f + j*%8.4f;\n", ix+1,   crealf(x),    cimagf(x));
        fprintf(fid,"y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+1, crealf(y[0]), cimagf(y[0]));
        fprintf(fid,"y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+2, crealf(y[1]), cimagf(y[1]));

        printf("y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+1, crealf(y[0]), cimagf(y[0]));
        printf("y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+2, crealf(y[1]), cimagf(y[1]));
    }

    // re-create interpolator
    printf("recreating filter...\n");
    f = resamp2_cccf_recreate(f,h1_len,0.0f,As);

    // TODO: push additional values?


    for (i=0; i<N/2; i++) {
        x = cexpf(_Complex_I*theta);
        theta += dtheta;

        resamp2_cccf_interp_execute(f, x, y);

        ix++;
        iy++;
        fprintf(fid,"x(%3u) = %8.4f + j*%8.4f;\n", ix+1,   crealf(x),    cimagf(x));
        fprintf(fid,"y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+1, crealf(y[0]), cimagf(y[0]));
        fprintf(fid,"y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+2, crealf(y[1]), cimagf(y[1]));

        printf("y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+1, crealf(y[0]), cimagf(y[0]));
        printf("y(%3u) = %8.4f + j*%8.4f;\n", 2*iy+2, crealf(y[1]), cimagf(y[1]));
    }


    fprintf(fid,"nfft=512;\n");
    fprintf(fid,"X=20*log10(abs(fftshift(fft(x.*hamming(length(x))',nfft))));\n");
    fprintf(fid,"Y=20*log10(abs(fftshift(fft(y.*hamming(length(y))',nfft))));\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure; plot(f/2,X,'Color',[0.5 0.5 0.5],f,Y,'LineWidth',2);\n");
    fprintf(fid,"grid on;\nxlabel('normalized frequency');\nylabel('PSD [dB]');\n");
    fprintf(fid,"legend('original','interpolated',1);");

    fclose(fid);
    printf("results written to %s\n",OUTPUT_FILENAME);

    resamp2_cccf_destroy(f);
    printf("done.\n");
    return 0;
}
