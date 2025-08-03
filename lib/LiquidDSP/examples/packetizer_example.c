//
// packetizer_example.c
//
// Demonstrates the functionality of the packetizer object.  Data are encoded
// using two forward error-correction schemes (an inner and outer code) before
// data errors are introduced.  The decoder then tries to recover the original
// data message.
//
// SEE ALSO: fec_example.c
//           crc_example.c
//           packetizer_soft_example.c
//

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
    unsigned char msg_rec[k];   // received message
    unsigned char msg_dec[n];   // decoded message
    int crc_pass;

    // initialize original data message
    for (i=0; i<n; i++)
        msg_org[i] = rand() % 256;

    // encode packet
    packetizer_encode(p,msg_org,msg_enc);

    // add error(s)
    memmove(msg_rec, msg_enc, k*sizeof(unsigned char));
    msg_rec[0] ^= 0x01;

    // decode packet
    crc_pass =
    packetizer_decode(p,msg_rec,msg_dec);

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

    printf("received message:  [%3u] ",k);
    for (i=0; i<k; i++)
        printf("%c%.2X", msg_rec[i]==msg_enc[i] ? ' ' : '*', (unsigned int) (msg_rec[i]));
    printf("\n");

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

