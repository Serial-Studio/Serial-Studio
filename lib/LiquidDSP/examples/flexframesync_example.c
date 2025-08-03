//
// flexframesync_example.c
//
// This example demonstrates the basic interface to the flexframegen and
// flexframesync objects used to completely encapsulate raw data bytes
// into frame samples (nearly) ready for over-the-air transmission. A
// 14-byte header and variable length payload are encoded into baseband
// symbols using the flexframegen object.  The resulting symbols are
// interpolated using a root-Nyquist filter and the resulting samples are
// then fed into the flexframesync object which attempts to decode the
// frame. Whenever frame is found and properly decoded, its callback
// function is invoked.
//
// SEE ALSO: flexframesync_reconfig_example.c
//           framesync64_example.c
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
    printf("flexframesync_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  s     : signal-to-noise ratio [dB], default: 20\n");
    printf("  F     : carrier frequency offset, default: 0.01\n");
    printf("  n     : payload length [bytes], default: 480\n");
    printf("  m     : modulation scheme (qpsk default)\n");
    liquid_print_modulation_schemes();
    printf("  v     : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf("  c     : coding scheme (inner): h74 default\n");
    printf("  k     : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
    printf("  d     : enable debugging\n");
}

// flexframesync callback function
static int callback(unsigned char *  _header,
                    int              _header_valid,
                    unsigned char *  _payload,
                    unsigned int     _payload_len,
                    int              _payload_valid,
                    framesyncstats_s _stats,
                    void *           _userdata);

int main(int argc, char *argv[])
{
    //srand( time(NULL) );

    // options
    modulation_scheme ms     =  LIQUID_MODEM_QPSK; // mod. scheme
    crc_scheme check         =  LIQUID_CRC_32;     // data validity check
    fec_scheme fec0          =  LIQUID_FEC_NONE;   // fec (inner)
    fec_scheme fec1          =  LIQUID_FEC_NONE;   // fec (outer)
    unsigned int payload_len =  480;               // payload length
    int debug_enabled        =  0;                 // enable debugging?
    float noise_floor        = -60.0f;             // noise floor
    float SNRdB              =  20.0f;             // signal-to-noise ratio
    float dphi               =  0.01f;             // carrier frequency offset

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"uhs:F:n:m:v:c:k:d")) != EOF){
        switch (dopt) {
        case 'u':
        case 'h': usage();                                       return 0;
        case 's': SNRdB         = atof(optarg);                  break;
        case 'F': dphi          = atof(optarg);                  break;
        case 'n': payload_len   = atol(optarg);                  break;
        case 'm': ms            = liquid_getopt_str2mod(optarg); break;
        case 'v': check         = liquid_getopt_str2crc(optarg); break;
        case 'c': fec0          = liquid_getopt_str2fec(optarg); break;
        case 'k': fec1          = liquid_getopt_str2fec(optarg); break;
        case 'd': debug_enabled = 1;                             break;
        default:
            exit(-1);
        }
    }

    // derived values
    unsigned int i;
    float nstd  = powf(10.0f, noise_floor/20.0f);         // noise std. dev.
    float gamma = powf(10.0f, (SNRdB+noise_floor)/20.0f); // channel gain

    // create flexframegen object
    flexframegenprops_s fgprops;
    flexframegenprops_init_default(&fgprops);
    fgprops.mod_scheme  = ms;
    fgprops.check       = check;
    fgprops.fec0        = fec0;
    fgprops.fec1        = fec1;
    flexframegen fg = flexframegen_create(&fgprops);

    // create flexframesync object
    flexframesync fs = flexframesync_create(callback,NULL);
    if (debug_enabled)
        flexframesync_debug_enable(fs);

    // assemble the frame (NULL pointers for default values)
    flexframegen_assemble(fg, NULL, NULL, payload_len);
    flexframegen_print(fg);

    // generate the frame in blocks
    unsigned int  buf_len = 256;
    float complex x[buf_len];
    float complex y[buf_len];

    int frame_complete = 0;
    float phi = 0.0f;
    while (!frame_complete) {
        // write samples to buffer
        frame_complete = flexframegen_write_samples(fg, x, buf_len);

        // add noise and push through synchronizer
        for (i=0; i<buf_len; i++) {
            // apply channel gain and carrier offset to input
            y[i] = gamma * x[i] * cexpf(_Complex_I*phi);
            phi += dphi;

            // add noise
            y[i] += nstd*( randnf() + _Complex_I*randnf())*M_SQRT1_2;
        }

        // run through frame synchronizer
        flexframesync_execute(fs, y, buf_len);
    }

    // export debugging file
    if (debug_enabled)
        flexframesync_debug_print(fs, "flexframesync_debug.m");

    flexframesync_print(fs);
    // destroy allocated objects
    flexframegen_destroy(fg);
    flexframesync_destroy(fs);

    printf("done.\n");
    return 0;
}

static int callback(unsigned char *  _header,
                    int              _header_valid,
                    unsigned char *  _payload,
                    unsigned int     _payload_len,
                    int              _payload_valid,
                    framesyncstats_s _stats,
                    void *           _userdata)
{
    printf("******** callback invoked\n");

    // count bit errors (assuming all-zero message)
    unsigned int bit_errors = 0;
    unsigned int i;
    for (i=0; i<_payload_len; i++)
        bit_errors += liquid_count_ones(_payload[i]);

    framesyncstats_print(&_stats);
    printf("    header crc          :   %s\n", _header_valid ?  "pass" : "FAIL");
    printf("    payload length      :   %u\n", _payload_len);
    printf("    payload crc         :   %s\n", _payload_valid ?  "pass" : "FAIL");
    printf("    payload bit errors  :   %u / %u\n", bit_errors, 8*_payload_len);

    return 0;
}

