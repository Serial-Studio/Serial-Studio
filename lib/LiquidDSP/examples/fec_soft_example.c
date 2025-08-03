//
// fec_soft_example.c
//
// This example demonstrates the interface for forward error-correction
// (FEC) codes with soft-decision decoding.  A buffer of data bytes is
// encoded before the data are corrupted with at least one error and
// noise. The decoder then attempts to recover the original data set
// from the soft input bits.  The user may select the FEC scheme from
// the command-line interface.
//
// SEE ALSO: fec_example.c
//           packetizer_soft_example.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "liquid.h"

// print usage/help message
void usage()
{
    printf("fecsoft_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  v/q   : verbose/queit (print soft bits?)\n");
    printf("  n     : input data size (number of uncoded bytes)\n");
    printf("  c     : coding scheme, (h74 default):\n");
    liquid_print_fec_schemes();
}


int main(int argc, char*argv[])
{
    // options
    unsigned int n = 4;                     // data length (bytes)
    unsigned int nmax = 2048;               // maximum data length
    fec_scheme fs = LIQUID_FEC_HAMMING74;   // error-correcting scheme
    int verbose = 1;                        // verbose?

    int dopt;
    while((dopt = getopt(argc,argv,"uhvqn:c:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage();          return 0;
        case 'v': verbose = 1;      break;
        case 'q': verbose = 0;      break;
        case 'n': n = atoi(optarg); break;
        case 'c':
            fs = liquid_getopt_str2fec(optarg);
            if (fs == LIQUID_FEC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported fec scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    // ensure proper data length
    n = (n > nmax) ? nmax : n;

    // create arrays
    unsigned int n_enc = fec_get_enc_msg_length(fs,n);
    printf("dec msg len : %u\n", n);
    printf("enc msg len : %u\n", n_enc);
    unsigned char data[n];               // original data message
    unsigned char msg_enc[n_enc];        // encoded data message
    unsigned char msg_cor_soft[8*n_enc]; // corrupted data message (soft bits)
    unsigned char msg_cor_hard[n_enc];   // corrupted data message (hard bits)
    unsigned char msg_dec[n];            // decoded data message

    // create object
    fec q = fec_create(fs,NULL);
    fec_print(q);

    unsigned int i;

    // create message
    for (i=0; i<n; i++)
        data[i] = rand() & 0xff;

    // encode message
    fec_encode(q, n, data, msg_enc);

    // convert to soft bits and add 'noise'
    for (i=0; i<n_enc; i++) {
        msg_cor_soft[8*i+0] = (msg_enc[i] & 0x80) ? 255 : 0;
        msg_cor_soft[8*i+1] = (msg_enc[i] & 0x40) ? 255 : 0;
        msg_cor_soft[8*i+2] = (msg_enc[i] & 0x20) ? 255 : 0;
        msg_cor_soft[8*i+3] = (msg_enc[i] & 0x10) ? 255 : 0;
        msg_cor_soft[8*i+4] = (msg_enc[i] & 0x08) ? 255 : 0;
        msg_cor_soft[8*i+5] = (msg_enc[i] & 0x04) ? 255 : 0;
        msg_cor_soft[8*i+6] = (msg_enc[i] & 0x02) ? 255 : 0;
        msg_cor_soft[8*i+7] = (msg_enc[i] & 0x01) ? 255 : 0;
    }

    // flip first bit (ensure error)
    msg_cor_soft[0] = 255 - msg_cor_soft[0];

    // add noise (but not so much that it would cause a bit error)
    for (i=0; i<8*n_enc; i++) {
        int soft_bit = 0.8*msg_cor_soft[i] + (int)(20*randnf());
        if (soft_bit > 255) soft_bit = 255;
        if (soft_bit <   0) soft_bit = 0;
        msg_cor_soft[i] = soft_bit;
    }

    // convert to hard bits (printing purposes)
    for (i=0; i<n_enc; i++) {
        msg_cor_hard[i] = 0x00;

        msg_cor_hard[i] |=(msg_cor_soft[8*i+0] >> 0) & 0x80;
        msg_cor_hard[i] |=(msg_cor_soft[8*i+1] >> 1) & 0x40;
        msg_cor_hard[i] |=(msg_cor_soft[8*i+2] >> 2) & 0x20;
        msg_cor_hard[i] |=(msg_cor_soft[8*i+3] >> 3) & 0x10;
        msg_cor_hard[i] |=(msg_cor_soft[8*i+4] >> 4) & 0x08;
        msg_cor_hard[i] |=(msg_cor_soft[8*i+5] >> 5) & 0x04;
        msg_cor_hard[i] |=(msg_cor_soft[8*i+6] >> 6) & 0x02;
        msg_cor_hard[i] |=(msg_cor_soft[8*i+7] >> 7) & 0x01;
    }

    // decode message
    fec_decode_soft(q, n, msg_cor_soft, msg_dec);

    printf("original message:  [%3u] ",n);
    for (i=0; i<n; i++)
        printf(" %.2X", (unsigned int) (data[i]));
    printf("\n");

    printf("encoded message:   [%3u] ",n_enc);
    for (i=0; i<n_enc; i++)
        printf(" %.2X", (unsigned int) (msg_enc[i]));
    printf("\n");

    // print compact result
    printf("corrupted message: [%3u] ",n_enc);
    for (i=0; i<n_enc; i++)
        printf("%c%.2X", msg_cor_hard[i]==msg_enc[i] ? ' ' : '*', (unsigned int) (msg_cor_hard[i]));
    printf("\n");

    if (verbose) {
        // print expanded result (print each soft bit value)
        for (i=0; i<n_enc; i++) {
            printf("%5u: ", i);
            unsigned int j;
            for (j=0; j<8; j++) {
                unsigned int bit_enc = (msg_enc[i] >> (8-j-1)) & 0x01;
                unsigned int bit_rec = (msg_cor_soft[8*i+j] > 127) ? 1 : 0;
                //printf("%1u %3u (%1u) %c", bit_enc, msg_cor_soft[i], bit_rec, bit_enc != bit_rec ? '*' : ' ');
                printf("%4u%c", msg_cor_soft[8*i+j], bit_enc != bit_rec ? '*' : ' ');
            }
            printf("  :  %c%.2X\n", msg_cor_hard[i]==msg_enc[i] ? ' ' : '*', (unsigned int) (msg_cor_hard[i]));
        }
    } // verbose

    printf("decoded message:   [%3u] ",n);
    for (i=0; i<n; i++)
        printf("%c%.2X", msg_dec[i] == data[i] ? ' ' : '*', (unsigned int) (msg_dec[i]));
    printf("\n");
    printf("\n");

    // count bit errors
    unsigned int j, num_sym_errors=0, num_bit_errors=0;
    unsigned char e;
    for (i=0; i<n; i++) {
        num_sym_errors += (data[i] == msg_dec[i]) ? 0 : 1;

        e = data[i] ^ msg_dec[i];
        for (j=0; j<8; j++) {
            num_bit_errors += e & 0x01;
            e >>= 1;
        }
    }

    //printf("number of symbol errors detected: %d\n", num_errors_detected);
    printf("number of symbol errors received: %3u / %3u\n", num_sym_errors, n);
    printf("number of bit errors received:    %3u / %3u\n", num_bit_errors, n*8);

    // clean up objects
    fec_destroy(q);

    return 0;
}

