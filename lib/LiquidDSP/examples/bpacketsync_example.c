//
// bpacketsync_example.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#include "liquid.h"

int callback(unsigned char *  _payload,
             int              _payload_valid,
             unsigned int     _payload_len,
             framesyncstats_s _stats,
             void *           _userdata)
{
    printf("callback invoked, payload (%u bytes) : %s\n",
            _payload_len,
            _payload_valid ? "valid" : "INVALID!");

    // copy data if valid
    if (_payload_valid) {
        unsigned char * msg_dec = (unsigned char*) _userdata;

        memmove(msg_dec, _payload, _payload_len*sizeof(unsigned char));
    }

    return 0;
}

// print usage/help message
void usage()
{
    printf("bpacketsync_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  n     : input data size (number of uncoded bytes): 8 default\n");
    printf("  e     : bit error rate of channel, default: 0\n");
    printf("  v     : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf("  c     : coding scheme (inner): h74 default\n");
    printf("  k     : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
}


int main(int argc, char*argv[]) {
    srand(time(NULL));

    // options
    unsigned int n=8;                       // original data message length
    crc_scheme check = LIQUID_CRC_32;       // data integrity check
    fec_scheme fec0 = LIQUID_FEC_HAMMING74; // inner code
    fec_scheme fec1 = LIQUID_FEC_NONE;      // outer code
    float bit_error_rate = 0.0f;            // bit error rate

    // read command-line options
    int dopt;
    while((dopt = getopt(argc,argv,"uhn:e:v:c:k:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage(); return 0;
        case 'n': n = atoi(optarg);     break;
        case 'e': bit_error_rate = atof(optarg);     break;
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

    // validate input
    if (n == 0) {
        fprintf(stderr,"error: %s, packet length must be greater than zero\n", argv[0]);
        exit(1);
    } else if (bit_error_rate < 0.0f || bit_error_rate > 1.0f) {
        fprintf(stderr,"error: %s, channel bit error rate must be in [0,1]\n", argv[0]);
        exit(1);
    }

    // create packet generator
    bpacketgen pg = bpacketgen_create(0, n, check, fec0, fec1);
    bpacketgen_print(pg);

    unsigned int i;

    // compute packet length
    unsigned int k = bpacketgen_get_packet_len(pg);

    // initialize arrays
    unsigned char msg_org[n];   // original message
    unsigned char msg_enc[k];   // encoded message
    unsigned char msg_rec[k+1]; // received message
    unsigned char msg_dec[n];   // decoded message

    // create packet synchronizer
    bpacketsync ps = bpacketsync_create(0, callback, (void*)msg_dec);
    bpacketsync_print(ps);

    // initialize original data message
    for (i=0; i<n; i++)
        msg_org[i] = rand() % 256;

    // 
    // encode packet
    //
    bpacketgen_encode(pg,msg_org,msg_enc);

    // 
    // channel
    //
    // add delay
    msg_rec[0] = rand() & 0xff; // initialize first byte as random
    memmove(&msg_rec[1], msg_enc, k*sizeof(unsigned char));
    liquid_lbshift(msg_rec, (k+1)*sizeof(unsigned char), rand()%8); // random shift
    // add random errors
    for (i=0; i<k+1; i++) {
        unsigned int j;
        for (j=0; j<8; j++) {
            if (randf() < bit_error_rate)
                msg_rec[i] ^= 1 << (8-j-1);
        }
    }

    // 
    // run packet synchronizer
    //

    // push random bits through synchronizer
    for (i=0; i<100; i++)
        bpacketsync_execute_byte(ps, rand() & 0xff);

    // push packet through synchronizer
    for (i=0; i<k+1; i++)
        bpacketsync_execute_byte(ps, msg_rec[i]);

    // 
    // count errors
    //
    unsigned int num_bit_errors = 0;
    for (i=0; i<n; i++)
        num_bit_errors += count_bit_errors(msg_org[i], msg_dec[i]);
    printf("number of bit errors received:    %4u / %4u\n", num_bit_errors, n*8);

    // clean up allocated objects
    bpacketgen_destroy(pg);
    bpacketsync_destroy(ps);

    return 0;
}

