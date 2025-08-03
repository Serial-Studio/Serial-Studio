//
// firpfbchr_crcf_example.c
//
// Example of the finite impulse response (FIR) polyphase filterbank
// (PFB) channelizer with an output rate independent of channel spacing.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firpfbchr_crcf_example.m"

int main(int argc, char*argv[])
{
    // options
    unsigned int M = 16;            // number of channels
    unsigned int P =  6;            // output decimation rate
    unsigned int m =  5;            // filter semi-length (symbols)
    unsigned int num_blocks=1<<16;  // number of symbols
    float As = 60.0f;               // filter stop-band attenuation
    
    unsigned int i;
    unsigned int channel_id = 3;

    // create filterbank objects from prototype
    firpfbchr_crcf qa = firpfbchr_crcf_create_kaiser(M, P, m, As);
    firpfbchr_crcf_print(qa);

    // create multi-signal source generator
    msourcecf gen = msourcecf_create_default();

    // add signals          (gen,  fc,    bw,    gain, {options})
    msourcecf_add_noise(gen,  0.00f, 1.0f, -60);   // wide-band noise
    msourcecf_add_noise(gen, -0.30f, 0.1f, -20);   // narrow-band noise
    msourcecf_add_tone (gen,  0.08f, 0.0f,   0);   // tone
    // modulated data
    msourcecf_add_modem(gen,
       (float)channel_id/(float)M, // center frequency
       0.080f,                     // bandwidth (symbol rate)
       -20,                        // gain
       LIQUID_MODEM_QPSK,          // modulation scheme
       12,                         // filter semi-length
       0.3f);                      // modem parameters

    // create spectral periodogoram
    unsigned int nfft = 2400;
    spgramcf     p0   = spgramcf_create_default(nfft);
    spgramcf     p1   = spgramcf_create_default(nfft);

    // run channelizer
    float complex buf_0[P];
    float complex buf_1[M];
    for (i=0; i<num_blocks; i++) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf_0, P);

        // run analysis filterbank
        firpfbchr_crcf_push   (qa, buf_0);
        firpfbchr_crcf_execute(qa, buf_1);

        // push results through periodograms
        spgramcf_write(p0, buf_0, P);
        spgramcf_push (p1, buf_1[channel_id]);
    }
    spgramcf_print(p0);
    spgramcf_print(p1);

    // compute power spectral density output
    float psd_0[nfft];
    float psd_1[nfft];
    spgramcf_get_psd(p0, psd_0);
    spgramcf_get_psd(p1, psd_1);

    // destroy objects
    firpfbchr_crcf_destroy(qa);
    msourcecf_destroy(gen);
    spgramcf_destroy(p0);
    spgramcf_destroy(p1);

    //
    // EXPORT DATA TO FILE
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"M = %u;\n", M);
    fprintf(fid,"P = %u;\n", P);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"ch = %u;\n", channel_id);
    fprintf(fid,"f  = [0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"f0 = f;\n");
    fprintf(fid,"f1 = f/P + %d/M;\n", channel_id < M/2 ? channel_id : (int)channel_id - (int)M);
    fprintf(fid,"X  = zeros(1,nfft);\n");
    fprintf(fid,"Y  = zeros(1,nfft);\n");

    // save input and output arrays
    for (i=0; i<nfft; i++) {
        fprintf(fid,"X(%4u) = %12.4e;\n", i+1, psd_0[i]);
        fprintf(fid,"Y(%4u) = %12.4e;\n", i+1, psd_1[i]);
    }

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"title('composite');\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(f0, X, '-', 'LineWidth',1.5,...\n");
    fprintf(fid,"       f1, Y, '-', 'LineWidth',1.5);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-0.5 0.5 -80 20]);\n");
    fprintf(fid,"  ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(f0, Y, '-', 'LineWidth',1.5);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-0.5 0.5 -80 20]);\n");
    fprintf(fid,"  ylabel('Power Spectral Density [dB]');\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
