#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "liquid.h"

int main()
{
    // create mod/demod objects
    modulation_scheme ms = LIQUID_MODEM_QAM16;

    // create the modem objects
    modemcf mod   = modemcf_create(ms);
    modemcf demod = modemcf_create(ms);

    // ensure bits/symbol matches modem description (only
    // applicable to certain specific modems)
    unsigned int bps = modemcf_get_bps(mod);
    modemcf_print(mod);

    unsigned int i; // modulated symbol
    unsigned int s; // demodulated symbol
    unsigned int num_symbols = 1<<bps;
    float complex x;
    unsigned int num_sym_errors = 0;
    unsigned int num_bit_errors = 0;
    for (i=0; i<num_symbols; i++) {
        modemcf_modulate(mod, i, &x);
        modemcf_demodulate(demod, x, &s);

        printf("%4u : %12.8f + j*%12.8f\n", i, crealf(x), cimagf(x));

        num_sym_errors += i == s ? 0 : 1;
        num_bit_errors += count_bit_errors(i,s);
    }
    printf("num sym errors: %4u / %4u\n", num_sym_errors, num_symbols);
    printf("num bit errors: %4u / %4u\n", num_bit_errors, num_symbols*bps);

    modemcf_destroy(mod);
    modemcf_destroy(demod);
    return 0;
}
