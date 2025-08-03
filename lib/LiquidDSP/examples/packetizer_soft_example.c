//
// packetizer_soft_example.c
//
// This example demonstrates the functionality of the packetizer object
// for soft-decision decoding.  Data are encoded using two forward error-
// correction schemes (an inner and outer code) before noise and data
// errors are added. The decoder then tries to recover the original data
// message. Only the outer code uses soft-decision decoding.
//
// SEE ALSO: fec_soft_example.c
//           packetizer_example.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "liquid.h"

// print usage/help message
void usage()
{
    printf("packetizer_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  n     : input data size (number of uncoded bytes): 8 default\n");
    printf("  v     : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf("  c     : coding scheme (inner): h74 default\n");
    printf("  k     : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
}


int main(int argc, char*argv[]) {
    // options
    unsigned int n=8;                       // original data message length
    crc_scheme check = LIQUID_CRC_32;       // data integrity check
    fec_scheme fec0 = LIQUID_FEC_HAMMING74; // inner code
    fec_scheme fec1 = LIQUID_FEC_NONE;      // outer code

    // read command-line options
    int dopt;
    while((dopt = getopt(argc,argv,"uhn:v:c:k:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage(); return 0;
        case 'n':
            n = atoi(optarg);
            if (n < 1) {
                printf("error: packet length must be positive\n");
                usage();
                exit(-1);
            }
            break;
        case 'v':
            // data integrity check
            check = liquid_getopt_str2crc(optarg);
            if (check == LIQUID_CRC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported CRC scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        case 'c':
            // inner FEC scheme
            fec0 = liquid_getopt_str2fec(optarg);
            if (fec0 == LIQUID_FEC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported inner FEC scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        case 'k':
            // outer FEC scheme
            fec1 = liquid_getopt_str2fec(optarg);
            if (fec1 == LIQUID_FEC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported outer FEC scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    unsigned int i;
    unsigned int k = packetizer_compute_enc_msg_len(n,check,fec0,fec1);
    packetizer p = packetizer_create(n,check,fec0,fec1);
    packetizer_print(p);

    // initialize arrays
    unsigned char msg_org[n];   // original message
    unsigned char msg_enc[k];   // encoded message
    unsigned char msg_rec[8*k]; // received message (soft bits)
    unsigned char msg_dec[n];   // decoded message
    int crc_pass;

    // initialize original data message
    for (i=0; i<n; i++)
        msg_org[i] = rand() % 256;

    // encode packet
    packetizer_encode(p,msg_org,msg_enc);

    // convert to soft bits and add 'noise'
    for (i=0; i<k; i++) {
        msg_rec[8*i+0] = (msg_enc[i] & 0x80) ? 255 : 0;
        msg_rec[8*i+1] = (msg_enc[i] & 0x40) ? 255 : 0;
        msg_rec[8*i+2] = (msg_enc[i] & 0x20) ? 255 : 0;
        msg_rec[8*i+3] = (msg_enc[i] & 0x10) ? 255 : 0;
        msg_rec[8*i+4] = (msg_enc[i] & 0x08) ? 255 : 0;
        msg_rec[8*i+5] = (msg_enc[i] & 0x04) ? 255 : 0;
        msg_rec[8*i+6] = (msg_enc[i] & 0x02) ? 255 : 0;
        msg_rec[8*i+7] = (msg_enc[i] & 0x01) ? 255 : 0;
    }

    // flip first bit (ensure error)
    msg_rec[0] = 255 - msg_rec[0];

    // add noise (but not so much that it would cause a bit error)
    for (i=0; i<8*k; i++) {
        int soft_bit = msg_rec[i] + (int)(20*randnf());
        if (soft_bit > 255) soft_bit = 255;
        if (soft_bit <   0) soft_bit = 0;
        msg_rec[i] = soft_bit;
    }

    // decode packet
    crc_pass =
    packetizer_decode_soft(p,msg_rec,msg_dec);

    // clean up allocated objects
    packetizer_destroy(p);

    // print results
    printf("original message:  [%3u] ",n);
    for (i=0; i<n; i++)
        printf(" %.2X", (unsigned int) (msg_org[i]));
    printf("\n");

    printf("encoded message:   [%3u] ",k);
    for (i=0; i<k; i++)
        printf(" %.2X", (unsigned int) (msg_enc[i]));
    printf("\n");

#if 0
    printf("received message:  [%3u] ",k);
    for (i=0; i<k; i++)
        printf("%c%.2X", msg_rec[i]==msg_enc[i] ? ' ' : '*', (unsigned int) (msg_rec[i]));
    printf("\n");
#endif

    //if (verbose) {
    if (1) {
        // print expanded result (print each soft bit value)
        for (i=0; i<k; i++) {
            unsigned char msg_cor_hard = 0x00;
            printf("%5u: ", i);
            unsigned int j;
            for (j=0; j<8; j++) {
                msg_cor_hard |= (msg_rec[8*i+j] > 127) ? 1<<(8-j-1) : 0;
                unsigned int bit_enc = (msg_enc[i] >> (8-j-1)) & 0x01;
                unsigned int bit_rec = (msg_rec[8*i+j] > 127) ? 1 : 0;
                //printf("%1u %3u (%1u) %c", bit_enc, msg_rec[i], bit_rec, bit_enc != bit_rec ? '*' : ' ');
                printf("%4u%c", msg_rec[8*i+j], bit_enc != bit_rec ? '*' : ' ');
            }
            printf("  :  %c%.2X\n", msg_cor_hard==msg_enc[i] ? ' ' : '*', (unsigned int) (msg_cor_hard));
        }
    } // verbose


    printf("decoded message:   [%3u] ",n);
    for (i=0; i<n; i++)
        printf("%c%.2X", msg_dec[i] == msg_org[i] ? ' ' : '*', (unsigned int) (msg_dec[i]));
    printf("\n");
    printf("\n");

    // count bit errors
    unsigned int num_sym_errors=0;
    unsigned int num_bit_errors=0;
    for (i=0; i<n; i++) {
        num_sym_errors += (msg_org[i] == msg_dec[i]) ? 0 : 1;

        num_bit_errors += count_bit_errors(msg_org[i], msg_dec[i]);
    }

    //printf("number of symbol errors detected: %d\n", num_errors_detected);
    printf("number of symbol errors received: %4u / %4u\n", num_sym_errors, n);
    printf("number of bit errors received:    %4u / %4u\n", num_bit_errors, n*8);

    if (crc_pass)
        printf("(crc passed)\n");
    else
        printf("(crc failed)\n");

    return 0;
}

