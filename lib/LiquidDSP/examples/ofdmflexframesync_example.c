//
// ofdmflexframesync_example.c
//
// Example demonstrating the OFDM flexible frame synchronizer.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid.h"

void usage()
{
    printf("ofdmflexframesync_example [options]\n");
    printf(" -h        : print usage\n");
    printf(" -s  <snr> : signal-to-noise ratio [dB], default: 20\n");
    printf(" -F <freq> : carrier frequency offset, default: 0.002\n");
    printf(" -M  <num> : number of subcarriers (must be even), default: 64\n");
    printf(" -C  <len> : cyclic prefix length, default: 16\n");
    printf(" -n  <len> : payload length [bytes], default: 120\n");
    printf(" -m  <mod> : modulation scheme (qpsk default)\n");
    liquid_print_modulation_schemes();
    printf(" -v  <crc> : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf(" -c  <fec> : coding scheme (inner): h74 default\n");
    printf(" -k  <fec> : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
    printf(" -d       : enable debugging\n");
}

// callback function
int callback(unsigned char *  _header,
             int              _header_valid,
             unsigned char *  _payload,
             unsigned int     _payload_len,
             int              _payload_valid,
             framesyncstats_s _stats,
             void *           _userdata);

int main(int argc, char*argv[])
{
    //srand(time(NULL));

    // options
    unsigned int      M           = 64;                 // number of subcarriers
    unsigned int      cp_len      = 16;                 // cyclic prefix length
    unsigned int      taper_len   = 4;                  // taper length
    unsigned int      payload_len = 120;                // length of payload (bytes)
    modulation_scheme ms          = LIQUID_MODEM_QPSK;  // modulation scheme
    fec_scheme        fec0        = LIQUID_FEC_NONE;    // inner code
    fec_scheme        fec1        = LIQUID_FEC_HAMMING128; // outer code
    crc_scheme        check       = LIQUID_CRC_32;      // validity check
    float             noise_floor = -80.0f;             // noise floor [dB]
    float             SNRdB       = 20.0f;              // signal-to-noise ratio [dB]
    float             dphi        = 0.02f;              // carrier frequency offset
    int               debug       =  0;                 // enable debugging?

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"uhds:F:M:C:n:m:v:c:k:")) != EOF){
        switch (dopt) {
        case 'u':
        case 'h': usage();                    return 0;
        case 'd': debug       = 1;            break;
        case 's': SNRdB       = atof(optarg); break;
        case 'F': dphi        = atof(optarg); break;
        case 'M': M           = atoi(optarg); break;
        case 'C': cp_len      = atoi(optarg); break;
        case 'n': payload_len = atol(optarg); break;
        case 'm': ms          = liquid_getopt_str2mod(optarg); break;
        case 'v': check       = liquid_getopt_str2crc(optarg); break;
        case 'c': fec0        = liquid_getopt_str2fec(optarg); break;
        case 'k': fec1        = liquid_getopt_str2fec(optarg); break;
        default:
            exit(-1);
        }
    }

    unsigned int i;

    // TODO : validate options

    // derived values
    unsigned int  buf_len = 256;
    float complex buf[buf_len]; // time-domain buffer

    // allocate memory for header, payload
    unsigned char header[8];
    unsigned char payload[payload_len];

    // create frame generator
    ofdmflexframegenprops_s fgprops;
    ofdmflexframegenprops_init_default(&fgprops);
    fgprops.check           = check;
    fgprops.fec0            = fec0;
    fgprops.fec1            = fec1;
    fgprops.mod_scheme      = ms;
    ofdmflexframegen fg = ofdmflexframegen_create(M, cp_len, taper_len, NULL, &fgprops);

    // create frame synchronizer
    ofdmflexframesync fs = ofdmflexframesync_create(M, cp_len, taper_len, NULL, callback, (void*)payload);
    if (debug)
        ofdmflexframesync_debug_enable(fs);

    // initialize header/payload and assemble frame
    for (i=0; i<8; i++)
        header[i] = i & 0xff;
    for (i=0; i<payload_len; i++)
        payload[i] = rand() & 0xff;
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
    ofdmflexframegen_print(fg);
    ofdmflexframesync_print(fs);

    // create channel and add impairments
    channel_cccf channel = channel_cccf_create();
    channel_cccf_add_awgn(channel, noise_floor, SNRdB);
    channel_cccf_add_carrier_offset(channel, dphi, 0.0f);

    // generate frame, push through channel
    int last_symbol=0;
    while (!last_symbol) {
        // generate symbol
        last_symbol = ofdmflexframegen_write(fg, buf, buf_len);

        // apply channel to buffer (in place)
        channel_cccf_execute_block(channel, buf, buf_len, buf);

        // push samples through synchronizer
        ofdmflexframesync_execute(fs, buf, buf_len);
    }

    // export debugging file
    if (debug)
        ofdmflexframesync_debug_print(fs, "ofdmflexframesync_debug.m");

    ofdmflexframesync_print(fs);
    // destroy objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);
    channel_cccf_destroy(channel);

    printf("done.\n");
    return 0;
}

// callback function
int callback(unsigned char *  _header,
             int              _header_valid,
             unsigned char *  _payload,
             unsigned int     _payload_len,
             int              _payload_valid,
             framesyncstats_s _stats,
             void *           _userdata)
{
    printf("**** callback invoked : rssi = %8.3f dB, evm = %8.3f dB, cfo = %8.5f\n", _stats.rssi, _stats.evm, _stats.cfo);

    unsigned int i;

    // print header data to standard output
    printf("  header rx  :");
    for (i=0; i<8; i++)
        printf(" %.2X", _header[i]);
    printf("\n");

    // print payload data to standard output
    printf("  payload rx :");
    for (i=0; i<_payload_len; i++) {
        printf(" %.2X", _payload[i]);
        if ( ((i+1)%26)==0 && i !=_payload_len-1 )
            printf("\n              ");
    }
    printf("\n");

    // count errors in received payload and print to standard output
    unsigned char * payload_tx = (unsigned char*) _userdata;
    unsigned int num_errors = count_bit_errors_array(_payload, payload_tx, _payload_len);
    printf("  bit errors : %u / %u\n", num_errors, 8*_payload_len);

    return 0;
}

