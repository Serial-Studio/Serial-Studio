// 
// packetizer_persistent_ber_test.c
//
// Simulate persistent decoding error rate using persistent vs.
// regular decoding
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <getopt.h>

#include "liquid.internal.h"

#define OUTPUT_FILENAME "packetizer_persistent_ber_test.m"

void usage()
{
    printf("Usage: packetizer_persistent_ber_test\n");
    printf("  Simulates persistent decoding (best for small packets, large CRC)\n");
    printf("options:\n");
    printf("  u/h   : print usage/help\n");
    printf("  s     : SNR min [dB], default: 0\n");
    printf("  x     : SNR max [dB], default: 8\n");
    printf("  n     : number of SNR steps, default: 33\n");
    printf("  t     : number of trials, default: 2000\n");
    printf("  f     : frame size, default: 20\n");
    printf("  v     : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf("  c     : coding scheme (inner), default: h128\n");
    printf("  k     : coding scheme (outer), default: none\n");
    liquid_print_fec_schemes();
}

// persistent decoding methods
int packetizer_decode_persistent(packetizer _p, unsigned char * _pkt, unsigned char * _msg);
int packetizer_decode_persistent2(packetizer _p, unsigned char * _pkt, unsigned char * _msg);

int main(int argc, char *argv[]) {
    // set initial seed to random
    srand(time(NULL));

    // options
    unsigned int n = 20;                    // frame size (bytes)
    float SNRdB_min =  0.0f;                // signal-to-noise ratio (minimum)
    float SNRdB_max =  8.0f;                // signal-to-noise ratio (maximum)
    unsigned int num_snr = 33;              // number of SNR steps
    unsigned int num_trials=2000;           // number of trials
    crc_scheme check = LIQUID_CRC_32;       // error-detection scheme
    fec_scheme fec0 = LIQUID_FEC_HAMMING128;// error-correcting scheme (inner)
    fec_scheme fec1 = LIQUID_FEC_NONE;      // error-correcting scheme (outer)

    // get command-line options
    int dopt;
    while((dopt = getopt(argc,argv,"uhs:x:n:t:f:v:c:k:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage(); return 0;
        case 's': SNRdB_min = atof(optarg);     break;
        case 'x': SNRdB_max = atof(optarg);     break;
        case 'n': num_snr = atoi(optarg);       break;
        case 't': num_trials = atoi(optarg);    break;
        case 'f': n = atoi(optarg);             break;
        case 'v':
            // data integrity check
            check = liquid_getopt_str2crc(optarg);
            if (check == LIQUID_CRC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported CRC scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        case 'c':
            fec0 = liquid_getopt_str2fec(optarg);
            if (fec0 == LIQUID_FEC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported fec scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        case 'k':
            fec1 = liquid_getopt_str2fec(optarg);
            if (fec1 == LIQUID_FEC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported fec scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // create arrays
    unsigned int k = packetizer_compute_enc_msg_len(n,check,fec0,fec1);
    printf("dec msg len : %u\n", n);
    printf("enc msg len : %u\n", k);
    float rate = (float)n / (float)k;
    unsigned char msg_org[n];       // original data message
    unsigned char msg_enc[k];       // encoded data message
    float complex sym_rec[8*k];     // received BPSK symbols
    unsigned char msg_cor[k];       // corrupted data message
    unsigned char msg_dec_per0[n];  // decoded data message (regular decoding)
    unsigned char msg_dec_per2[n];  // decoded data message (persistent decoding)

    // create object
    packetizer q = packetizer_create(n,check,fec0,fec1);
    packetizer_print(q);

    unsigned int packet_errors_per0[num_snr];
    unsigned int packet_errors_per2[num_snr];

    //
    // set up parameters
    //
    float SNRdB_step = (SNRdB_max - SNRdB_min) / (num_snr-1);

    // 
    // start trials
    //
    
    printf("  %8s %8s [%6s] %8s %8s %8s %8s\n",
            "SNR [dB]", "Eb/N0", "trials", "per", "(PER)", "reg", "(PER)");
    unsigned int s;
    for (s=0; s<num_snr; s++) {
        // compute SNR for this level
        float SNRdB = SNRdB_min + s*SNRdB_step; // SNR in dB for this round
        float nstd = powf(10.0f, -SNRdB/20.0f); // noise standard deviation

        // reset results
        packet_errors_per0[s] = 0;
        packet_errors_per2[s] = 0;

        unsigned int t;
        for (t=0; t<num_trials; t++) {
            // generate data
            for (i=0; i<n; i++)
                msg_org[i] = rand() & 0xff;

            // encode message
            packetizer_encode(q, msg_org, msg_enc);

            // modulate with BPSK
            for (i=0; i<k; i++) {
                sym_rec[8*i+0] = (msg_enc[i] & 0x80) ? 1.0f : -1.0f;
                sym_rec[8*i+1] = (msg_enc[i] & 0x40) ? 1.0f : -1.0f;
                sym_rec[8*i+2] = (msg_enc[i] & 0x20) ? 1.0f : -1.0f;
                sym_rec[8*i+3] = (msg_enc[i] & 0x10) ? 1.0f : -1.0f;
                sym_rec[8*i+4] = (msg_enc[i] & 0x08) ? 1.0f : -1.0f;
                sym_rec[8*i+5] = (msg_enc[i] & 0x04) ? 1.0f : -1.0f;
                sym_rec[8*i+6] = (msg_enc[i] & 0x02) ? 1.0f : -1.0f;
                sym_rec[8*i+7] = (msg_enc[i] & 0x01) ? 1.0f : -1.0f;
            }

            // add noise
            for (i=0; i<8*k; i++)
                sym_rec[i] += nstd*(randnf() + _Complex_I*randf())*M_SQRT1_2;

            // convert to hard bits (hard decoding)
            for (i=0; i<k; i++) {
                msg_cor[i] = 0x00;

                msg_cor[i] |= crealf(sym_rec[8*i+0]) > 0.0f ? 0x80 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+1]) > 0.0f ? 0x40 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+2]) > 0.0f ? 0x20 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+3]) > 0.0f ? 0x10 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+4]) > 0.0f ? 0x08 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+5]) > 0.0f ? 0x04 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+6]) > 0.0f ? 0x02 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+7]) > 0.0f ? 0x01 : 0x00;
            }

            // decode
            packetizer_decode(            q, msg_cor, msg_dec_per0);
            packetizer_decode_persistent2(q, msg_cor, msg_dec_per2);
            
            // tabulate results
            packet_errors_per0[s] += count_bit_errors_array(msg_org, msg_dec_per0, n) ? 1 : 0;
            packet_errors_per2[s] += count_bit_errors_array(msg_org, msg_dec_per2, n) ? 1 : 0;
        }

        // print results for this SNR step
        printf("  %8.3f %8.3f [%6u] %8u %8.2f %8u %8.2f\n",
                SNRdB,
                SNRdB - 10*log10f(rate),
                num_trials,
                packet_errors_per2[s], 100.0f * (float)(packet_errors_per2[s]) / (float)(num_trials),
                packet_errors_per0[s], 100.0f * (float)(packet_errors_per0[s]) / (float)(num_trials) );
    }

    // clean up objects
    packetizer_destroy(q);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"\n\n");
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"n = %u;    %% frame size [bytes]\n", n);
    fprintf(fid,"k = %u;    %% encoded frame size [bytes]\n", k);
    fprintf(fid,"r = n / k; %% true rate\n");
    fprintf(fid,"num_snr = %u;\n", num_snr);
    fprintf(fid,"num_trials = %u;\n", num_trials);
    for (i=0; i<num_snr; i++) {
        fprintf(fid,"SNRdB(%4u) = %12.8f;\n",i+1, SNRdB_min + i*SNRdB_step);
        fprintf(fid,"packet_errors_per0(%6u) = %u;\n", i+1, packet_errors_per0[i]);
        fprintf(fid,"packet_errors_per2(%6u) = %u;\n", i+1, packet_errors_per2[i]);
    }
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(SNRdB, packet_errors_per0 / num_trials + 1e-12,  '-x',\n");
    fprintf(fid,"         SNRdB, packet_errors_per2 / num_trials + 1e-12,  '-x');\n");
    fprintf(fid,"axis([%f %f 1e-3 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"legend('regular','persistent',1);\n");
    fprintf(fid,"xlabel('SNR [dB]');\n");
    fprintf(fid,"ylabel('Packet Error Rate');\n");
    fprintf(fid,"title('BER vs. SNR for %s//%s//%s, r=%f');\n", fec_scheme_str[fec0][1],
                                                                fec_scheme_str[fec1][1],
                                                                crc_scheme_str[check][1],
                                                                (float)n / (float)k);
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

// Execute the packetizer to decode an input message, return validity
// check of resulting data; persistent attempts to find error(s)
//
//  _p      :   packetizer object
//  _pkt    :   input message (coded bytes)
//  _msg    :   decoded output message
int packetizer_decode_persistent(packetizer _p,
                                 unsigned char * _pkt,
                                 unsigned char * _msg)
{
    // try regular decoding
    int crc_pass = packetizer_decode(_p, _pkt, _msg);

    // return if decoding was successful
    if (crc_pass)
        return crc_pass;

    unsigned int i;
    unsigned int key=0;

    // result is stored in _p->buffer_0; flip bits to try to get
    // CRC to pass
    for (i=0; i<_p->msg_len + _p->crc_length; i++) {
        unsigned int j;
        for (j=0; j<8; j++) {
            // flip bit
            unsigned char mask = 1 << (8-j-1);
            _p->buffer_0[i] ^= mask;

            // strip crc, validate message
            key = 0;
            unsigned int k;
            for (k=0; k<_p->crc_length; k++) {
                key <<= 8;

                key |= _p->buffer_0[_p->msg_len+k];
            }

            // compute crc validity
            crc_pass = crc_validate_message(_p->check,
                                            _p->buffer_0,
                                            _p->msg_len,
                                            key);

            // check validity
            if (crc_pass) {
                // copy result to output and return
                memmove(_msg, _p->buffer_0, _p->msg_len);
                //printf("persistent decoding worked!\n");
                return crc_pass;
            } else {
                // flip bit back
                _p->buffer_0[i] ^= mask;
            }
        }
    }
    
    // copy result to output and return
    memmove(_msg, _p->buffer_0, _p->msg_len);
    return crc_pass;
}
    
// double bit errors in a single byte
//  nchoosek(8,2) = 28
unsigned char packetizer_persistent_mask2[28] = {
    0x03,   // 00000011
    0x06,   // 00000110
    0x0C,   // 00001100
    0x18,   // 00011000
    0x30,   // 00110000
    0x60,   // 01100000
    0xC0,   // 11000000

    0x05,   // 00000101
    0x0A,   // 00001010
    0x14,   // 00010100
    0x28,   // 00101000
    0x50,   // 01010000
    0xA0,   // 10100000

    0x09,   // 00001001
    0x12,   // 00010010
    0x24,   // 00100100
    0x48,   // 01001000
    0x90,   // 10010000

    0x11,   // 00010001
    0x22,   // 00100010
    0x44,   // 01000100
    0x88,   // 10001000

    0x21,   // 00100001
    0x42,   // 01000010
    0x84,   // 10000100

    0x41,   // 01000001
    0x82,   // 10000010

    0x81,   // 10000001
};

// Execute the packetizer to decode an input message, return validity
// check of resulting data; persistent attempts to find error(s)
//
//  _p      :   packetizer object
//  _pkt    :   input message (coded bytes)
//  _msg    :   decoded output message
int packetizer_decode_persistent2(packetizer _p,
                                  unsigned char * _pkt,
                                  unsigned char * _msg)
{
    // try first-order persistent decoding
    int crc_pass = packetizer_decode_persistent(_p, _pkt, _msg);

    // return if decoding was successful
    if (crc_pass)
        return crc_pass;

    unsigned int i;
    unsigned int key=0;

    // result is stored in _p->buffer_0; flip bits to try to get
    // CRC to pass
    for (i=0; i<_p->msg_len + _p->crc_length; i++) {
        unsigned int j;
        for (j=0; j<28; j++) {
            // apply mask
            _p->buffer_0[i] ^= packetizer_persistent_mask2[j];

            // strip crc, validate message
            key = 0;
            unsigned int k;
            for (k=0; k<_p->crc_length; k++) {
                key <<= 8;

                key |= _p->buffer_0[_p->msg_len+k];
            }

            // compute crc validity
            crc_pass = crc_validate_message(_p->check,
                                            _p->buffer_0,
                                            _p->msg_len,
                                            key);

            // check validity
            if (crc_pass) {
                // copy result to output and return
                memmove(_msg, _p->buffer_0, _p->msg_len);
                //printf("persistent decoding worked! (2)\n");
                return crc_pass;
            } else {
                // flip bit back
                _p->buffer_0[i] ^= packetizer_persistent_mask2[j];
            }
        }
    }
    
    // copy result to output and return
    memmove(_msg, _p->buffer_0, _p->msg_len);
    return crc_pass;
}

