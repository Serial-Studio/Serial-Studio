// This example demonstrates how the nco/pll object (numerically-controlled
// oscillator with phase-locked loop) can be used for carrier frequency
// recovery in digital modems.  The modem type, SNR, and other parameters are
// specified via the command-line interface.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "nco_pll_modem_example.m"

// print usage/help message
void usage()
{
    printf("nco_pll_modem_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  s     : signal-to-noise ratio, default: 30dB\n");
    printf("  b     : pll bandwidth, default: 0.002\n");
    printf("  n     : number of symbols, default: 1200\n");
    printf("  P     : phase offset (radians), default: pi/10 ~ 0.3146\n");
    printf("  F     : frequency offset (radians), default: 0.1\n");
    printf("  m     : modulation scheme, default: qpsk\n");
    liquid_print_modulation_schemes();
}

int main(int argc, char*argv[]) {
    srand( time(NULL) );
    // parameters
    float phase_offset = M_PI/10;
    float frequency_offset = 0.10f;
    float SNRdB = 30.0f;
    float pll_bandwidth = 0.002f;
    modulation_scheme ms = LIQUID_MODEM_QPSK;
    unsigned int n=1200;    // number of iterations

    int dopt;
    while ((dopt = getopt(argc,argv,"uhs:b:n:P:F:m:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();    return 0;
        case 's':   SNRdB = atof(optarg);           break;
        case 'b':   pll_bandwidth = atof(optarg);   break;
        case 'n':   n = atoi(optarg);               break;
        case 'P':   phase_offset = atof(optarg);    break;
        case 'F':   frequency_offset= atof(optarg); break;
        case 'm':
            ms = liquid_getopt_str2mod(optarg);
            if (ms == LIQUID_MODEM_UNKNOWN) {
                fprintf(stderr,"error: %s, unknown/unsupported modulation scheme \"%s\"\n", argv[0], optarg);
                return 1;
            }
            break;
        default:
            exit(1);
        }
    }
    unsigned int d=n/32;      // print every "d" lines

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid, "%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid, "clear all; close all;\n");
    fprintf(fid, "phi=zeros(1,%u);\n",n);
    fprintf(fid, "r=zeros(1,%u);\n",n);

    // objects
    nco_crcf nco_tx = nco_crcf_create(LIQUID_VCO);
    nco_crcf nco_rx = nco_crcf_create(LIQUID_VCO);

    modemcf mod   = modemcf_create(ms);
    modemcf demod = modemcf_create(ms);

    unsigned int bps = modemcf_get_bps(mod);

    // initialize objects
    nco_crcf_set_phase(nco_tx, phase_offset);
    nco_crcf_set_frequency(nco_tx, frequency_offset);
    nco_crcf_pll_set_bandwidth(nco_rx, pll_bandwidth);

    float noise_power = powf(10.0f, -SNRdB/20.0f);

    // print parameters
    printf("PLL example :\n");
    printf("modem : %u-%s\n", 1<<bps, modulation_types[ms].name);
    printf("frequency offset: %6.3f, phase offset: %6.3f, SNR: %6.2fdB, pll b/w: %6.3f\n",
            frequency_offset, phase_offset, SNRdB, pll_bandwidth);

    // run loop
    unsigned int i, M=1<<bps, sym_in, sym_out, num_errors=0;
    float phase_error;
    float complex x, r, v, noise;
    for (i=0; i<n; i++) {
        // generate random symbol
        sym_in = rand() % M;

        // modulate
        modemcf_modulate(mod, sym_in, &x);

        // channel
        //r = nco_crcf_cexpf(nco_tx);
        nco_crcf_mix_up(nco_tx, x, &r);

        // add complex white noise
        crandnf(&noise);
        r += noise * noise_power;

        // 
        //v = nco_crcf_cexpf(nco_rx);
        nco_crcf_mix_down(nco_rx, r, &v);

        // demodulate
        modemcf_demodulate(demod, v, &sym_out);
        num_errors += count_bit_errors(sym_in, sym_out);

        // error estimation
        //phase_error = cargf(r*conjf(v));
        phase_error = modemcf_get_demodulator_phase_error(demod);

        // perfect error estimation
        //phase_error = nco_tx->theta - nco_rx->theta;

        // print every line in a format that octave can read
        fprintf(fid, "phi(%u) = %10.6E;\n", i+1, phase_error);
        fprintf(fid, "dphi(%u) = %10.6E;\n", i+1, nco_crcf_get_frequency(nco_rx));
        fprintf(fid, "r(%u) = %10.6E + j*%10.6E;\n",
                i+1, crealf(v), cimagf(v));

        if ((i+1)%d == 0 || i==n-1) {
            printf("  %4u: e_hat : %6.3f, phase error : %6.3f, freq error : %12.9f\n",
                    i+1,                                // iteration
                    phase_error,                        // estimated phase error
                    nco_crcf_get_phase(nco_tx) - nco_crcf_get_phase(nco_rx),// true phase error
                    nco_crcf_get_frequency(nco_tx) - nco_crcf_get_frequency(nco_rx)// true frequency error
                  );
        }

        // update tx nco object
        nco_crcf_step(nco_tx);

        // update pll
        nco_crcf_pll_step(nco_rx, phase_error);

        // update rx nco object
        nco_crcf_step(nco_rx);
    }

    fprintf(fid, "figure('color','white','position',[100 100 1200 400]);\n");
    fprintf(fid, "subplot(1,3,1:2);\n");
    fprintf(fid, "plot(1:length(phi),phi,'LineWidth',2,'Color',[0 0.25 0.5]);\n");
    fprintf(fid, "xlabel('Symbol Index');\n");
    fprintf(fid, "ylabel('Phase Error [radians]');\n");
    fprintf(fid, "grid on;\n");

    fprintf(fid, "t0 = round(0.25*length(r));\n");
    fprintf(fid, "subplot(1,3,3);\n");
    fprintf(fid, "plot(r(1:t0),'.','Color',[0.6 0.6 0.6],r(t0:end),'.','Color',[0 0.25 0.5]);\n");
    fprintf(fid, "grid on;\n");
    fprintf(fid, "axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid, "axis('square');\n");
    fprintf(fid, "xlabel('In-Phase');\n");
    fprintf(fid, "ylabel('Quadrature');\n");
    //fprintf(fid, "legend(['first 25%%'],['last 75%%'],1);\n");
    fclose(fid);

    printf("results written to %s.\n",OUTPUT_FILENAME);

    nco_crcf_destroy(nco_tx);
    nco_crcf_destroy(nco_rx);

    modemcf_destroy(mod);
    modemcf_destroy(demod);

    printf("bit errors: %u / %u\n", num_errors, bps*n);
    printf("done.\n");
    return 0;
}
