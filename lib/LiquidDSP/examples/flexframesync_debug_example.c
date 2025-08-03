// This example exports the output constellation to file for debugging.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <assert.h>

#include "liquid.h"

// flexframesync callback function
static int callback(unsigned char *  _header,
                    int              _header_valid,
                    unsigned char *  _payload,
                    unsigned int     _payload_len,
                    int              _payload_valid,
                    framesyncstats_s _stats,
                    void *           _userdata)
{
    const char * filename = (const char*)_userdata;
    FILE * fid = fopen(filename,"w");
    if (fid == NULL) {
        printf("could not open '%s' for writing\n", filename);
        return 0;
    }
    unsigned int i;
    for (i=0; i<_stats.num_framesyms; i++)
        fprintf(fid,"%12.8f %12.8f\n", crealf(_stats.framesyms[i]), cimagf(_stats.framesyms[i]));
    fclose(fid);
    return 0;
}

int main(int argc, char *argv[])
{
    // options
    modulation_scheme ms     =  LIQUID_MODEM_QPSK; // mod. scheme
    crc_scheme check         =  LIQUID_CRC_32;     // data validity check
    fec_scheme fec0          =  LIQUID_FEC_NONE;   // fec (inner)
    fec_scheme fec1          =  LIQUID_FEC_NONE;   // fec (outer)
    unsigned int payload_len =  480;               // payload length
    const char * filename    = "flexframesync_debug_example.dat";

    // create flexframegen object
    flexframegenprops_s fgprops;
    flexframegenprops_init_default(&fgprops);
    fgprops.mod_scheme  = ms;
    fgprops.check       = check;
    fgprops.fec0        = fec0;
    fgprops.fec1        = fec1;
    flexframegen fg = flexframegen_create(&fgprops);

    // create flexframesync object
    flexframesync fs = flexframesync_create(callback,(void*)filename);

    // assemble the frame (NULL pointers for default values)
    flexframegen_assemble(fg, NULL, NULL, payload_len);

    // generate the frame in blocks
    unsigned int  buf_len = 256;
    float complex buf[buf_len];

    int frame_complete = 0;
    while (!frame_complete) {
        // write samples to buffer
        frame_complete = flexframegen_write_samples(fg, buf, buf_len);

        // run through frame synchronizer
        flexframesync_execute(fs, buf, buf_len);
    }

    // destroy allocated objects
    flexframegen_destroy(fg);
    flexframesync_destroy(fs);
    return 0;
}

