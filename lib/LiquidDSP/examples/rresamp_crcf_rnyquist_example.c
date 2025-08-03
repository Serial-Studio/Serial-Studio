//
// rresamp_crcf_rnyquist_example.c
//
// Demonstration of matched filter interpolator and decimator running at
// rational rate that is only slightly higher than occupied bandwidth.
// The resulting constellation has minimal inter-symbol interference and
// is normalized to unity gain.
//

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "rresamp_crcf_rnyquist_example.m"

// print usage/help message
void usage()
{
    printf("Usage: %s [OPTION]\n", __FILE__);
    printf("Resample a signal at a rate P/Q\n");
    printf("  -h            : print help\n");
    printf("  -P <decim>    : decimation (output) rate,       default: 3\n");
    printf("  -Q <interp>   : interpolation (input) rate,     default: 5\n");
    printf("  -m <len>      : filter semi-length (delay),     default: 12\n");
    printf("  -b <beta>     : filter excess bandwidth factor, default: 0.2\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int    P   = 5;        // output rate (interpolation factor)
    unsigned int    Q   = 4;        // input rate (decimation factor)
    unsigned int    m   = 15;       // resampling filter semi-length (filter delay)
    float           beta= 0.15f;    // filter excess bandwidth factor
    unsigned int    num_symbols = 800; // number of symbols to generate

    int dopt;
    while ((dopt = getopt(argc,argv,"hP:Q:m:b:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();            return 0;
        case 'P':   P    = atoi(optarg); break;
        case 'Q':   Q    = atoi(optarg); break;
        case 'm':   m    = atoi(optarg); break;
        case 'b':   beta = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (P == 0 || P > 1000) {
        fprintf(stderr,"error: %s, input rate P must be in [1,1000]\n", argv[0]);
        exit(1);
    } else if (Q == 0 || Q > 1000) {
        fprintf(stderr,"error: %s, output rate Q must be in [1,1000]\n", argv[0]);
        exit(1);
    } else if (Q > P) {
        fprintf(stderr,"error: %s, this example requires P > Q (interpolation)\n", argv[0]);
        exit(1);
    }

    // create resampler objects
    rresamp_crcf q0 = rresamp_crcf_create_prototype(LIQUID_FIRFILT_ARKAISER,P,Q,m,beta);
    rresamp_crcf q1 = rresamp_crcf_create_prototype(LIQUID_FIRFILT_ARKAISER,Q,P,m,beta);
    rresamp_crcf_print(q0);

    // input/output buffers
    float complex buf_x[Q]; // input
    float complex buf_y[P]; // interp
    float complex buf_z[Q]; // decim

    // generate input symbols
    unsigned int i, n = 0;
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"x = []; y = []; z = [];\n");
    float g = M_SQRT1_2;
    while (n < num_symbols) {
        n += Q;
        if (n > num_symbols-2*m)
            g = 0;
        // generate symbols
        for (i=0; i<Q; i++)
            buf_x[i] = g * ( (rand() & 1 ? 1 : -1 ) + ( rand() & 1 ? 1 : -1 )*_Complex_I);

        // run resampler and write P output samples
        rresamp_crcf_execute(q0, buf_x, buf_y);

        // run through matched filter
        rresamp_crcf_execute(q1, buf_y, buf_z);

        // write results to file
        for (i=0; i<Q; i++) fprintf(fid,"x(end+1) = %12.4e + 1i*%12.4e;\n", crealf(buf_x[i]), cimagf(buf_x[i]));
        for (i=0; i<P; i++) fprintf(fid,"y(end+1) = %12.4e + 1i*%12.4e;\n", crealf(buf_y[i]), cimagf(buf_y[i]));
        for (i=0; i<Q; i++) fprintf(fid,"z(end+1) = %12.4e + 1i*%12.4e;\n", crealf(buf_z[i]), cimagf(buf_z[i]));
    }
    fprintf(fid,"ny = length(y);\n");
    fprintf(fid,"ty = 0:(ny-1);\n");
    fprintf(fid,"nfft = 2^nextpow2(ny);\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"figure('position',[1 1 800 600]);\n");
    fprintf(fid,"subplot(2,2,1:2)\n");
    fprintf(fid,"  plot(ty,real(y),ty,imag(y));\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([0 ny -1.6 1.6]);\n");
    fprintf(fid,"  xlabel('output sample index');\n");
    fprintf(fid,"  ylabel('signal');\n");
    fprintf(fid,"  legend('real','imag');\n");
    fprintf(fid,"subplot(2,2,3)\n");
    fprintf(fid,"  plot(z,'x');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  xlabel('real');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"subplot(2,2,4)\n");
    fprintf(fid,"  plot(f,20*log10(abs(fftshift(fft(y/sqrt(ny),nfft)))));\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-0.5 0.5 -100 20]);\n");
    fprintf(fid,"  xlabel('Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fclose(fid);

    // clean up allocated objects
    rresamp_crcf_destroy(q0);
    rresamp_crcf_destroy(q1);
    printf("results written to %s\n",OUTPUT_FILENAME);
    return 0;
}
