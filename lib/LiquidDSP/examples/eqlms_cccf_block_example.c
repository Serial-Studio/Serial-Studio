// 
// eqlms_cccf_block_example.c
//
// This example tests the least mean-squares (LMS) equalizer (EQ) on a
// signal with an unknown modulation and carrier frequency offset.
// Equalization is performed blind on a block of samples and the reulting
// constellation is output to a file for plotting.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <getopt.h>
#include <time.h>
#include "liquid.h"

#define OUTPUT_FILENAME "eqlms_cccf_block_example.m"

// print usage/help message
void usage()
{
    printf("Usage: eqlms_cccf_block_example [OPTION]\n");
    printf("  h     : print help\n");
    printf("  n     : number of symbols, default: 500\n");
    printf("  c     : number of channel filter taps (minimum: 1), default: 5\n");
    printf("  k     : samples/symbol, default: 2\n");
    printf("  m     : filter semi-length (symbols), default: 4\n");
    printf("  b     : filter excess bandwidth factor, default: 0.3\n");
    printf("  p     : equalizer semi-length (symbols), default: 3\n");
    printf("  u     : equalizer learning rate, default; 0.05\n");
}

int main(int argc, char*argv[])
{
    //srand(time(NULL));

    // options
    unsigned int    num_samples = 2400;     // number of symbols to observe
    unsigned int    hc_len      = 5;        // channel filter length
    unsigned int    k           = 2;        // matched filter samples/symbol
    unsigned int    m           = 3;        // matched filter delay (symbols)
    float           beta        = 0.3f;     // matched filter excess bandwidth factor
    unsigned int    p           = 3;        // equalizer length (symbols, hp_len = 2*k*p+1)
    float           mu          = 0.15f;    // equalizer learning rate
    modulation_scheme ms        = LIQUID_MODEM_QPSK;

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:c:k:m:b:p:u:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'n': num_samples   = atoi(optarg); break;
        case 'c': hc_len        = atoi(optarg); break;
        case 'k': k             = atoi(optarg); break;
        case 'm': m             = atoi(optarg); break;
        case 'b': beta          = atof(optarg); break;
        case 'p': p             = atoi(optarg); break;
        case 'u': mu            = atof(optarg); break;
        default:
            exit(1);
        }
    }
    unsigned int i;

    // validate input
    if (num_samples == 0) {
        fprintf(stderr,"error: %s, number of symbols must be greater than zero\n", argv[0]);
        exit(1);
    } else if (hc_len == 0) {
        fprintf(stderr,"error: %s, channel must have at least 1 tap\n", argv[0]);
        exit(1);
    } else if (k < 2) {
        fprintf(stderr,"error: %s, samples/symbol must be at least 2\n", argv[0]);
        exit(1);
    } else if (m == 0) {
        fprintf(stderr,"error: %s, filter semi-length must be at least 1 symbol\n", argv[0]);
        exit(1);
    } else if (beta < 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: %s, filter excess bandwidth must be in [0,1]\n", argv[0]);
        exit(1);
    } else if (p == 0) {
        fprintf(stderr,"error: %s, equalizer semi-length must be at least 1 symbol\n", argv[0]);
        exit(1);
    } else if (mu < 0.0f || mu > 1.0f) {
        fprintf(stderr,"error: %s, equalizer learning rate must be in [0,1]\n", argv[0]);
        exit(1);
    }

    // derived/fixed values
    unsigned int    buf_len = 37;
    float complex   buf_input  [buf_len];
    float complex   buf_channel[buf_len];
    float complex   buf_output [buf_len];

    // 
    // generate input sequence using symbol stream generator
    //
    symstreamcf gen = symstreamcf_create_linear(LIQUID_FIRFILT_ARKAISER,k,m,beta,ms);

    //
    // create multi-path channel filter
    //
    float complex hc[hc_len];
    for (i=0; i<hc_len; i++)
        hc[i] = (i==0) ? 0.5f : (randnf() + _Complex_I*randnf())*0.2f;
    firfilt_cccf channel = firfilt_cccf_create(hc, hc_len);

    //
    // create equalizer
    //
    eqlms_cccf eq = eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_RRC, k, p, beta, 0.0f);
    eqlms_cccf_set_bw(eq, mu);

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"x = [];\n");
    fprintf(fid,"y = [];\n");
    fprintf(fid,"z = [];\n");

    unsigned int num_samples_total = 0;
    while (num_samples_total < num_samples)
    {
        // generate block of input samples
        symstreamcf_write_samples(gen, buf_input, buf_len);

        // apply channel to input signal
        firfilt_cccf_execute_block(channel, buf_input, buf_len, buf_channel);

        // run equalizer
        eqlms_cccf_execute_block(eq, k, buf_channel, buf_len, buf_output);
        
        // save results to output file
        for (i=0; i<buf_len; i++) {
            fprintf(fid,"x(end+1) = %12.4e + %12.4ei;\n", crealf(buf_input  [i]), cimagf(buf_input  [i]));
            fprintf(fid,"y(end+1) = %12.4e + %12.4ei;\n", crealf(buf_channel[i]), cimagf(buf_channel[i]));
            fprintf(fid,"z(end+1) = %12.4e + %12.4ei;\n", crealf(buf_output [i]), cimagf(buf_output [i]));
        }

        // increment number of samples
        num_samples_total += buf_len;
    }

    // destroy objects
    symstreamcf_destroy(gen);
    firfilt_cccf_destroy(channel);
    eqlms_cccf_destroy(eq);

    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"n = length(x);\n");

    fprintf(fid,"figure('Color','white','position',[500 500 1200 500]);\n");
    // plot constellation
    fprintf(fid,"subplot(2,8,[5 6 7 8 13 14 15 16]),\n");
    fprintf(fid,"  s = 1:k:n;\n");
    fprintf(fid,"  s0 = round(length(s)/2);\n");
    fprintf(fid,"  syms_rx_0 = z(s(1:s0));\n");
    fprintf(fid,"  syms_rx_1 = z(s(s0:end));\n");
    fprintf(fid,"  plot(real(syms_rx_0),imag(syms_rx_0),'x','Color',[1 1 1]*0.7,...\n");
    fprintf(fid,"       real(syms_rx_1),imag(syms_rx_1),'x','Color',[1 1 1]*0.0);\n");
    fprintf(fid,"  xlabel('In-Phase');\n");
    fprintf(fid,"  ylabel('Quadrature');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    // plot time response
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"subplot(2,8,1:4),\n");
    fprintf(fid,"  plot(t,   real(z),   '-','Color',[1 1 1]*0.7,...\n");
    fprintf(fid,"       t(s),real(z(s)),'s','Color',[0 0.2 0.5],'MarkerSize',3);\n");
    fprintf(fid,"  axis([0 n -1.5 1.5]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('real');\n");
    fprintf(fid,"subplot(2,8,9:12),\n");
    fprintf(fid,"  plot(t,   imag(z),   '-','Color',[1 1 1]*0.7,...\n");
    fprintf(fid,"       t(s),imag(z(s)),'s','Color',[0 0.5 0.2],'MarkerSize',3);\n");
    fprintf(fid,"  axis([0 n -1.5 1.5]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('imag');\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
