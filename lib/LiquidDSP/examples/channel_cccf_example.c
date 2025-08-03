//
// channel_cccf_example.c
//
// This example demonstrates how the channel_cccf object can be used to
// emulate a multi-path fading, log-normal shadowing, and AWGN channel.
// A stream of modulated and interpolated symbols are generated using the
// symstream object. The resulting samples are passed through a channel
// to add various impairments. The symtrack object recovers timing,
// carrier, and other information imparted by the channel and returns
// data symbols ready for demodulation.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "channel_cccf_example.m"

// print usage/help message
void usage()
{
    printf("channel_cccf_example [options]\n");
    printf("  h     : print this help file\n");
    printf("  k     : filter samples/symbol,     default: 2\n");
    printf("  m     : filter delay (symbols),    default: 3\n");
    printf("  b     : filter excess bandwidth,   default: 0.5\n");
    printf("  H     : multi-path channel length, default: 4000\n");
    printf("  n     : number of symbols,         default: 4000\n");
    printf("  s     : signal-to-noise ratio,     default: 30 dB\n");
    printf("  w     : timing pll bandwidth,      default: 0.02\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int k           = 2;       // samples per symbol
    unsigned int m           = 7;       // filter delay (symbols)
    float        beta        = 0.25f;   // filter excess bandwidth factor
    unsigned int num_symbols = 4000;    // number of data symbols
    unsigned int hc_len      = 5;       // channel filter length
    float        noise_floor = -60.0f;  // noise floor [dB]
    float        SNRdB       = 30.0f;   // signal-to-noise ratio [dB]
    float        bandwidth   =  0.02f;  // loop filter bandwidth
    float        dphi        =  0.00f;  // carrier frequency offset [radians/sample]
    float        phi         =  2.1f;   // carrier phase offset [radians]
    modulation_scheme ms     = LIQUID_MODEM_QPSK;

    int dopt;
    while ((dopt = getopt(argc,argv,"hk:m:b:H:n:s:w:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                        return 0;
        case 'k':   k           = atoi(optarg);     break;
        case 'm':   m           = atoi(optarg);     break;
        case 'b':   beta        = atof(optarg);     break;
        case 'H':   hc_len      = atoi(optarg);     break;
        case 'n':   num_symbols = atoi(optarg);     break;
        case 's':   SNRdB       = atof(optarg);     break;
        case 'w':   bandwidth   = atof(optarg);     break;
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

    // derived/fixed values
    unsigned int num_samples = num_symbols*k;

    float complex x[num_samples];    // input (interpolated) samples
    float complex y[num_samples];    // channel output samples
    float complex sym_out[num_symbols + 64];// synchronized symbols

    // 
    // generate input sequence using symbol stream generator
    //
    symstreamcf gen = symstreamcf_create_linear(LIQUID_FIRFILT_ARKAISER,k,m,beta,ms);
    symstreamcf_write_samples(gen, x, num_samples);
    symstreamcf_destroy(gen);

    // create channel
    channel_cccf channel = channel_cccf_create();

    // add channel impairments
    channel_cccf_add_awgn          (channel, noise_floor, SNRdB);
    channel_cccf_add_carrier_offset(channel, dphi, phi);
    channel_cccf_add_multipath     (channel, NULL, hc_len);
    channel_cccf_add_shadowing     (channel, 1.0f, 0.1f);

    // print channel internals
    channel_cccf_print(channel);

    // apply channel to input signal
    channel_cccf_execute_block(channel, x, num_samples, y);

    // destroy channel
    channel_cccf_destroy(channel);

    // 
    // create and run symbol synchronizer
    //
    symtrack_cccf symtrack = symtrack_cccf_create(LIQUID_FIRFILT_RRC,k,m,beta,ms);
    
    // set tracking bandwidth
    symtrack_cccf_set_bandwidth(symtrack,0.05f);

    unsigned int num_symbols_sync = 0;
    symtrack_cccf_execute_block(symtrack, y, num_samples, sym_out, &num_symbols_sync);
    symtrack_cccf_destroy(symtrack);

    // print results
    printf("symbols in  : %u\n", num_symbols);
    printf("symbols out : %u\n", num_symbols_sync);

    // estimate spectrum
    unsigned int nfft = 1200;
    float        psd[nfft];
    spgramcf_estimate_psd(nfft, y, num_samples, psd);

    //
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"close all;\nclear all;\n\n");
    fprintf(fid,"num_symbols=%u;\n",num_symbols_sync);

    for (i=0; i<num_symbols_sync; i++)
        fprintf(fid,"z(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));

    // power spectral density estimate
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"f=[0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"psd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%3u) = %12.8f;\n", i+1, psd[i]);

    fprintf(fid,"iz0 = 1:round(length(z)*0.5);\n");
    fprintf(fid,"iz1 = round(length(z)*0.5):length(z);\n");
    fprintf(fid,"figure('Color','white','position',[500 500 800 800]);\n");
    fprintf(fid,"subplot(2,2,1);\n");
    fprintf(fid,"plot(real(z(iz0)),imag(z(iz0)),'x','MarkerSize',4);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.6);\n");
    fprintf(fid,"  xlabel('In-phase');\n");
    fprintf(fid,"  ylabel('Quadrature');\n");
    fprintf(fid,"  title('First 50%% of symbols');\n");
    fprintf(fid,"subplot(2,2,2);\n");
    fprintf(fid,"  plot(real(z(iz1)),imag(z(iz1)),'x','MarkerSize',4);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"  xlabel('In-phase');\n");
    fprintf(fid,"  ylabel('Quadrature');\n");
    fprintf(fid,"  title('Last 50%% of symbols');\n");
    fprintf(fid,"subplot(2,2,3:4);\n");
    fprintf(fid,"  plot(f, psd, 'LineWidth',1.5,'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  pmin = 10*floor(0.1*min(psd - 5));\n");
    fprintf(fid,"  pmax = 10*ceil (0.1*max(psd + 5));\n");
    fprintf(fid,"  axis([-0.5 0.5 pmin pmax]);\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('Power Spectral Density [dB]');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    printf("done.\n");
    return 0;
}
