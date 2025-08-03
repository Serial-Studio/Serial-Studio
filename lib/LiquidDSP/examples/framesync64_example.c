// This example demonstrates the interfaces to the framegen64 and
// framesync64 objects used to completely encapsulate data for
// over-the-air transmission.  A 64-byte payload is generated, and then
// encoded, modulated, and interpolated using the framegen64 object.
// The resulting complex baseband samples are corrupted with noise and
// moderate carrier frequency and phase offsets before the framesync64
// object attempts to decode the frame.  The resulting data are compared
// to the original to validate correctness.
//
// SEE ALSO: flexframesync_example.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "liquid.h"
#define OUTPUT_FILENAME  "framesync64_example.m"

// static callback function
static int callback(unsigned char *  _header,
                    int              _header_valid,
                    unsigned char *  _payload,
                    unsigned int     _payload_len,
                    int              _payload_valid,
                    framesyncstats_s _stats,
                    void *           _userdata)
{
    printf("*** callback invoked ***\n");
    framesyncstats_print(&_stats);
    return 0;
}

int main(int argc, char*argv[])
{
    // create frame generator, synchronizer objects
    framegen64  fg = framegen64_create();
    framesync64 fs = framesync64_create(callback,NULL);

    // create buffer for the frame samples
    unsigned int frame_len = LIQUID_FRAME64_LEN; // fixed frame length
    float complex frame[frame_len];
    
    // generate the frame with random header and payload
    framegen64_execute(fg, NULL, NULL, frame);

    // add minor channel effects
    unsigned int i;
    for (i=0; i<frame_len; i++) {
        frame[i] *= cexpf(_Complex_I*(0.01f*i + 1.23456f));
        frame[i] *= 0.1f;
        frame[i] += 0.02f*( randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }

    // synchronize/receive the frame
    framesync64_execute(fs, frame, frame_len);
    framesync64_print(fs);

    // clean up allocated objects
    framegen64_destroy(fg);
    framesync64_destroy(fs);
    
    // export results
    FILE* fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s: auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"frame_len   = %u;\n", frame_len);
    for (i=0; i<frame_len; i++)
        fprintf(fid,"y(%4u)=%12.4e+j*%12.4e;\n", i+1, crealf(frame[i]), cimagf(frame[i]));
    fprintf(fid,"t=0:(length(y)-1);\n");
    fprintf(fid,"plot(t,real(y),t,imag(y));\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);
    printf("done.\n");
    return 0;
}
