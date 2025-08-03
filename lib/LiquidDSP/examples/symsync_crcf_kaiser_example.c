//
// symsync_crcf_kaiser_example.c
//
// This is a simplified example of the symync family of objects to show how
// symbol timing can be recovered after the matched filter output.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symsync_crcf_kaiser_example.m"

// print usage/help message
void usage(void);
void usage(void)
{
    printf("symsync_crcf_example [options]\n");
    printf(" -h            : print help\n");
    printf(" -k <samp/sym> : filter samples/symbol,   default:   2\n");
    printf(" -m <delay>    : filter delay (symbols),  default:   3\n");
    printf(" -b <beta>     : filter excess bandwidth, default:   0.5\n");
    printf(" -B <n>        : filter polyphase banks,  default:  32\n");
    printf(" -s <snr>      : signal-to-noise ratio,   default:  30 dB\n");
    printf(" -w <bw>       : timing pll bandwidth,    default:   0.02\n");
    printf(" -n <n>        : number of symbols,       default: 400\n");
    printf(" -t <tau>      : timing offset,           default:  -0.2\n");
}

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // options
    unsigned int k           =   2;     // samples/symbol (input)
    unsigned int m           =   3;     // filter delay (symbols)
    float        beta        =   0.5f;  // filter excess bandwidth factor
    unsigned int num_filters =  32;     // number of filters in the bank
    float        SNRdB       =  30.0f;  // signal-to-noise ratio
    float        bt          =   0.02f; // loop filter bandwidth
    unsigned int num_symbols = 400;     // number of data symbols
    float        tau         =  -0.20f; // fractional symbol offset

    // Nyquist filter type
    liquid_firfilt_type ftype = LIQUID_FIRFILT_KAISER;
    
    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:b:B:s:w:n:t:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                        return 0;
        case 'k':   k           = atoi(optarg);     break;
        case 'm':   m           = atoi(optarg);     break;
        case 'b':   beta        = atof(optarg);     break;
        case 'B':   num_filters = atoi(optarg);     break;
        case 's':   SNRdB       = atof(optarg);     break;
        case 'w':   bt          = atof(optarg);     break;
        case 'n':   num_symbols = atoi(optarg);     break;
        case 't':   tau         = atof(optarg);     break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // validate input
    if (k < 2) {
        fprintf(stderr,"error: k (samples/symbol) must be at least 2\n");
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: m (filter delay) must be greater than 0\n");
        exit(1);
    } else if (beta <= 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: beta (excess bandwidth factor) must be in (0,1]\n");
        exit(1);
    } else if (num_filters == 0) {
        fprintf(stderr,"error: number of polyphase filters must be greater than 0\n");
        exit(1);
    } else if (bt <= 0.0f) {
        fprintf(stderr,"error: timing PLL bandwidth must be greater than 0\n");
        exit(1);
    } else if (num_symbols == 0) {
        fprintf(stderr,"error: number of symbols must be greater than 0\n");
        exit(1);
    } else if (tau < -1.0f || tau > 1.0f) {
        fprintf(stderr,"error: timing phase offset must be in [-1,1]\n");
        exit(1);
    }

    // derived values
    unsigned int num_samples = k*num_symbols;
    float complex x[num_samples];           // interpolated samples
    float complex y[num_samples];           // received signal (with noise)
    float         tau_hat[num_samples];     // instantaneous timing offset estimate
    float complex sym_out[num_symbols + 64];// synchronized symbols

    // create sequence of Nyquist-interpolated QPSK symbols
    firinterp_crcf interp = firinterp_crcf_create_prototype(ftype,k,m,beta,tau);
    for (i=0; i<num_symbols; i++) {
        // generate random QPSK symbol
        float complex s = ( rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2 ) +
                          ( rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2 ) * _Complex_I;

        // interpolate symbol
        firinterp_crcf_execute(interp, s, &x[i*k]);
    }
    firinterp_crcf_destroy(interp);


    // add noise
    float nstd = powf(10.0f, -SNRdB/20.0f);
    for (i=0; i<num_samples; i++)
        y[i] = x[i] + nstd*(randnf() + _Complex_I*randnf());


    // create and run symbol synchronizer
    symsync_crcf decim = symsync_crcf_create_kaiser(k, m, beta, num_filters);
    symsync_crcf_set_lf_bw(decim,bt);   // set loop filter bandwidth

    // NOTE: we could just synchronize entire block (see following line);
    //       however we would like to save the instantaneous timing offset
    //       estimate for plotting purposes
    //symsync_crcf_execute(d, y, num_samples, sym_out, &num_symbols_sync);

    unsigned int num_symbols_sync = 0;
    unsigned int num_written=0;
    for (i=0; i<num_samples; i++) {
        // save instantaneous timing offset estimate
        tau_hat[i] = symsync_crcf_get_tau(decim);

        // execute one sample at a time
        symsync_crcf_execute(decim, &y[i], 1, &sym_out[num_symbols_sync], &num_written);

        // increment number of symbols synchronized
        num_symbols_sync += num_written;
    }
    symsync_crcf_destroy(decim);

    // print last several symbols to screen
    printf("output symbols:\n");
    for (i=num_symbols_sync-10; i<num_symbols_sync; i++)
        printf("  sym_out(%2u) = %8.4f + j*%8.4f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));


    //
    // export output file
    //
    FILE* fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"close all;\nclear all;\n\n");

    fprintf(fid,"k=%u;\n",k);
    fprintf(fid,"m=%u;\n",m);
    fprintf(fid,"beta=%12.8f;\n",beta);
    fprintf(fid,"num_filters=%u;\n",num_filters);
    fprintf(fid,"num_symbols=%u;\n",num_symbols);

    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        
    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        
    for (i=0; i<num_samples; i++)
        fprintf(fid,"tau_hat(%3u) = %12.8f;\n", i+1, tau_hat[i]);
        
    for (i=0; i<num_symbols_sync; i++)
        fprintf(fid,"sym_out(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));
        
    fprintf(fid,"i0 = 1:round( 0.5*num_symbols );\n");
    fprintf(fid,"i1 = round( 0.5*num_symbols ):num_symbols;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(real(sym_out(i0)),imag(sym_out(i0)),'x','MarkerSize',4,'Color',[0.6 0.6 0.6]);\n");
    fprintf(fid,"plot(real(sym_out(i1)),imag(sym_out(i1)),'o','MarkerSize',4,'Color',[0 0.25 0.5]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.6);\n");
    fprintf(fid,"xlabel('In-phase');\n");
    fprintf(fid,"ylabel('Quadrature');\n");
    fprintf(fid,"legend(['first 50%%'],['last 50%%'],1);\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"tt = 0:(length(tau_hat)-1);\n");
    fprintf(fid,"b = floor(num_filters*tau_hat + 0.5);\n");
    fprintf(fid,"stairs(tt,tau_hat*num_filters);\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(tt,b,'-k','Color',[0 0 0]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('filterbank index');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([0 length(tau_hat) -1 num_filters]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    printf("done.\n");
    return 0;
}
