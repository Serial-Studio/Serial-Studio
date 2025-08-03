// Run recursive QPSK modulation/demodulation test.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "recursive_qpsk_test.m"

// print usage/help message
void usage()
{
    printf("%s [options]\n", __FILE__);
    printf("  -h        : print usage\n");
    printf("  -b<beta>  : beta value for constellation, beta in (0,1)\n");
}

int main(int argc, char*argv[])
{
    // create mod/demod objects
    float           beta        = 0.25f;
    float           SNRdB_min   = 0.0f;
    float           SNRdB_max   = 30.0f;
    unsigned int    num_snr     = 31;
    unsigned int    num_trials  = 1e6;

    int dopt;
    while ((dopt = getopt(argc,argv,"hb:")) != EOF) {
        switch (dopt) {
        case 'h':   usage();                return 0;
        case 'b':   beta = atof(optarg);    break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    float complex map_qpsk[4] = {
        M_SQRT1_2 + _Complex_I*M_SQRT1_2,   // 00
       -M_SQRT1_2 + _Complex_I*M_SQRT1_2,   // 01
        M_SQRT1_2 - _Complex_I*M_SQRT1_2,   // 10
       -M_SQRT1_2 - _Complex_I*M_SQRT1_2,   // 11
    };

    // generate composite constellation
    float complex map[16];
    for (i=0; i<4; i++) {
        map[4*i+0] = map_qpsk[i] + beta*map_qpsk[0];
        map[4*i+1] = map_qpsk[i] + beta*map_qpsk[1];
        map[4*i+2] = map_qpsk[i] + beta*map_qpsk[2];
        map[4*i+3] = map_qpsk[i] + beta*map_qpsk[3];
    }

    // create arbitrary modem
    modemcf mod = modemcf_create_arbitrary(map, 16);

    // open file for writing
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    unsigned int    sym_tx_0;
    unsigned int    sym_tx_1;
    unsigned int    sym_tx;
    float complex   s;
    unsigned int    sym_rx;
    unsigned int    sym_rx_0;
    unsigned int    sym_rx_1;
    unsigned int    num_bit_errors_0 = 0;
    unsigned int    num_bit_errors_1 = 0;

    // print constellation
    fprintf(fid,"map = zeros(1,16);\n");
    for (i=0; i<16; i++) {
        modemcf_modulate(mod, i, &s);
        fprintf(fid,"map(%2u) = %12.4e + %12.4ei;\n", i+1, crealf(s), cimagf(s));
    }

    unsigned int n;
    float complex SNRdB_step = (SNRdB_max - SNRdB_min) / (num_snr - 1);
    for (n=0; n<num_snr; n++) {
        float SNRdB = SNRdB_min + n*SNRdB_step;
        float nstd  = powf(10.0f, -SNRdB/20.0f);

        num_bit_errors_0 = 0;
        num_bit_errors_1 = 0;

        for (i=0; i<num_trials; i++) {
            // generate independent streams, each 2 bits long
            sym_tx_0 = rand() & 0x3;
            sym_tx_1 = rand() & 0x3;

            // composite
            sym_tx = (sym_tx_0 << 2) | sym_tx_1;

            // modulate
            modemcf_modulate(mod, sym_tx, &s);
            
            // add noise
            s += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

            // demodulate
            modemcf_demodulate(mod, s, &sym_rx);

            // recover streams
            sym_rx_0 = (sym_rx >> 2) & 0x3;
            sym_rx_1 = (sym_rx     ) & 0x3;

            // accumulate errors
            num_bit_errors_0 += count_bit_errors(sym_tx_0, sym_rx_0);
            num_bit_errors_1 += count_bit_errors(sym_tx_1, sym_rx_1);
        }
        float BER_QPSK = 0.5f*erfcf(powf(10.0f,SNRdB/20.0f)*M_SQRT1_2);
        float BER_0    = num_bit_errors_0 / (float)(2*num_trials);
        float BER_1    = num_bit_errors_1 / (float)(2*num_trials);

        // print results to screen
        printf("%3u : SNR = %6.3f dB, {%8u %8u} / %8u, QPSK: %12.4e, BER : {%12.4e %12.4e}\n",
                n, SNRdB, num_bit_errors_0, num_bit_errors_1, 2*num_trials, BER_QPSK, BER_0, BER_1);

        // print results to file
        fprintf(fid,"SNRdB(%3u)=%6.3f; BER_QPSK(%3u)=%12.4e; BER_0(%3u)=%12.4e; BER_1(%3u)=%12.4e;\n",
                n+1, SNRdB, n+1, BER_QPSK, n+1, BER_0, n+1, BER_1);
    }
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(SNRdB,BER_QPSK+1e-12,SNRdB,BER_0+1e-12,SNRdB,BER_1+1e-12);\n");
    fprintf(fid,"axis([%.3f %.3f 1e-5 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('SNR [dB]');\n");
    fprintf(fid,"ylabel('BER');\n");
    fprintf(fid,"legend('qpsk  ', 'stream 0  ', 'stream 1  ', 'location', 'northeast');\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    // destroy modem object
    modemcf_destroy(mod);

    printf("done.\n");
    return 0;
}

