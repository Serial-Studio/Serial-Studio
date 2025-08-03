// This example demonstrates the functionality of the arbitrary
// modem, a digital modulator/demodulator object with signal
// constellation points chosen arbitrarily.  A simple bit-error
// rate simulation is then run to test the performance of the
// modem.  The results are written to a file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "modem_arb_example.m"

// print usage/help message
void usage()
{
    printf("modem_arb_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  p     : modulation depth (default 4 bits/symbol)\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int bps=6;         // bits per symbol
    unsigned int n=1024;        // number of data points to evaluate

    int dopt;
    while ((dopt = getopt(argc,argv,"uhp:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage(); return 0;
        case 'p': bps = atoi(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (bps == 0) {
        fprintf(stderr,"error: %s, bits/symbol must be greater than zero\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int i;
    unsigned int M = 1<<bps;    // constellation size

    // initialize constellation table
    float complex constellation[M];
    // initialize constellation (spiral)
    for (i=0; i<M; i++) {
        float r   = (float)i / logf((float)M) + 4.0f;
        float phi = (float)i / logf((float)M);
        constellation[i] = r * cexpf(_Complex_I*phi);
    }
    
    // create mod/demod objects
    modemcf mod   = modemcf_create_arbitrary(constellation, M);
    modemcf demod = modemcf_create_arbitrary(constellation, M);

    modemcf_print(mod);

    // run simulation
    float complex x[n];
    unsigned int num_errors = 0;

    // run simple BER simulation
    num_errors = 0;
    unsigned int sym_in;
    unsigned int sym_out;
    for (i=0; i<n; i++) {
        // generate and modulate random symbol
        sym_in = modemcf_gen_rand_sym(mod);
        modemcf_modulate(mod, sym_in, &x[i]);

        // add noise
        x[i] += 0.05 * randnf() * cexpf(_Complex_I*M_PI*randf());

        // demodulate
        modemcf_demodulate(demod, x[i], &sym_out);

        // accumulate errors
        num_errors += count_bit_errors(sym_in,sym_out);
    }
    printf("num bit errors: %4u / %4u\n", num_errors, bps*n);

    // destroy modem objects
    modemcf_destroy(mod);
    modemcf_destroy(demod);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"bps = %u;\n", bps);
    fprintf(fid,"M = %u;\n", M);

    for (i=0; i<n; i++) {
        fprintf(fid,"x(%3u) = %12.4e + j*%12.4e;\n", i+1,
                                                     crealf(x[i]),
                                                     cimagf(x[i]));
    }

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(x,'x','MarkerSize',1);\n");
    fprintf(fid,"xlabel('in-phase');\n");
    fprintf(fid,"ylabel('quadrature phase');\n");
    fprintf(fid,"title(['Arbitrary ' num2str(M) '-QAM']);\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.9);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);

    printf("results written to '%s'\n", OUTPUT_FILENAME);
    printf("done.\n");

    return 0;
}
