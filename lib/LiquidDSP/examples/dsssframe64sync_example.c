// This example demonstrates the basic interface to the dsssframe64gen and
// dsssframe64sync objects.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <assert.h>

#include "liquid.h"

// static callback function
static int callback(unsigned char *  _header,
                    int              _header_valid,
                    unsigned char *  _payload,
                    unsigned int     _payload_len,
                    int              _payload_valid,
                    framesyncstats_s _stats,
                    void *           _context)
{
    printf("*** callback invoked (%s) ***\n", _payload_valid ? "pass" : "FAIL");
    framesyncstats_print(&_stats);

    // save recovered symbols to file
    unsigned int i;
    FILE * fid = (FILE*)_context;
    for (i=0; i<_stats.num_framesyms; i++) {
        fprintf(fid,"s(%3u) = %12.8f + %12.8fj;\n", i+1,
            crealf(_stats.framesyms[i]), cimagf(_stats.framesyms[i]));
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // options
    unsigned int nfft  = 2400;
    float        SNRdB =   -10.0f;
    const char * filename = "dsssframe64sync_example.m";

    // create dsssframe64gen object
    dsssframe64gen fg = dsssframe64gen_create();

    // generate the frame in blocks
    unsigned int  buf_len = dsssframe64gen_get_frame_len(fg);
    float complex * buf_tx = (float complex *)malloc(buf_len*sizeof(float complex));
    float complex * buf_rx = (float complex *)malloc(buf_len*sizeof(float complex));

    // export results to file
    FILE * fid = fopen(filename,"w");
    fprintf(fid,"%% %s : auto-generated file\n", filename);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"s=[];\n");

    // generate in one step (for now)
    dsssframe64gen_execute(fg, NULL, NULL, buf_tx);

    // apply channel (AWGN)
    float nstd = powf(10.0f,-SNRdB/20.0f);
    unsigned int i;
    for (i=0; i<buf_len; i++)
        buf_rx[i] = buf_tx[i]*M_SQRT1_2 + nstd*(randnf() + _Complex_I*randnf())/M_SQRT2;

    // run through sync
    dsssframe64sync fs = dsssframe64sync_create(callback, (void*)fid);
    dsssframe64sync_execute(fs, buf_rx, buf_len);
    dsssframe64sync_destroy(fs);

    // push resulting sample through periodogram
    spgramcf periodogram = spgramcf_create_default(nfft);
    spgramcf_write(periodogram, buf_rx, buf_len);
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);
    spgramcf_destroy(periodogram);

    // plot results
    fprintf(fid,"nfft=%u; Y=zeros(1,nfft);\n", nfft);
    for (i=0; i<nfft; i++)
        fprintf(fid,"Y(%4u) = %12.8f;\n", i+1, psd[i]);
    fprintf(fid,"figure('position',[100 100 1200 400]);\n");
    fprintf(fid,"subplot(1,5,1:3);\n");
    fprintf(fid,"  f=[0:(nfft-1)]/nfft-0.5; plot(f,Y); xlim([-0.5 0.5]); grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/Fs]'); ylabel('PSD [dB]');\n");
    fprintf(fid,"subplot(1,5,4:5);\n");
    fprintf(fid,"  plot(real(s),imag(s),'.','MarkerSize',6); grid on; axis([-1 1 -1 1]*1.5)\n");
    fprintf(fid,"  axis('square'); xlabel('I'); ylabel('Q');\n");
    fclose(fid);
    printf("results written to %s\n", filename);

    // destroy allocated objects and free memory
    dsssframe64gen_destroy(fg);
    free(buf_tx);
    free(buf_rx);
    return 0;
}

