//
// flexframesync_reconfig_example.c
//
// Demonstrates the reconfigurability of the flexframegen and
// flexframesync objects.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME  "flexframesync_reconfig_example.m"

void usage()
{
    printf("flexframesync_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  s     : signal-to-noise ratio [dB], default: 30\n");
    printf("  n     : number of frames, default: 3\n");
}

int main(int argc, char *argv[]) {
    srand( time(NULL) );

    // define parameters
    float SNRdB = 30.0f;
    float noise_floor = -30.0f;
    unsigned int num_frames = 3;

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"uhvqs:f:m:p:n:")) != EOF){
        switch (dopt) {
        case 'u':
        case 'h': usage();                      return 0;
        case 's': SNRdB = atof(optarg);         break;
        case 'n': num_frames = atoi(optarg);    break;
        default:
            exit(1);
        }
    }

    // create flexframegen object
    flexframegenprops_s fgprops;
    flexframegenprops_init_default(&fgprops);
    flexframegen fg = flexframegen_create(NULL);

    // frame data
    unsigned char   header[14];
    unsigned char * payload = NULL;

    // create flexframesync object with default properties
    flexframesync fs = flexframesync_create(NULL,NULL);

    // channel
    float nstd  = powf(10.0f, noise_floor/20.0f);         // noise std. dev.
    float gamma = powf(10.0f, (SNRdB+noise_floor)/20.0f); // channel gain

    unsigned int i;
    // initialize header, payload
    for (i=0; i<14; i++)
        header[i] = i;

    // frame buffers, properties
    unsigned int  buf_len = 256;
    float complex buf[buf_len];

    unsigned int j;
    for (j=0; j<num_frames; j++) {
        // configure frame generator properties
        unsigned int payload_len = (rand() % 256) + 1;   // random payload length
        fgprops.check            = LIQUID_CRC_NONE;      // data validity check
        fgprops.fec0             = LIQUID_FEC_NONE;      // inner FEC scheme
        fgprops.fec1             = LIQUID_FEC_NONE;      // outer FEC scheme
        fgprops.mod_scheme       = (rand() % 2) ? LIQUID_MODEM_QPSK : LIQUID_MODEM_QAM16;

        // reallocate memory for payload
        payload = realloc(payload, payload_len*sizeof(unsigned char));

        // initialize payload
        for (i=0; i<payload_len; i++)
            payload[i] = rand() & 0xff;

        // set properties and assemble the frame
        flexframegen_setprops(fg, &fgprops);
        flexframegen_assemble(fg, header, payload, payload_len);
        printf("frame %u, ", j);
        flexframegen_print(fg);

        // write the frame in blocks
        int frame_complete = 0;
        while (!frame_complete) {
            // write samples to buffer
            frame_complete = flexframegen_write_samples(fg, buf, buf_len);

            // add channel impairments (gain and noise)
            for (i=0; i<buf_len; i++)
                buf[i] = buf[i]*gamma + nstd * (randnf() + _Complex_I*randnf()) * M_SQRT1_2;

            // push through sync
            flexframesync_execute(fs, buf, buf_len);
        }

    } // num frames

    // print frame data statistics
    flexframesync_print(fs);

    // clean up allocated memory
    flexframegen_destroy(fg);
    flexframesync_destroy(fs);
    free(payload);

    printf("done.\n");
    return 0;
}

