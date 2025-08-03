//
// interleaver_soft_example.c
//

#include <stdio.h>
#include <stdlib.h> // for rand()

#include "liquid.h"

int main() {
    // options
    unsigned int n=9; // message length (bits)

    // create the interleaver
    interleaver q = interleaver_create(n);
    interleaver_print(q);

    // create arrays
    unsigned char msg_org[n];   // original message data (bytes)
    unsigned char msg_int[n];   // interleaved data (bytes)
    unsigned char msg_rec[8*n]; // interleaved data (soft bits)
    unsigned char msg_dec[8*n]; // de-interleaved data (soft bits)
    unsigned char msg_ppp[n];   // de-interleaved data (bytes)

    // generate random data sequence
    unsigned int i;
    for (i=0; i<n; i++)
        msg_org[i] = rand()%256;

    // interleave the data
    interleaver_encode(q, msg_org, msg_int);

    // expand to 'soft' bits
    for (i=0; i<n; i++) {
        msg_rec[8*i+0] = ( (msg_int[i] >> 7) & 0x01 ) ? 255 : 0;
        msg_rec[8*i+1] = ( (msg_int[i] >> 6) & 0x01 ) ? 255 : 0;
        msg_rec[8*i+2] = ( (msg_int[i] >> 5) & 0x01 ) ? 255 : 0;
        msg_rec[8*i+3] = ( (msg_int[i] >> 4) & 0x01 ) ? 255 : 0;
        msg_rec[8*i+4] = ( (msg_int[i] >> 3) & 0x01 ) ? 255 : 0;
        msg_rec[8*i+5] = ( (msg_int[i] >> 2) & 0x01 ) ? 255 : 0;
        msg_rec[8*i+6] = ( (msg_int[i] >> 1) & 0x01 ) ? 255 : 0;
        msg_rec[8*i+7] = ( (msg_int[i] >> 0) & 0x01 ) ? 255 : 0;
    }

    // de-interleave the soft bits
    interleaver_decode_soft(q, msg_rec, msg_dec);

    // pack the bits
    for (i=0; i<n; i++) {
        msg_ppp[i] = 0x00;

        msg_ppp[i] |= (msg_dec[8*i+0] > 127) ? 0x80 : 0;
        msg_ppp[i] |= (msg_dec[8*i+1] > 127) ? 0x40 : 0;
        msg_ppp[i] |= (msg_dec[8*i+2] > 127) ? 0x20 : 0;
        msg_ppp[i] |= (msg_dec[8*i+3] > 127) ? 0x10 : 0;
        msg_ppp[i] |= (msg_dec[8*i+4] > 127) ? 0x08 : 0;
        msg_ppp[i] |= (msg_dec[8*i+5] > 127) ? 0x04 : 0;
        msg_ppp[i] |= (msg_dec[8*i+6] > 127) ? 0x02 : 0;
        msg_ppp[i] |= (msg_dec[8*i+7] > 127) ? 0x01 : 0;
    }

    // compute errors and print results
    unsigned int num_errors=0;
    printf("%6s %6s %6s\n", "org", "int", "dec");
    for (i=0; i<n; i++) {
        unsigned char byte_int = 0x00;
        byte_int |= (msg_rec[8*i+0] > 127) ? 0x80 : 0;
        byte_int |= (msg_rec[8*i+1] > 127) ? 0x40 : 0;
        byte_int |= (msg_rec[8*i+2] > 127) ? 0x20 : 0;
        byte_int |= (msg_rec[8*i+3] > 127) ? 0x10 : 0;
        byte_int |= (msg_rec[8*i+4] > 127) ? 0x08 : 0;
        byte_int |= (msg_rec[8*i+5] > 127) ? 0x04 : 0;
        byte_int |= (msg_rec[8*i+6] > 127) ? 0x02 : 0;
        byte_int |= (msg_rec[8*i+7] > 127) ? 0x01 : 0;

        printf("%6u %6u %6u\n", msg_org[i], byte_int, msg_ppp[i]);
        //printf("y[%u] = %u\n", i, (unsigned int) (y[i]));
        //printf("y[%u] = %#0x\n", i, (unsigned int) (y[i]));
        num_errors += (msg_org[i] == msg_ppp[i]) ? 0 : 1;
    }
    printf("errors: %u / %u\n", num_errors, n);

    // destroy the interleaver object
    interleaver_destroy(q);

    printf("done.\n");
    return 0;
}

