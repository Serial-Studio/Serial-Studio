// 
// cpfskmodem_example.c
//
// This example demonstrates the continuous phase frequency-shift keying
// (CP-FSK) modem in liquid. A message signal is modulated and the
// resulting signal is recovered using a demodulator object.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "cpfskmodem_example.m"

// print usage/help message
void usage()
{
    printf("cpfskmodem_example -- continuous-phase frequency-shift keying example\n");
    printf("options:\n");
    printf(" -h             : print help\n");
    printf(" -t <type>      : filter type: [square], rcos-full, rcos-half, gmsk\n");
    printf(" -p <b/s>       : bits/symbol,              default:  1\n");
    printf(" -H <index>     : modulation index,         default:  0.5\n");
    printf(" -k <s/sym>     : samples/symbol,           default:  8\n");
    printf(" -m <delay>     : filter delay (symbols),   default:  3\n");
    printf(" -b <rolloff>   : filter roll-off,          default:  0.35\n");
    printf(" -n <num>       : number of data symbols,   default: 80\n");
    printf(" -s <snr>       : SNR [dB],                 default: 40\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int    bps         = 1;        // number of bits/symbol
    float           h           = 0.5f;     // modulation index (h=1/2 for MSK)
    unsigned int    k           = 4;        // filter samples/symbol
    unsigned int    m           = 3;        // filter delay (symbols)
    float           beta        = 0.35f;    // GMSK bandwidth-time factor
    unsigned int    num_symbols = 20;       // number of data symbols
    float           SNRdB       = 40.0f;    // signal-to-noise ratio [dB]
    int             filter_type = LIQUID_CPFSK_SQUARE;

    int dopt;
    while ((dopt = getopt(argc,argv,"ht:p:H:k:m:b:n:s:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 't':
            if (strcmp(optarg,"square")==0) {
                filter_type = LIQUID_CPFSK_SQUARE;
            } else if (strcmp(optarg,"rcos-full")==0) {
                filter_type = LIQUID_CPFSK_RCOS_FULL;
            } else if (strcmp(optarg,"rcos-half")==0) {
                filter_type = LIQUID_CPFSK_RCOS_PARTIAL;
            } else if (strcmp(optarg,"gmsk")==0) {
                filter_type = LIQUID_CPFSK_GMSK;
            } else {
                fprintf(stderr,"error: %s, unknown filter type '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        case 'p': bps   = atoi(optarg);         break;
        case 'H': h     = atof(optarg);         break;
        case 'k': k     = atoi(optarg);         break;
        case 'm': m     = atoi(optarg);         break;
        case 'b': beta  = atof(optarg);         break;
        case 'n': num_symbols = atoi(optarg);   break;
        case 's': SNRdB = atof(optarg);         break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int  num_samples = k*num_symbols;
    unsigned int  M           = 1 << bps;
    float         nstd        = powf(10.0f, -SNRdB/20.0f);

    // arrays
    unsigned int  sym_in [num_symbols]; // input symbols
    float complex x      [num_samples]; // transmitted signal
    float complex y      [num_samples]; // received signal
    unsigned int  sym_out[num_symbols]; // output symbols

    // create modem objects
    cpfskmod mod = cpfskmod_create(bps, h, k, m, beta, filter_type);
    cpfskdem dem = cpfskdem_create(bps, h, k, m, beta, filter_type);

    // print modulator
    cpfskmod_print(mod);
    cpfskdem_print(dem);
    
    // get full symbol delay
    unsigned int delay = cpfskmod_get_delay(mod) + cpfskdem_get_delay(dem);
    printf("delay: %u samples\n", delay);

    // generate message signal
    for (i=0; i<num_symbols; i++)
        sym_in[i] = rand() % M;

    // modulate signal
    for (i=0; i<num_symbols; i++)
        cpfskmod_modulate(mod, sym_in[i], &x[k*i]);

    // push through channel (add noise)
    for (i=0; i<num_samples; i++)
        y[i] = x[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

    // demodulate signal
    for (i=0; i<num_symbols; i++)
        sym_out[i] = cpfskdem_demodulate(dem, &y[i*k]);

    // print/count errors
    unsigned int num_errors = 0;
    for (i=delay; i<num_symbols; i++) {
        int is_err = (sym_in[i-delay] == sym_out[i]) ? 0 : 1;
        printf("  %3u : %2u %2u %s\n", i, sym_in[i-delay], sym_out[i], is_err ? "*" : "");
        num_errors += is_err;
    }
    printf("symbol errors: %u / %u\n", num_errors, num_symbols-delay);

    // destroy modem objects
    cpfskmod_destroy(mod);
    cpfskdem_destroy(dem);

    // compute power spectral density of transmitted signal
    unsigned int nfft = 1024;
    float psd[nfft];
    spgramcf_estimate_psd(nfft, x, num_samples, psd);

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"h = %f;\n", h);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"nfft        = %u;\n", nfft);
    fprintf(fid,"delay       = %u; %% receive filter delay\n", m);

    fprintf(fid,"x   = zeros(1,num_samples);\n");
    fprintf(fid,"y   = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }
    // save power spectral density
    fprintf(fid,"psd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%4u) = %12.8f;\n", i+1, psd[i]);

    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"i = 1:k:num_samples;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,4,1:3);\n");
    fprintf(fid,"  plot(t,real(x),'-', t(i),real(x(i)),'ob',...\n");
    fprintf(fid,"       t,imag(x),'-', t(i),imag(x(i)),'og');\n");
    fprintf(fid,"  axis([0 num_symbols -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('x(t)');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,4,5:7);\n");
    fprintf(fid,"  plot(t,real(y),'-', t(i),real(y(i)),'ob',...\n");
    fprintf(fid,"       t,imag(y),'-', t(i),imag(y(i)),'og');\n");
    fprintf(fid,"  axis([0 num_symbols -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('y(t)');\n");
    fprintf(fid,"  grid on;\n");
    // plot I/Q constellations
    fprintf(fid,"subplot(3,4,4);\n");
    fprintf(fid,"  plot(real(x),imag(x),'-',real(x(i)),imag(x(i)),'rs','MarkerSize',4);\n");
    fprintf(fid,"  xlabel('I');\n");
    fprintf(fid,"  ylabel('Q');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,4,8);\n");
    fprintf(fid,"  plot(real(y),imag(y),'-',real(y(i)),imag(y(i)),'rs','MarkerSize',4);\n");
    fprintf(fid,"  xlabel('I');\n");
    fprintf(fid,"  ylabel('Q');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    // plot PSD
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"subplot(3,4,9:12);\n");
    fprintf(fid,"  plot(f,psd,'LineWidth',1.5);\n");
    fprintf(fid,"  axis([-0.5 0.5 -60 20]);\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fprintf(fid,"  grid on;\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
