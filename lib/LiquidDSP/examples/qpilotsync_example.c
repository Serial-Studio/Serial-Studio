// Demonstrate using qpilotsync for carrier recovery.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <assert.h>

#include "liquid.h"

void usage()
{
    printf("%s [options]\n", __FILE__);
    printf("  h     : print usage\n");
    printf("  n     : payload length [symbols], default: 400\n");
    printf("  p     : pilot spacing [symbols],  default: 20\n");
    printf("  s     : SNR [dB],                 default: 20\n");
    printf("  m     : modulation scheme,        default: qam16\n");
    liquid_print_modulation_schemes();
}

int main(int argc, char *argv[])
{
    //srand( time(NULL) );

    // options
    int          ms            = LIQUID_MODEM_QAM16;        // mod. scheme
    unsigned int payload_len   = 400;                       // payload length
    unsigned int pilot_spacing =  20;                       // pilot spacing
    float        SNRdB         = 20.0f;                     // signal-to-noise ratio [dB]
    const char   filename[]    = "qpilotsync_example.m";    // output filename
    float        dphi          = -0.0075f;                   // carrier frequency offset
    float        phi           = 2.1800f;                   // carrier phase offset
    float        gain          = 0.5f;                      // channel gain

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hn:p:s:m:")) != EOF){
        switch (dopt) {
        case 'h': usage();                                       return 0;
        case 'n': payload_len   = atoi(optarg);                  break;
        case 'p': pilot_spacing = atoi(optarg);                  break;
        case 's': SNRdB         = atof(optarg);                  break;
        case 'm': ms            = liquid_getopt_str2mod(optarg); break;
        default:
            exit(-1);
        }
    }
    unsigned int i;

    // derived values
    float nstd = powf(10.0f, -SNRdB/20.0f);

    // create pilot generator and synchronizer objects
    qpilotgen  pg = qpilotgen_create( payload_len, pilot_spacing);
    qpilotsync ps = qpilotsync_create(payload_len, pilot_spacing);
    qpilotgen_print(pg);

    // get frame length
    unsigned int frame_len = qpilotgen_get_frame_len(pg);

    // allocate arrays
    unsigned char payload_sym_tx[payload_len];  // transmitted payload symbols
    float complex payload_tx    [payload_len];  // transmitted payload samples
    float complex frame_tx      [frame_len];    // transmitted frame samples
    float complex frame_rx      [frame_len];    // received frame samples
    float complex payload_rx    [payload_len];  // received payload samples
    unsigned char payload_sym_rx[payload_len];  // received payload symbols

    // create modem objects for payload
    modemcf mod = modemcf_create(ms);
    modemcf dem = modemcf_create(ms);

    // assemble payload symbols
    for (i=0; i<payload_len; i++) {
        // generate random symbol
        payload_sym_tx[i] = modemcf_gen_rand_sym(mod);

        // modulate
        modemcf_modulate(mod, payload_sym_tx[i], &payload_tx[i]);
    }

    // assemble frame
    qpilotgen_execute(pg, payload_tx, frame_tx);

    // add channel impairments
    for (i=0; i<frame_len; i++) {
        frame_rx[i]  = frame_tx[i] * cexpf(_Complex_I*dphi*i + _Complex_I*phi);
        frame_rx[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
        frame_rx[i] *= gain;
    }

    // receive frame
    qpilotsync_execute(ps, frame_rx, payload_rx);

    // demodulate
    for (i=0; i<payload_len; i++) {
        unsigned int sym_demod;
        modemcf_demodulate(dem, payload_rx[i], &sym_demod);
        payload_sym_rx[i] = (unsigned char)sym_demod;
    }

    // count errors
    unsigned int bit_errors = 0;
    for (i=0; i<payload_len; i++)
        bit_errors += count_bit_errors(payload_sym_rx[i], payload_sym_tx[i]);
    printf("received bit errors : %u / %u\n", bit_errors, payload_len * modemcf_get_bps(mod));

    // destroy allocated objects
    qpilotgen_destroy(pg);
    qpilotsync_destroy(ps);
    modemcf_destroy(mod);
    modemcf_destroy(dem);

    // write symbols to output file for plotting
    FILE * fid = fopen(filename,"w");
    if (!fid) {
        fprintf(stderr,"error: could not open '%s' for writing\n", filename);
        return -1;
    }
    fprintf(fid,"%% %s : auto-generated file\n", filename);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"payload_len = %u;\n", payload_len);
    fprintf(fid,"frame_len   = %u;\n", frame_len);
    fprintf(fid,"frame_tx   = zeros(1,frame_len);\n");
    fprintf(fid,"payload_rx = zeros(1,payload_len);\n");
    for (i=0; i<frame_len; i++)
        fprintf(fid,"frame_rx(%6u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(frame_rx[i]), cimagf(frame_rx[i]));
    for (i=0; i<payload_len; i++)
        fprintf(fid,"payload_rx(%6u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(payload_rx[i]), cimagf(payload_rx[i]));
    fprintf(fid,"figure('Color','white','position',[100 100 950 400]);\n");
    fprintf(fid,"subplot(1,2,1);\n");
    fprintf(fid,"  plot(real(frame_rx),  imag(frame_rx),  'x','MarkerSize',3);\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('real');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"  title('received');\n");
    fprintf(fid,"subplot(1,2,2);\n");
    fprintf(fid,"  plot(real(payload_rx),imag(payload_rx),'x','MarkerSize',3);\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('real');\n");
    fprintf(fid,"  ylabel('imag');\n");
    fprintf(fid,"  title('recovered');\n");

    fclose(fid);
    printf("results written to '%s'\n", filename);

    printf("done.\n");
    return 0;
}

