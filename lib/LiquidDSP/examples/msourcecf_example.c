//
// msourcecf_example.c
//
// This example demonstrates generating multiple signal sources simultaneously
// for testing using the msource (multi-source) family of objects.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "msourcecf_example.m"

// user-defined callback; generate tones
int callback(void *          _userdata,
             float complex * _v,
             unsigned int    _n)
{
    unsigned int * counter = (unsigned int*)_userdata;
    unsigned int i;
    for (i=0; i<_n; i++) {
        _v[i] = *counter==0 ? 1 : 0;
        *counter = (*counter+1) % 8;
    }
    return 0;
}

int main()
{
    // msource parameters
    int          ms     = LIQUID_MODEM_QPSK;    // linear modulation scheme
    unsigned int m      =    12;                // modulation filter semi-length
    float        beta   = 0.30f;                // modulation filter excess bandwidth factor
    float        bt     = 0.35f;                // GMSK filter bandwidth-time factor

    // spectral periodogram options
    unsigned int nfft        =   2400;  // spectral periodogram FFT size
    unsigned int num_samples =  48000;  // number of samples

    // create spectral periodogram
    spgramcf periodogram = spgramcf_create_default(nfft);

    unsigned int buf_len = 1024;
    float complex buf[buf_len];

    // create multi-signal source generator
    msourcecf gen = msourcecf_create_default();

    // add signals     (gen,  fc,   bw,    gain, {options})
    msourcecf_add_noise(gen,  0.0f, 1.00f, -40);               // wide-band noise
    msourcecf_add_noise(gen,  0.0f, 0.20f,   0);               // narrow-band noise
    msourcecf_add_tone (gen, -0.4f, 0.00f,  20);               // tone
    msourcecf_add_modem(gen,  0.2f, 0.10f,   0, ms, m, beta);  // modulated data (linear)
    msourcecf_add_gmsk (gen, -0.2f, 0.05f,   0, m, bt);        // modulated data (GMSK)
    unsigned int counter = 0;
    msourcecf_add_user (gen,  0.4f, 0.15f, -10, (void*)&counter, callback); // tones

    // print source generator object
    msourcecf_print(gen);

    unsigned int total_samples = 0;
    while (total_samples < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(periodogram, buf, buf_len);

        // accumulated samples
        total_samples += buf_len;
    }
    printf("total samples: %u\n", total_samples);

    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);

    // destroy objects
    msourcecf_destroy(gen);
    spgramcf_destroy(periodogram);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"f    = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"H    = zeros(1,nfft);\n");

    unsigned int i;
    for (i=0; i<nfft; i++)
        fprintf(fid,"H(%6u) = %12.4e;\n", i+1, psd[i]);

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f, H, '-', 'LineWidth',1.5);\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"grid on;\n");
    //fprintf(fid,"axis([-0.5 0.5 -60 40]);\n");
    fprintf(fid,"axis([-0.5 0.5 -80 40]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    return 0;
}

