//
// symtrack_cccf_example.c
//
// This example demonstrates how to recover data symbols using the symtrack
// object. A stream of modulated and interpolated symbols are generated using
// the symstream object. The resulting samples are passed through a channel
// to add various impairments. The symtrack object recovers timing, carrier,
// and other information imparted by the channel and returns data symbols
// ready for demodulation.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symtrack_cccf_example.m"

// print usage/help message
void usage()
{
    printf("symtrack_cccf_example [options]\n");
    printf("  h     : print this help file\n");
    printf("  k     : filter samples/symbol,   default: 2\n");
    printf("  m     : filter delay (symbols),  default: 3\n");
    printf("  b     : filter excess bandwidth, default: 0.5\n");
    printf("  s     : signal-to-noise ratio,   default: 30 dB\n");
    printf("  w     : timing pll bandwidth,    default: 0.02\n");
    printf("  n     : number of symbols,       default: 4000\n");
}

int main(int argc, char*argv[])
{
    // options
    int          ftype       = LIQUID_FIRFILT_ARKAISER;
    int          ms          = LIQUID_MODEM_QAM16;
    unsigned int k           = 2;       // samples per symbol
    unsigned int m           = 7;       // filter delay (symbols)
    float        beta        = 0.20f;   // filter excess bandwidth factor
    unsigned int num_symbols = 4000;    // number of data symbols
    unsigned int hc_len      =   4;     // channel filter length
    float        noise_floor = -60.0f;  // noise floor [dB]
    float        SNRdB       = 30.0f;   // signal-to-noise ratio [dB]
    float        bandwidth   =  0.10f;  // loop filter bandwidth
    float        dphi        =  0.02f;  // carrier frequency offset [radians/sample]
    float        phi         =  2.1f;   // carrier phase offset [radians]

    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_samples = 200000;  // number of samples

    int dopt;
    while ((dopt = getopt(argc,argv,"hk:m:b:s:w:n:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                        return 0;
        case 'k':   k           = atoi(optarg);     break;
        case 'm':   m           = atoi(optarg);     break;
        case 'b':   beta        = atof(optarg);     break;
        case 's':   SNRdB       = atof(optarg);     break;
        case 'w':   bandwidth   = atof(optarg);     break;
        case 'n':   num_symbols = atoi(optarg);     break;
        default:
            exit(1);
        }
    }

    // validate input
    if (k < 2) {
        fprintf(stderr,"error: k (samples/symbol) must be greater than 1\n");
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: m (filter delay) must be greater than 0\n");
        exit(1);
    } else if (beta <= 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: beta (excess bandwidth factor) must be in (0,1]\n");
        exit(1);
    } else if (bandwidth <= 0.0f) {
        fprintf(stderr,"error: timing PLL bandwidth must be greater than 0\n");
        exit(1);
    } else if (num_symbols == 0) {
        fprintf(stderr,"error: number of symbols must be greater than 0\n");
        exit(1);
    }

    unsigned int i;

    // buffers
    unsigned int    buf_len = 800;      // buffer size
    float complex   x   [buf_len];      // original signal
    float complex   y   [buf_len];      // channel output
    float complex   syms[buf_len];      // recovered symbols
    // window for saving last few symbols
    windowcf sym_buf = windowcf_create(buf_len);

    // create stream generator
    symstreamcf gen = symstreamcf_create_linear(ftype,k,m,beta,ms);

    // create channel emulator and add impairments
    channel_cccf channel = channel_cccf_create();
    channel_cccf_add_awgn          (channel, noise_floor, SNRdB);
    channel_cccf_add_carrier_offset(channel, dphi, phi);
    channel_cccf_add_multipath     (channel, NULL, hc_len);

    // create symbol tracking synchronizer
    symtrack_cccf symtrack = symtrack_cccf_create(ftype,k,m,beta,ms);
    symtrack_cccf_set_bandwidth(symtrack,bandwidth);
    //symtrack_cccf_set_eq_off(symtrack); // disable equalization
    symtrack_cccf_print(symtrack);

    // create spectral periodogram for estimating spectrum
    spgramcf periodogram = spgramcf_create_default(nfft);

    unsigned int total_samples = 0;
    unsigned int total_symbols = 0;
    while (total_samples < num_samples)
    {
        // write samples to buffer
        symstreamcf_write_samples(gen, x, buf_len);

        // apply channel
        channel_cccf_execute_block(channel, x, buf_len, y);

        // push resulting sample through periodogram
        spgramcf_write(periodogram, y, buf_len);

        // run resulting stream through synchronizer
        unsigned int num_symbols_sync;
        symtrack_cccf_execute_block(symtrack, y, buf_len, syms, &num_symbols_sync);
        total_symbols += num_symbols_sync;

        // write resulting symbols to window buffer for plotting
        windowcf_write(sym_buf, syms, num_symbols_sync);

        // accumulated samples
        total_samples += buf_len;
    }
    printf("total samples: %u\n", total_samples);
    printf("total symbols: %u\n", total_symbols);

    // write accumulated power spectral density estimate
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    //
    // export output file
    //

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    // read buffer and write last symbols to file
    float complex * rc;
    windowcf_read(sym_buf, &rc);
    fprintf(fid,"syms = zeros(1,%u);\n", buf_len);
    for (i=0; i<buf_len; i++)
        fprintf(fid,"syms(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(rc[i]), cimagf(rc[i]));

    // power spectral density estimate
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"f=[0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"psd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%3u) = %12.8f;\n", i+1, psd[i]);

    fprintf(fid,"figure('Color','white','position',[500 500 1400 400]);\n");
    fprintf(fid,"subplot(1,3,1);\n");
    fprintf(fid,"plot(real(syms),imag(syms),'x','MarkerSize',4);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.6);\n");
    fprintf(fid,"  xlabel('In-phase');\n");
    fprintf(fid,"  ylabel('Quadrature');\n");
    fprintf(fid,"  title('Last %u symbols');\n", buf_len);
    fprintf(fid,"subplot(1,3,2:3);\n");
    fprintf(fid,"  plot(f, psd, 'LineWidth',1.5,'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  pmin = 10*floor(0.1*min(psd - 5));\n");
    fprintf(fid,"  pmax = 10*ceil (0.1*max(psd + 5));\n");
    fprintf(fid,"  axis([-0.5 0.5 pmin pmax]);\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('Power Spectral Density [dB]');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // destroy objects
    symstreamcf_destroy  (gen);
    spgramcf_destroy     (periodogram);
    channel_cccf_destroy (channel);
    symtrack_cccf_destroy(symtrack);
    windowcf_destroy     (sym_buf);

    // clean it up
    printf("done.\n");
    return 0;
}
