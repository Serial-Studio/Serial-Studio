//
// rresamp_rrrf_example.c
//
// Demonstration of rresamp object whereby an input signal
// is resampled at a rational rate Q/P.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "rresamp_rrrf_example.m"

// print usage/help message
void usage()
{
    printf("Usage: %s [OPTION]\n", __FILE__);
    printf("Resample a signal at a rate P/Q\n");
    printf("  -h            : print help\n");
    printf("  -P <decim>    : decimation (output) rate,          default: 3\n");
    printf("  -Q <interp>   : interpolation (input) rate,        default: 5\n");
    printf("  -m <len>      : filter semi-length (delay),        default: 12\n");
    printf("  -w <bandwidth>: filter bandwidth,                  default: 0.5f\n");
    printf("  -s <atten>    : filter stop-band attenuation [dB], default: 60\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int    P   = 3;        // output rate (interpolation factor)
    unsigned int    Q   = 5;        // input rate (decimation factor)
    unsigned int    m   = 12;       // resampling filter semi-length (filter delay)
    float           bw  = 0.5f;     // resampling filter bandwidth
    float           As  = 60.0f;    // resampling filter stop-band attenuation [dB]

    int dopt;
    while ((dopt = getopt(argc,argv,"hP:Q:m:s:w:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();            return 0;
        case 'P':   P    = atoi(optarg); break;
        case 'Q':   Q    = atoi(optarg); break;
        case 'm':   m    = atoi(optarg); break;
        case 'w':   bw   = atof(optarg); break;
        case 's':   As   = atof(optarg); break;
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
    }

    // create resampler object
    rresamp_rrrf q = rresamp_rrrf_create_kaiser(P,Q,m,bw,As);
    rresamp_rrrf_print(q);
    float rate = rresamp_rrrf_get_rate(q);

    // number of sample blocks (limit by large interp/decim rates)
    unsigned int n = 120e3 / (P > Q ? P : Q);

    // input/output buffers
    float buf_x[Q]; // input
    float buf_y[P]; // output

    // create wide-band noise source with one-sided cut-off frequency
    iirfilt_rrrf lowpass = iirfilt_rrrf_create_lowpass(15, 0.7f*0.5f*(rate > 1.0 ? 1.0 : rate));

    // create spectral periodogram objects
    unsigned int nfft = 2400;
    spgramf px = spgramf_create_default(nfft);
    spgramf py = spgramf_create_default(nfft);

    // generate input signal (filtered noise)
    unsigned int i, j;
    for (i=0; i<n; i++) {
        // write Q input samples to buffer
        for (j=0; j<Q; j++)
            iirfilt_rrrf_execute(lowpass, randnf(), &buf_x[j]);

        // run resampler and write P output samples
        rresamp_rrrf_execute(q, buf_x, buf_y);

        // write input and output to respective spectral periodogram estimate
        spgramf_write(px, buf_x, Q);
        spgramf_write(py, buf_y, P);
    }
    printf("num samples out : %llu\n", spgramf_get_num_samples_total(py));
    printf("num samples in  : %llu\n", spgramf_get_num_samples_total(px));

    // clean up allocated objects
    rresamp_rrrf_destroy(q);
    iirfilt_rrrf_destroy(lowpass);

    // compute power spectral density output
    float X[nfft];
    float Y[nfft];
    spgramf_get_psd(px, X);
    spgramf_get_psd(py, Y);

    // export results to file for plotting
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"P    = %u;\n", P);
    fprintf(fid,"Q    = %u;\n", Q);
    fprintf(fid,"r    = P/Q;\n");
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"X    = zeros(1,nfft);\n");
    fprintf(fid,"Y    = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++) {
        fprintf(fid,"X(%3u) = %12.4e;\n", i+1, X[i]);
        fprintf(fid,"Y(%3u) = %12.4e;\n", i+1, Y[i]);
    }
    fprintf(fid,"\n\n");
    fprintf(fid,"%% plot time-domain result\n");
    fprintf(fid,"fx=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"fy=fx*r;\n");
    fprintf(fid,"figure('Color','white','position',[500 500 800 600]);\n");
    fprintf(fid,"plot(fx,X,'-','LineWidth',2,'Color',[0.5 0.5 0.5],'MarkerSize',1,...\n");
    fprintf(fid,"     fy,Y,'-','LineWidth',2,'Color',[0.5 0 0],    'MarkerSize',1);\n");
    fprintf(fid,"legend('original','resampled','location','northeast');");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"fmin = min(fx(   1),fy(   1));\n");
    fprintf(fid,"fmax = max(fx(nfft),fy(nfft));\n");
    fprintf(fid,"axis([fmin fmax -100 20]);\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s\n",OUTPUT_FILENAME);
    return 0;
}
