//
// fec_example.c
//
// This example demonstrates the interface for forward error-
// correction (FEC) codes.  A buffer of data bytes is encoded and
// corrupted with several errors.  The decoder then attempts to
// recover the original data set.  The user may select the FEC
// scheme from the command-line interface.
// SEE ALSO: crc_example.c
//           checksum_example.c
//           packetizer_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "liquid.h"

// print usage/help message
void usage()
{
    printf("fec_example [options]\n");
    printf("  u/h   : print usage\n");
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

    int dopt;
    while((dopt = getopt(argc,argv,"uhn:c:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage(); return 0;
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
    unsigned char data[n];          // original data message
    unsigned char msg_enc[n_enc];   // encoded data message
    unsigned char msg_cor[n_enc];   // corrupted data message
    unsigned char msg_dec[n];       // decoded data message

    // create object
    fec q = fec_create(fs,NULL);
    fec_print(q);

    unsigned int i;

    // create message
    for (i=0; i<n; i++)
        data[i] = rand() & 0xff;

    // encode message
    fec_encode(q, n, data, msg_enc);

    // corrupt encoded message
    memmove(msg_cor, msg_enc, n_enc);
    msg_cor[0] ^= 0x04; // position 5
#if 0
    msg_cor[1] ^= 0x04; //
    msg_cor[2] ^= 0x02; //
    msg_cor[3] ^= 0x01; //
    msg_cor[4] ^= 0x80; //
    msg_cor[5] ^= 0x40; //
    msg_cor[6] ^= 0x20; //
    msg_cor[7] ^= 0x10; //
#endif

    // decode message
    fec_decode(q, n, msg_cor, msg_dec);

    printf("original message:  [%3u] ",n);
    for (i=0; i<n; i++)
        printf(" %.2X", (unsigned int) (data[i]));
    printf("\n");

    printf("encoded message:   [%3u] ",n_enc);
    for (i=0; i<n_enc; i++)
        printf(" %.2X", (unsigned int) (msg_enc[i]));
    printf("\n");

    printf("corrupted message: [%3u] ",n_enc);
    for (i=0; i<n_enc; i++)
        printf("%c%.2X", msg_cor[i]==msg_enc[i] ? ' ' : '*', (unsigned int) (msg_cor[i]));
    printf("\n");

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

