//
// firdes_energy_test.c
//

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "liquid.h"

// print usage/help message
void usage()
{
    printf("firdes_energy_test:\n");
    printf("  u/h   : print usage/help\n");
    printf("  f     : filter cutoff frequency,           0 < f < 0.5, default: 0.2\n");
    printf("  t     : filter transition bandwidth,       0 < t < 0.5, default: 0.1\n");
    printf("  s     : filter stop-band attenuation [dB], 0 < s,       default: 60\n");
    printf("  m     : fractional sample delay,        -0.5 < m < 0.5, default: 0\n");
}

int main(int argc, char*argv[]) {
    // options
    float fc=0.2f;          // filter cutoff frequency
    float ft=0.1f;          // filter transition
    float As=60.0f;         // stop-band attenuation [dB]
    float mu=0.0f;          // fractional timing offset

    int dopt;
    while ((dopt = getopt(argc,argv,"uhf:t:s:m:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();                      return 0;
        case 'f': fc = atof(optarg);            break;
        case 't': ft = atof(optarg);            break;
        case 's': As = atof(optarg);            break;
        case 'm': mu = atof(optarg);            break;
        default:
            exit(1);
        }
    }

    // derived values
    unsigned int h_len = estimate_req_filter_len(ft,As);
    printf("h_len : %u\n", h_len);

    printf("filter design parameters\n");
    printf("    cutoff frequency            :   %8.4f\n", fc);
    printf("    transition bandwidth        :   %8.4f\n", ft);
    printf("    stop-band attenuation [dB]  :   %8.4f\n", As);
    printf("    fractional sample offset    :   %8.4f\n", mu);
    printf("    filter length               :   %u\n", h_len);

    // generate the filter
    unsigned int i;
    float h[h_len];
    liquid_firdes_kaiser(h_len,fc,As,mu,h);

    // print coefficients
    for (i=0; i<h_len; i++)
        printf("h(%4u) = %16.12f;\n", i+1, h[i]);

    // compute energy test
    unsigned int nfft = 1024;
#if 0
    float fs = fc + 0.5f*ft;    // filter stop-band
    float r = liquid_filter_energy(h,h_len,fs,nfft);
    printf("r = %f\n", 10*log10f(r));
#else
    // in-band energy at filter cut-off
    float fs = fc;    // filter stop-band
    float r = liquid_filter_energy(h,h_len,fs,nfft);
    printf(" in-band energy : %8.3f %%\n", (1-r)*100.0f);
#endif


    printf("done.\n");
    return 0;
}

