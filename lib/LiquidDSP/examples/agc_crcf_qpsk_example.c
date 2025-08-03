// Automatic gain control test for data signals with fluctuating signal
// levels.  QPSK modulation introduces periodic random zero-crossings
// which gives instantaneous amplitude levels near zero.  This example
// tests the response of the AGC to these types of signals.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "agc_crcf_qpsk_example.m"

int main(int argc, char*argv[])
{
    // options
    float        noise_floor= -40.0f;   // noise floor [dB]
    float        SNRdB      = 20.0f;    // signal-to-noise ratio [dB]
    float        bt         = 0.05f;    // loop bandwidth
    unsigned int num_symbols= 100;      // number of iterations
    unsigned int d          = 5;        // print every d iterations

    unsigned int k          = 2;        // interpolation factor (samples/symbol)
    unsigned int m          = 3;        // filter delay (symbols)
    float        beta       = 0.3f;     // filter excess bandwidth factor
    float        dt         = 0.0f;     // filter fractional sample delay

    // derived values
    unsigned int num_samples=num_symbols*k;
    float gamma = powf(10.0f, (SNRdB+noise_floor)/20.0f);   // channel gain
    float nstd = powf(10.0f, noise_floor / 20.0f);

    // arrays
    float complex x[num_samples];
    float complex y[num_samples];
    float rssi[num_samples];

    // create objects
    modemcf mod = modemcf_create(LIQUID_MODEM_QPSK);
    firinterp_crcf interp = firinterp_crcf_create_prototype(LIQUID_FIRFILT_RRC,k,m,beta,dt);
    agc_crcf p = agc_crcf_create();
    agc_crcf_set_bandwidth(p, bt);

    unsigned int i;

    // print info
    printf("automatic gain control // loop bandwidth: %4.2e\n",bt);

    unsigned int sym;
    float complex s;
    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        sym = modemcf_gen_rand_sym(mod);
        modemcf_modulate(mod, sym, &s);
        s *= gamma;

        firinterp_crcf_execute(interp, s, &x[i*k]);
    }

    // add noise
    for (i=0; i<num_samples; i++)
        x[i] += nstd*(randnf() + _Complex_I*randnf()) * M_SQRT1_2;

    // run agc
    for (i=0; i<num_samples; i++) {
        agc_crcf_execute(p, x[i], &y[i]);

        rssi[i] = agc_crcf_get_rssi(p);
    }

    // destroy objects
    modemcf_destroy(mod);
    agc_crcf_destroy(p);
    firinterp_crcf_destroy(interp);

    // print results to screen
    printf("received signal strength indication (rssi):\n");
    for (i=0; i<num_samples; i+=d) {
        printf("%4u : %8.2f\n", i, rssi[i]);
    }


    // 
    // export results
    //
    FILE* fid = fopen(OUTPUT_FILENAME,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, could not open '%s' for writing\n", argv[0], OUTPUT_FILENAME);
        exit(1);
    }
    fprintf(fid,"%% %s: auto-generated file\n\n",OUTPUT_FILENAME);
    fprintf(fid,"n = %u;\n", num_samples);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"   x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"   y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"rssi(%4u) = %12.4e;\n", i+1, rssi[i]);
    }
    fprintf(fid,"\n\n");
    fprintf(fid,"n = length(x);\n");
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"figure('position',[100 100 800 600]);\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,rssi,'-k','LineWidth',2);\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('rssi [dB]');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,real(y),t,imag(y));\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('agc output');\n");
    fprintf(fid,"  grid on;\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

