// This example demonstrates the performance of the qpacket modem
// object to combine forward error-correction and modulation in one
// simple interface.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <assert.h>

#include "liquid.h"
    
#define OUTPUT_FILENAME "qpacketmodem_performance_example.m"

void usage()
{
    printf("qpacketmodem_performance_example [options]\n");
    printf(" -h     : print usage\n");
    printf(" -p     : payload length [bytes], default: 400\n");
    printf(" -m     : modulation scheme (qpsk default)\n");
    liquid_print_modulation_schemes();
    printf(" -v     : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf(" -c     : coding scheme (inner): g2412 default\n");
    printf(" -k     : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
    printf(" -s     : SNR start [dB], default: -5\n");
    printf(" -x     : SNR max [dB], default: 20\n");
    printf(" -d     : SNR step [dB], default: 0.5\n");
    printf(" -e     : minimum number of packet errors per step, default: 50\n");
    printf(" -t     : minimum number of packet trials per step, default: 2000\n");
    printf(" -T     : maximum number of packet trials per step, default: 40000\n");
}

int main(int argc, char *argv[])
{
    // options (defaults to frame64 parameters)
    modulation_scheme ms                = LIQUID_MODEM_QPSK;    // mod. scheme
    crc_scheme        check             = LIQUID_CRC_24;        // data validity check
    fec_scheme        fec0              = LIQUID_FEC_NONE;      // fec (inner)
    fec_scheme        fec1              = LIQUID_FEC_GOLAY2412; // fec (outer)
    unsigned int      payload_len       =  72;                  // payload length
    float             SNRdB_min         = -5.0f;                // signal-to-noise ratio (minimum)
    float             SNRdB_max         = 20.0f;                // signal-to-noise ratio (maximum)
    float             SNRdB_step        =  0.5f;                // signal-to-noise ratio (maximum)
    unsigned int      min_packet_errors =   50;                 // minimum errors to observe for each step
    unsigned int      min_packet_trials = 2000;                 // minimum number of packets for each step
    unsigned int      max_packet_trials =40000;                 // maximum number of packets for each step

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hp:m:v:c:k:s:x:d:e:t:T:")) != EOF){
        switch (dopt) {
        case 'h': usage();                                           return 0;
        case 'p': payload_len       = atol(optarg);                  break;
        case 'm': ms                = liquid_getopt_str2mod(optarg); break;
        case 'v': check             = liquid_getopt_str2crc(optarg); break;
        case 'c': fec0              = liquid_getopt_str2fec(optarg); break;
        case 'k': fec1              = liquid_getopt_str2fec(optarg); break;
        case 's': SNRdB_min         = atof(optarg);                  break;
        case 'x': SNRdB_max         = atof(optarg);                  break;
        case 'd': SNRdB_step        = atof(optarg);                  break;
        case 'e': min_packet_errors = atoi(optarg);                  break;
        case 't': min_packet_trials = atoi(optarg);                  break;
        case 'T': max_packet_trials = atoi(optarg);                  break;
        default:
            exit(-1);
        }
    }
    unsigned int i;

    // create and configure packet encoder/decoder object
    qpacketmodem q = qpacketmodem_create();
    qpacketmodem_configure(q, payload_len, check, fec0, fec1, ms);
    qpacketmodem_print(q);

    // get frame length
    unsigned int frame_len = qpacketmodem_get_frame_len(q);

    // initialize payload
    unsigned char payload_tx       [payload_len]; // payload (transmitted)
    unsigned char payload_rx       [payload_len]; // payload (received)
    float complex frame_tx         [frame_len];   // frame samples (transmitted)
    float complex frame_rx         [frame_len];   // frame samples (received)

    // output file
    FILE* fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s: auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all; SNR=[]; ber=[]; per=[];\n");

    printf("  %8s %8s %8s %12s %8s %8s %6s\n",
            "SNR [dB]", "errors", "bits", "BER", "errors", "packets", "PER");
    float SNRdB = SNRdB_min;
    while (SNRdB < SNRdB_max) {
        // reset counters
        float nstd = powf(10.0f, -SNRdB/20.0f); // noise standard deviation
        unsigned int num_packet_trials = 0, num_packet_errors = 0;
        unsigned int num_bit_trials = 0, num_bit_errors = 0;
        while (1) {
            // initialize payload
            for (i=0; i<payload_len; i++) {
                payload_tx[i] = rand() & 0xff;
                payload_rx[i] = 0x00;
            }

            // encode frame
            qpacketmodem_encode(q, payload_tx, frame_tx);

            // add noise
            for (i=0; i<frame_len; i++)
                frame_rx[i] = frame_tx[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

            // decode frame
            int crc_pass = qpacketmodem_decode(q, frame_rx, payload_rx);

            // accumulate errors
            num_bit_errors    += count_bit_errors_array(payload_tx, payload_rx, payload_len);
            num_packet_errors += crc_pass ? 0 : 1;
            num_packet_trials += 1;
            num_bit_trials    += 8*payload_len;

            if (num_packet_trials < min_packet_trials)
                continue;
            if (num_packet_errors >= min_packet_errors)
                break;
            if (num_packet_trials >= max_packet_trials)
                break;
        }
        float BER = (float)num_bit_errors    / (float)num_bit_trials;
        float PER = (float)num_packet_errors / (float)num_packet_trials;
        printf("  %8.2f %8u %8u %12.4e %8u %8u %6.2f%%\n",
                SNRdB,
                num_bit_errors, num_bit_trials, BER,
                num_packet_errors, num_packet_trials, PER*100.0f);
        fprintf(fid,"SNR(end+1)=%g; ber(end+1)=%g; per(end+1)=%g;\n", SNRdB, BER, PER);
        if (num_packet_errors < min_packet_errors)
            break;
        SNRdB += SNRdB_step;
    }

    // destroy allocated objects
    qpacketmodem_destroy(q);

    fprintf(fid,"figure('position',[100 100 500 720]);\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  gamma = 10.^(SNR/10);\n");
    fprintf(fid,"  ber_qpsk = 0.5*(1 - erf(sqrt(gamma)/sqrt(2)));\n");
    fprintf(fid,"  hold on;\n");
    fprintf(fid,"    semilogy(SNR, ber,     '-o', 'LineWidth',2, 'MarkerSize',2);\n");
    fprintf(fid,"    semilogy(SNR, ber_qpsk,'-o', 'LineWidth',2, 'MarkerSize',2);\n");
    fprintf(fid,"  hold off;\n");
    fprintf(fid,"  xlabel('SNR [dB]');\n");
    fprintf(fid,"  ylabel('Bit Error Rate');\n");
    fprintf(fid,"  axis([%f SNR(end)+5 1e-6 1]);\n", SNRdB_min);
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  legend('Measured','Uncoded QPSK','location','southwest');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  semilogy(SNR, per,'-o', 'LineWidth',2, 'MarkerSize',2);\n");
    fprintf(fid,"  xlabel('SNR [dB]');\n");
    fprintf(fid,"  ylabel('Packet Error Rate');\n");
    fprintf(fid,"  axis([%f SNR(end)+5 1e-3 1]);\n", SNRdB_min);
    fprintf(fid,"  grid on;\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

