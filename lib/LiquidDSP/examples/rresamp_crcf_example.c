// Demonstration of rresamp object whereby an input signal
// is resampled at a rational rate P/Q = interp/decim.
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "rresamp_crcf_example.m"

// print usage/help message
void usage()
{
    printf("Usage: %s [OPTION]\n", __FILE__);
    printf("Resample a signal at a rate interp/decim\n");
    printf("  -h            : print help\n");
    printf("  -P <interp>   : interpolation rate,                default: 5\n");
    printf("  -Q <decim>    : decimation rate,                   default: 3\n");
    printf("  -m <len>      : filter semi-length (delay),        default: 12\n");
    printf("  -w <bandwidth>: filter bandwidth,                  default: -1\n");
    printf("  -s <atten>    : filter stop-band attenuation [dB], default: 60\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int interp = 3;        // output rate (interpolation factor)
    unsigned int decim  = 5;        // input rate (decimation factor)
    unsigned int m      = 20;       // resampling filter semi-length (filter delay)
    float        bw     = -1.0f;    // resampling filter bandwidth
    float        As     = 60.0f;    // resampling filter stop-band attenuation [dB]

    int dopt;
    while ((dopt = getopt(argc,argv,"hP:Q:m:s:w:")) != EOF) {
        switch (dopt) {
        case 'h': usage();               return 0;
        case 'P': interp = atoi(optarg); break;
        case 'Q': decim  = atoi(optarg); break;
        case 'm': m      = atoi(optarg); break;
        case 'w': bw     = atof(optarg); break;
        case 's': As     = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (interp == 0 || interp > 1000) {
        fprintf(stderr,"error: %s, interpolation rate must be in [1,1000]\n", argv[0]);
        exit(1);
    } else if (decim == 0 || decim > 1000) {
        fprintf(stderr,"error: %s, decimation rate must be in [1,1000]\n", argv[0]);
        exit(1);
    }

    // create resampler object
    rresamp_crcf q = rresamp_crcf_create_kaiser(interp,decim,m,bw,As);
    if (q == NULL) return -1;
    rresamp_crcf_print(q);
    float rate = rresamp_crcf_get_rate(q);

    // number of sample blocks (limit by large interp/decim rates)
    unsigned int n = 120e3 / (interp > decim ? interp : decim);

    // input/output buffers
    float complex buf_x[decim ]; // input
    float complex buf_y[interp]; // output

    // create signal generator (wide-band noise and tone)
    float noise_bw = 0.7f * (rate > 1.0 ? 1.0 : rate);
    msourcecf gen = msourcecf_create_default();
    msourcecf_add_noise(gen, 0.0f, noise_bw,  0);
    msourcecf_add_tone (gen, rate > 1.0 ? 0.4 : noise_bw, 0.00f, 20);

    // create spectral periodogram objects
    unsigned int nfft = 2400;
    spgramcf px = spgramcf_create_default(nfft);
    spgramcf py = spgramcf_create_default(nfft);

    // generate input signal (filtered noise)
    unsigned int i;
    for (i=0; i<n; i++) {
        // write decim input samples to buffer
        msourcecf_write_samples(gen, buf_x, decim);

        // run resampler and write interp output samples
        rresamp_crcf_execute(q, buf_x, buf_y);

        // write input and output to respective spectral periodogram estimate
        spgramcf_write(px, buf_x, decim);
        spgramcf_write(py, buf_y, interp);
    }
    printf("num samples out : %llu\n", spgramcf_get_num_samples_total(py));
    printf("num samples in  : %llu\n", spgramcf_get_num_samples_total(px));

    // clean up allocated objects
    rresamp_crcf_destroy(q);

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
    fprintf(fid,"interp = %u;\n", interp);
    fprintf(fid,"decim  = %u;\n", decim);
    fprintf(fid,"r      = interp/decim;\n");
    fprintf(fid,"nfft   = %u;\n", nfft);
    fprintf(fid,"X      = zeros(1,nfft);\n");
    fprintf(fid,"Y      = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++) {
        fprintf(fid,"X(%3u) = %12.4e;\n", i+1, X[i]);
        fprintf(fid,"Y(%3u) = %12.4e;\n", i+1, Y[i]);
    }
    fprintf(fid,"\n\n");
    fprintf(fid,"%% plot frequency-domain result\n");
    fprintf(fid,"fx=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"fy=fx*r;\n");
    fprintf(fid,"figure('Color','white','position',[500 500 800 600]);\n");
    fprintf(fid,"plot(fx,X,'-','LineWidth',2,'Color',[0.5 0.5 0.5],'MarkerSize',1,...\n");
    fprintf(fid,"     fy,Y,'-','LineWidth',2,'Color',[0.5 0 0],    'MarkerSize',1);\n");
    fprintf(fid,"legend('original','resampled','location','northwest');");
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
