// 
// gmskmodem_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "gmskmodem_example.m"

// print usage/help message
void usage()
{
    printf("gmskmodem_example -- Gaussian minimum-shift keying modem example\n");
    printf("options (default values in <>):\n");
    printf("  u/h   : print usage/help\n");
    printf("  k     : samples/symbol, default: 4\n");
    printf("  m     : filter delay [symbols], default: 3\n");
    printf("  n     : number of data symbols, default: 200\n");
    printf("  b     : bandwidth-time product, 0 <= b <= 1, default: 0.3\n");
    printf("  s     : SNR [dB], default: 30\n");
}

int main(int argc, char*argv[]) {
    // options
    unsigned int k=4;                   // filter samples/symbol
    unsigned int m=3;                   // filter delay (symbols)
    float BT=0.3f;                      // bandwidth-time product
    unsigned int num_data_symbols=200;  // number of data symbols
    float SNRdB = 30.0f;                // signal-to-noise ratio [dB]
    float phi = 0.0f;                   // carrier phase offset
    float dphi = 0.0f;                  // carrier frequency offset

    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:n:b:s:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();              return 0;
        case 'k': k = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'n': num_data_symbols = atoi(optarg); break;
        case 'b': BT = atof(optarg); break;
        case 's': SNRdB = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (BT <= 0.0f || BT >= 1.0f) {
        fprintf(stderr,"error: %s, bandwidth-time product must be in (0,1)\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int num_symbols = num_data_symbols + 2*m;
    unsigned int num_samples = k*num_symbols;
    float nstd = powf(10.0f,-SNRdB/20.0f);  // noise standard deviation

    // create modulator
    gmskmod mod   = gmskmod_create(k, m, BT);
    gmskmod_print(mod);

    // create demodulator
    gmskdem demod = gmskdem_create(k, m, BT);
    gmskdem_set_eq_bw(demod, 0.01f);
    gmskdem_print(demod);

    unsigned int i;
    unsigned int s[num_symbols];
    float complex x[num_samples];
    float complex y[num_samples];
    unsigned int sym_out[num_symbols];

    // generate random data sequence
    for (i=0; i<num_symbols; i++)
        s[i] = rand() % 2;

    // modulate signal
    for (i=0; i<num_symbols; i++)
        gmskmod_modulate(mod, s[i], &x[k*i]);

    // add channel impairments
    for (i=0; i<num_samples; i++) {
        y[i]  = x[i]*cexpf(_Complex_I*(phi + i*dphi));
        y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }

    // demodulate signal
    for (i=0; i<num_symbols; i++)
        gmskdem_demodulate(demod, &y[k*i], &sym_out[i]);

    // destroy modem objects
    gmskmod_destroy(mod);
    gmskdem_destroy(demod);

    // print results to screen
    unsigned int delay = 2*m;
    unsigned int num_errors=0;
    for (i=delay; i<num_symbols; i++) {
        //printf("  %4u : %2u (%2u)\n", i, s[i-delay], sym_out[i]);
        num_errors += (s[i-delay] == sym_out[i]) ? 0 : 1;
    }
    printf("symbol errors : %4u / %4u\n", num_errors, num_data_symbols);

    // write results to output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"BT = %f;\n", BT);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = %u;\n", num_samples);

    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }
    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,real(y),t,imag(y));\n");

    // artificially demodulate (generate receive filter, etc.)
    float hr[2*k*m+1];
    liquid_firdes_gmskrx(k,m,BT,0,hr);
    for (i=0; i<2*k*m+1; i++)
        fprintf(fid,"hr(%3u) = %12.8f;\n", i+1, hr[i]);
    fprintf(fid,"z = filter(hr,1,arg( ([y(2:end) 0]).*conj(y) ))/k;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,z,t(k:k:end),z(k:k:end),'or');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
