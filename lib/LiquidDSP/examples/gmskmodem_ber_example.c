// This example runs a bit error rate simulation for GMSK modulation.
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

// print usage/help message
void usage()
{
    printf("modem_example [options]\n");
    printf(" -h          : print help\n");
}

int main(int argc, char*argv[])
{
    // simulation parameters
    unsigned int k          = 4;        // filter samples/symbol
    unsigned int m          = 3;        // filter delay (symbols)
    float BT                = 0.3f;     // bandwidth-time product
    float SNRdB_min         = -5.0f;    // starting SNR value
    float SNRdB_max         = 15.0f;    // maximum SNR value
    float SNRdB_step        =  1.0f;    // step size
    unsigned int num_trials = 1e6;      // number of symbols
    char filename[]         = "gmskmodem_ber_example.m";

    int dopt;
    while ((dopt = getopt(argc,argv,"h")) != EOF) {
        switch (dopt) {
        case 'h':   usage();        return 0;
        default:
            exit(1);
        }
    }

    // create modem objects
    gmskmod mod = gmskmod_create(k, m, BT); // modulator
    gmskdem dem = gmskdem_create(k, m, BT); // demodulator

    // derived values and buffers
    unsigned int delay = 2*m;
    unsigned int symbol_buffer[delay];      // delay between tx/rx
    unsigned int symbol_index = 0;
    float complex buf[k];   // sample buffer
    unsigned int  i, j, sym_in, sym_out, sym_buf;

    // iterate through SNR values
    printf("# GMSK, k=%u, m=%u, BT=%.3f\n", k, m, BT);
    printf("# %8s %8s %8s %12s %12s\n", "SNR [dB]", "errors", "trials", "BER", "theory");
    float SNRdB = SNRdB_min;    // set SNR value
    FILE * fid = fopen(filename,"w");
    fprintf(fid,"%% %s : auto-generated file\n", filename);
    fprintf(fid,"clear all; close all; SNRdB=[]; BER=[]; theory=[];\n");
    while (1)
    {
        // compute noise standard deviation, compensating for over-sampling
        float nstd = powf(10.0f, -SNRdB/20.0f) * sqrtf((float)k);

        // reset modem objects (only necessary for differential schemes)
        gmskmod_reset(mod);
        gmskdem_reset(dem);

        // run trials
        unsigned int num_bit_errors = 0;
        for (i=0; i<num_trials + delay; i++) {
            // pull old symbol from buffer
            sym_buf = symbol_buffer[symbol_index];

            // generate random input symbol and modulate
            sym_in = rand() & 1;
            gmskmod_modulate(mod, sym_in, buf);

            // store symbol in buffer to account for delay
            symbol_buffer[symbol_index] = sym_in;
            symbol_index = (symbol_index + 1) % delay;

            // add complex noise and demodulate
            for (j=0; j<k; j++)
                buf[j] += nstd*(randnf() + _Complex_I*randnf())/sqrtf(2.0f);
            gmskdem_demodulate(dem, buf, &sym_out);

            // count errors only after delay
            if (i > delay)
                num_bit_errors += count_bit_errors(sym_buf, sym_out);
        }

        // compute results and print formatted results to screen
        unsigned int num_bit_trials = m * num_trials;
        float ber = (float)num_bit_errors / (float)(num_bit_trials);
        float gamma = powf(10.0f, SNRdB/10.0f);
        float theory = liquid_Qf(sqrtf(2*gamma));
        printf("  %8.2f %8u %8u %12.4e %12.4e\n", SNRdB, num_bit_errors, num_bit_trials, ber, theory);

        if (num_bit_errors > 0) {
            fprintf(fid,"SNRdB(end+1)=%12.3f; BER(end+1)=%12.4e; theory(end+1)=%12.4e;\n",
                SNRdB, ber, theory);
        }

        // stop iterating if SNR exceed maximum or no errors were detected
        SNRdB += SNRdB_step;
        if (SNRdB > SNRdB_max)
            break;
    }
    fprintf(fid,"figure; semilogy(SNRdB,BER,'-x',SNRdB,theory,'-x');grid on;\n");
    fprintf(fid,"axis([-5 15 1e-6 1]);xlabel('SNR [dB]'); ylabel('BER'); legend('sim','theory');\n");
    fclose(fid);
    printf("results written to %s\n", filename);

    // destroy modem objects and return
    gmskmod_destroy(mod);
    gmskdem_destroy(dem);
    return 0;
}
