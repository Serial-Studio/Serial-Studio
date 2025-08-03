// Demonstrate pi/4 differential QPSK modem.
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "modem_pi4dqpsk_example.m"

int main()
{
    // options
    float        SNRdB       = 25.0f;
    unsigned int num_symbols = 800;

    // create the modem objects
    modemcf mod   = modemcf_create(LIQUID_MODEM_PI4DQPSK);
    modemcf demod = modemcf_create(LIQUID_MODEM_PI4DQPSK);
    modemcf_print(mod);

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"n=%u; sym_rx=zeros(1,n); sym_rec=zeros(1,n);\n", num_symbols);

    unsigned int i, index, s;
    float complex sym_tx, sym_rx;
    unsigned int num_sym_errors = 0, num_bit_errors = 0;
    float nstd = powf(10.0f, -SNRdB/20.0f);
    float r_prime = 0.0f;
    float complex s_prime;
    for (i=0; i<num_symbols; i++) {
        // modulate symbol
        index = rand() & 3;
        modemcf_modulate(mod, index, &sym_tx);

        // add noise
        sym_rx = sym_tx + nstd*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;

        // demodulate using object
        modemcf_demodulate(demod, sym_rx, &s);

        // custom demod to show QPSK constellation
        s_prime = sym_rx * cexpf(-_Complex_I*r_prime);
        r_prime = cargf(sym_rx);

        // accumulate errors
        num_sym_errors += index != s;
        num_bit_errors += count_bit_errors(index,s);

        // write symbol to output file
        fprintf(fid,"sym_rx (%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(sym_rx), cimagf(sym_rx));
        fprintf(fid,"sym_rec(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(s_prime), cimagf(s_prime));
    }
    printf("num sym errors: %4u / %4u\n", num_sym_errors, num_symbols);
    printf("num bit errors: %4u / %4u\n", num_bit_errors, num_symbols*2);
    modemcf_destroy(mod);
    modemcf_destroy(demod);

    // plot results
    fprintf(fid,"\n\n");
    fprintf(fid,"figure('position',[100 100 800 400])\n");
    fprintf(fid,"subplot(1,2,1),");
    fprintf(fid,"  plot(real(sym_rx),imag(sym_rx),'o','MarkerSize',2);\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('in phase');\n");
    fprintf(fid,"  ylabel('quadrature phase');\n");
    fprintf(fid,"  title('Received');\n");
    fprintf(fid,"subplot(1,2,2),");
    fprintf(fid,"  plot(real(sym_rec),imag(sym_rec),'o','MarkerSize',2);\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('in phase');\n");
    fprintf(fid,"  ylabel('quadrature phase');\n");
    fprintf(fid,"  title('De-rotated');\n");
    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    return 0;
}
