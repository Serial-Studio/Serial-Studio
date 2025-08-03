//
// msresamp_crcf_noise_example.c
//
// Demonstration of mulsti-stage resamp object whereby an input noise signal
// is resampled at a rate 'r'.
//

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "msresamp_crcf_noise_example.m"

// print usage/help message
void usage()
{
    printf("Usage: %s [OPTION]\n", __FILE__);
    printf("  -h         : print help\n");
    printf("  -r <rate>  : resampling rate,                   default: 0.3\n");
    printf("  -s <atten> : filter stop-band attenuation [dB], default: 60\n");
}

int main(int argc, char*argv[])
{
    // options
    float   rate= 0.30f;    // resampling rate
    float   As  = 60.0f;    // resampling filter stop-band attenuation [dB]

    int dopt;
    while ((dopt = getopt(argc,argv,"hr:s:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();             return 0;
        case 'r':   rate = atof(optarg); break;
        case 's':   As   = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (rate >= 1.0f) {
        fprintf(stderr,"error: %s, input rate r must be less than 1\n", argv[0]);
        exit(1);
    }

    // create resampler object
    msresamp_crcf q = msresamp_crcf_create(rate,As);
    msresamp_crcf_print(q);

    // number of sample blocks
    unsigned int num_blocks = 1000;

    // arrays
    unsigned int  buf_len = 1024;
    float complex buf_x[  buf_len];
    float complex buf_y[2*buf_len];

    // create multi-signal source generator
    msourcecf gen = msourcecf_create_default();

    // wide-band noise
    msourcecf_add_noise(gen,  0.0f, 1.00f, -60);

    // in-band signal
    msourcecf_add_noise(gen, 0.0f, 0.5f*rate, 0);

    // high-power signal just out of band
    msourcecf_add_noise(gen, (0.5*rate + 0.12f), 0.10f, 10);

    // create spectral periodogram objects
    unsigned int nfft = 2400;
    spgramcf px = spgramcf_create_default(nfft);
    spgramcf py = spgramcf_create_default(nfft);

    // generate input signal (filtered noise)
    unsigned int i;
    for (i=0; i<num_blocks; i++) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf_x, buf_len);

        // run resampler in blocks
        unsigned int num_written = 0;
        msresamp_crcf_execute(q, buf_x, buf_len, buf_y, &num_written);

        // write input and output to respective spectral periodogram estimate
        spgramcf_write(px, buf_x, buf_len);
        spgramcf_write(py, buf_y, num_written);
    }
    printf("num samples in  : %llu\n", spgramcf_get_num_samples_total(px));
    printf("num samples out : %llu\n", spgramcf_get_num_samples_total(py));

    // clean up allocated objects
    msresamp_crcf_destroy(q);

    // compute power spectral density output
    float X[nfft];
    float Y[nfft];
    spgramcf_get_psd(px, X);
    spgramcf_get_psd(py, Y);

    // export results to file for plotting
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"r    = %f;\n", rate);
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
    fprintf(fid,"Y = Y - 10*log10(r);\n");
    fprintf(fid,"figure('Color','white','position',[500 500 800 600]);\n");
    fprintf(fid,"plot(fx,X,'-','LineWidth',2,'Color',[0.5 0.5 0.5],'MarkerSize',1,...\n");
    fprintf(fid,"     fy,Y,'-','LineWidth',2,'Color',[0.5 0 0],    'MarkerSize',1);\n");
    fprintf(fid,"legend('original','resampled','location','northeast');");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"fmin = min(fx(   1),fy(   1));\n");
    fprintf(fid,"fmax = max(fx(nfft),fy(nfft));\n");
    fprintf(fid,"axis([fmin fmax -80 20]);\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s\n",OUTPUT_FILENAME);
    return 0;
}
