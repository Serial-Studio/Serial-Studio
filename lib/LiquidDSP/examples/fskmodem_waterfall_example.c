// 
// fskmodem_waterfall_example.c
//
// This example demonstrates the M-ary frequency-shift keying
// (MFSK) modem in liquid by showing the resulting spectral
// waterfall.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

// print usage/help message
void usage()
{
    printf("fskmodem_waterfall_example -- frequency-shift keying waterfall example\n");
    printf("options:\n");
    printf("  -h       : print help\n");
    printf("  -m <bps> : bits/symbol,            default:  2\n");
    printf("  -b <bw>  : signal bandwidth        default:  0.2\n");
    printf("  -n <num> : number of data symbols, default: 80\n");
    printf("  -s <snr> : SNR [dB],               default: 40\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int m           =     2;   // number of bits/symbol
    unsigned int num_symbols =   400;   // number of data symbols
    float        SNRdB       = 30.0f;   // signal-to-noise ratio [dB]
    float        bandwidth   =  0.10;   // frequency spacing

    int dopt;
    while ((dopt = getopt(argc,argv,"hm:b:n:s:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'm': m           = atoi(optarg);   break;
        case 'b': bandwidth   = atof(optarg);   break;
        case 'n': num_symbols = atoi(optarg);   break;
        case 's': SNRdB       = atof(optarg);   break;
        default:
            exit(1);
        }
    }

    unsigned int i;
    unsigned int j;

    // derived values
    unsigned int M    = 1 << m;     // constellation size
    unsigned int k    = 500 * M;    // samples per symbol (highly over-sampled)
    float        nstd = powf(10.0f, -SNRdB/20.0f);  // noise std. dev.

    // validate input
    if (k < M) {
        fprintf(stderr,"errors: %s, samples/symbol must be at least modulation size (M=%u)\n", __FILE__,M);
        exit(1);
    } else if (k > 2048) {
        fprintf(stderr,"errors: %s, samples/symbol exceeds maximum (2048)\n", __FILE__);
        exit(1);
    } else if (M > 1024) {
        fprintf(stderr,"errors: %s, modulation size (M=%u) exceeds maximum (1024)\n", __FILE__, M);
        exit(1);
    } else if (bandwidth <= 0.0f || bandwidth >= 0.5f) {
        fprintf(stderr,"errors: %s, bandwidth must be in (0,0.5)\n", __FILE__);
        exit(1);
    }

    // create spectral waterfall object
    unsigned int nfft  = 1 << liquid_nextpow2(k);
    int          wtype = LIQUID_WINDOW_HAMMING;
    unsigned int wlen  = nfft/2;
    unsigned int delay = nfft/2;
    unsigned int time  =    512;
    spwaterfallcf periodogram = spwaterfallcf_create(nfft,wtype,wlen,delay,time);
    spwaterfallcf_print(periodogram);

    // create modulator/demodulator pair
    fskmod mod = fskmod_create(m,k,bandwidth);

    float complex buf_tx[k];    // transmit buffer
    float complex buf_rx[k];    // transmit buffer
    
    // modulate, demodulate, count errors
    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        unsigned int sym_in = rand() % M;

        // modulate
        fskmod_modulate(mod, sym_in, buf_tx);

        // add noise
        for (j=0; j<k; j++)
            buf_rx[j] = buf_tx[j] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // estimate power spectral density
        spwaterfallcf_write(periodogram, buf_rx, k);
    }

    // export output files
    spwaterfallcf_export(periodogram,"fskmodem_waterfall_example");

    // destroy objects
    spwaterfallcf_print(periodogram);
    fskmod_destroy(mod);

    return 0;
}
