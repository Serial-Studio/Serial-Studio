//
// qpacketmodem_example.c
//
// This example demonstrates the basic packet modem encoder/decoder
// operation. A packet of data is encoded and modulated into symbols,
// channel noise is added, and the resulting packet is demodulated
// and decoded.
//

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
    printf("ofdmflexframesync_example [options]\n");
    printf("  h     : print usage\n");
    printf("  n     : payload length [bytes], default: 400\n");
    printf("  m     : modulation scheme (qpsk default)\n");
    liquid_print_modulation_schemes();
    printf("  v     : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf("  c     : coding scheme (inner): g2412 default\n");
    printf("  k     : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
    printf("  s     : signal-to-noise ratio [dB], default: 6\n");
}

int main(int argc, char *argv[])
{
    //srand( time(NULL) );

    // options
    modulation_scheme ms     = LIQUID_MODEM_QPSK;        // mod. scheme
    crc_scheme   check       = LIQUID_CRC_32;            // data validity check
    fec_scheme   fec0        = LIQUID_FEC_GOLAY2412;     // fec (inner)
    fec_scheme   fec1        = LIQUID_FEC_NONE;          // fec (outer)
    unsigned int payload_len = 400;                      // payload length
    float        SNRdB       = 6.0f;                     // SNR [dB]
    const char   filename[]  = "qpacketmodem_example.m"; // output filename

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hn:m:v:c:k:s:")) != EOF){
        switch (dopt) {
        case 'h': usage();                                     return 0;
        case 'n': payload_len = atol(optarg);                  break;
        case 'm': ms          = liquid_getopt_str2mod(optarg); break;
        case 'v': check       = liquid_getopt_str2crc(optarg); break;
        case 'c': fec0        = liquid_getopt_str2fec(optarg); break;
        case 'k': fec1        = liquid_getopt_str2fec(optarg); break;
        case 's': SNRdB       = atof(optarg);                  break;
        default:
            exit(-1);
        }
    }
    unsigned int i;

    // derived values
    float nstd = powf(10.0f, -SNRdB/20.0f);

    // create and configure packet encoder/decoder object
    qpacketmodem q = qpacketmodem_create();
    qpacketmodem_configure(q, payload_len, check, fec0, fec1, ms);
    qpacketmodem_print(q);

    // initialize payload
    unsigned char payload_tx[payload_len];
    unsigned char payload_rx[payload_len];

    // initialize payload
    for (i=0; i<payload_len; i++) {
        payload_tx[i] = rand() & 0xff;
        payload_rx[i] = 0x00;
    }

    // get frame length
    unsigned int frame_len = qpacketmodem_get_frame_len(q);

    // allocate memory for frame samples
    float complex frame_tx[frame_len];
    float complex frame_rx[frame_len];

    // encode frame
    qpacketmodem_encode(q, payload_tx, frame_tx);

    // add noise
    for (i=0; i<frame_len; i++)
        frame_rx[i] = frame_tx[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

    // decode frame
    int crc_pass = qpacketmodem_decode(q, frame_rx, payload_rx);

    // count errors
    unsigned int num_bit_errors = count_bit_errors_array(payload_tx, payload_rx, payload_len);

    // print results
    printf("payload pass ? %s, errors: %u / %u\n",
            crc_pass ? "pass" : "FAIL",
            num_bit_errors,
            8*payload_len);

    // destroy allocated objects
    qpacketmodem_destroy(q);

    // write symbols to output file for plotting
    FILE * fid = fopen(filename,"w");
    if (!fid) {
        fprintf(stderr,"error: could not open '%s' for writing\n", filename);
        return -1;
    }
    fprintf(fid,"%% %s : auto-generated file\n", filename);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"frame_len = %u;\n", frame_len);
    fprintf(fid,"y = zeros(1,frame_len);\n");
    for (i=0; i<frame_len; i++)
        fprintf(fid,"y(%6u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(frame_rx[i]), cimagf(frame_rx[i]));
    fprintf(fid,"figure('Color','white');\n");
    fprintf(fid,"plot(real(y),imag(y),'x','MarkerSize',3);\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('real');\n");
    fprintf(fid,"ylabel('imag');\n");

    fclose(fid);
    printf("results written to '%s'\n", filename);

    printf("done.\n");
    return 0;
}

