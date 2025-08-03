//
// dsssframesync_example.c
//

#include <assert.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "liquid.h"

void usage()
{
    printf("dsssframesync_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  s     : signal-to-noise ratio [dB], default: 20\n");
    printf("  F     : carrier frequency offset, default: 0.01\n");
    printf("  n     : payload length [bytes], default: 480\n");
    printf("  v     : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf("  c     : coding scheme (inner): h74 default\n");
    printf("  k     : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
    printf("  d     : enable debugging\n");
}

// dsssframesync callback function
static int callback(unsigned char *  _header,
                    int              _header_valid,
                    unsigned char *  _payload,
                    unsigned int     _payload_len,
                    int              _payload_valid,
                    framesyncstats_s _stats,
                    void *           _userdata);

int main(int argc, char * argv[])
{
    srand(time(NULL));

    // options
    crc_scheme   check         = LIQUID_CRC_32;   // data validity check
    fec_scheme   fec0          = LIQUID_FEC_NONE; // fec (inner)
    fec_scheme   fec1          = LIQUID_FEC_NONE; // fec (outer)
    unsigned int payload_len   = 20;              // payload length
    int          debug_enabled = 0;               // enable debugging?
    float        noise_floor   = -60.0f;          // noise floor
    float        SNRdB         = -3.0f;           // signal-to-noise ratio
    float        dphi          = 0.03f;           // carrier frequency offset

    // get options
    int dopt;
    while ((dopt = getopt(argc, argv, "uhs:F:n:m:v:c:k:d")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage(); return 0;
        case 's': SNRdB = atof(optarg); break;
        case 'F': dphi = atof(optarg); break;
        case 'n': payload_len = atol(optarg); break;
        case 'v': check = liquid_getopt_str2crc(optarg); break;
        case 'c': fec0 = liquid_getopt_str2fec(optarg); break;
        case 'k': fec1 = liquid_getopt_str2fec(optarg); break;
        case 'd': debug_enabled = 1; break;
        default: exit(-1);
        }
    }

    // derived values
    unsigned int i;
    float        nstd  = powf(10.0f, noise_floor / 20.0f);           // noise std. dev.
    float        gamma = powf(10.0f, (SNRdB + noise_floor) / 20.0f); // channel gain

    // create dsssframegen object
    dsssframegenprops_s fgprops;
    fgprops.check   = check;
    fgprops.fec0    = fec0;
    fgprops.fec1    = fec1;
    dsssframegen fg = dsssframegen_create(&fgprops);

    // create dsssframesync object
    dsssframesync fs = dsssframesync_create(callback, NULL);
    if (debug_enabled) {
        // dsssframesync_debug_enable(fs);
    }

    // assemble the frame (NULL pointers for default values)
    dsssframegen_assemble(fg, NULL, NULL, payload_len);
    // dsssframegen_print(fg);

    // generate the frame in blocks
    unsigned int buf_len  = 256;
    float complex x[buf_len];
    float complex y[buf_len];

    int   frame_complete = 0;
    float phi            = 0.0f;
    while (!frame_complete) {
        frame_complete = dsssframegen_write_samples(fg, x, buf_len);

        // add noise and push through synchronizer
        for (i = 0; i < buf_len; i++) {
            // apply channel gain and carrier offset to input
            y[i] = gamma * x[i] * cexpf(_Complex_I * phi);
            phi += dphi;

            // add noise
            y[i] += nstd * (randnf() + _Complex_I * randnf()) * M_SQRT1_2;
        }

        // run through frame synchronizer
        dsssframesync_execute(fs, y, buf_len);
    }
    dsssframesync_print(fs);

    // export debugging file
    if (debug_enabled) {
        // dsssframesync_debug_print(fs, "dsssframesync_debug.m");
    }

    // dsssframesync_print(fs);
    // destroy allocated objects
    dsssframegen_destroy(fg);
    dsssframesync_destroy(fs);

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
    for (i = 0; i < _payload_len; i++)
        bit_errors += liquid_count_ones(_payload[i]);

    framesyncstats_print(&_stats);
    printf("    header crc          :   %s\n", _header_valid ? "pass" : "FAIL");
    printf("    payload length      :   %u\n", _payload_len);
    printf("    payload crc         :   %s\n", _payload_valid ? "pass" : "FAIL");
    printf("    payload bit errors  :   %u / %u\n", bit_errors, 8 * _payload_len);

    return 0;
}
