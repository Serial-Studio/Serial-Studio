// Generate continuous signals, decompose with analysis channelizer, show spectra
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firpfbch_crcf_msource_example.m"

int main() {
    // options
    unsigned int M          =  5;     // number of channels
    unsigned int m          = 12;     // filter delay
    unsigned int num_samples= 512000; // number of samples to generate
    unsigned int nfft       = 1200;   // FFT size for analysis

    // data arrays
    unsigned int i;
    float complex buf_0[M]; // time-domain input
    float complex buf_1[M]; // channelized output

    // create filterbank channelizer object using external filter coefficients
    firpfbch_crcf q = firpfbch_crcf_create_rnyquist(LIQUID_ANALYZER, M, m, 0.5f,
            LIQUID_FIRFILT_ARKAISER);

    // create multi-signal source generator
    msourcecf gen = msourcecf_create_default();
    // add signals     (gen,  fc,   bw,    gain, {options})
    msourcecf_add_noise(gen,  0.00f,1.00f, -40);               // noise floor
    msourcecf_add_noise(gen,  0.02f,0.05f,   0);               // narrow-band noise
    msourcecf_add_tone (gen, -0.40f,0.00f,  20);               // tone
    msourcecf_add_modem(gen,  0.20f,0.10f,   0, LIQUID_MODEM_QPSK, 12, 0.2f);  // modulated data (linear)
    msourcecf_add_gmsk (gen, -0.20f,0.05f,   0, 4, 0.3f);        // modulated data (GMSK)

    // create objects for computing spectra
    spgramcf psd_0 = spgramcf_create_default(nfft);
    spgramcf psd_1[M];
    for (i=0; i<M; i++)
        psd_1[i] = spgramcf_create_default(nfft);

    // generate signals
    while (msourcecf_get_num_samples(gen) < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf_0, M);

        // run through channelizer
        firpfbch_crcf_analyzer_execute(q, buf_0, buf_1);

        // push resulting sample through periodogram objects
        spgramcf_write(psd_0, buf_0, M);
        for (i=0; i<M; i++)
            spgramcf_push(psd_1[i], buf_1[i]);
    }

    // export results to file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"M = %u;\n", M);
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"%% input signal spectrum\n");
    fprintf(fid,"X = zeros(1,nfft);\n");
    float psd[nfft];
    spgramcf_get_psd(psd_0, psd);
    for (i=0; i<nfft; i++)
        fprintf(fid,"  X(%6u) = %12.4e;\n", i+1, psd[i]);
    fprintf(fid,"%% channelized output signal spectra\n");
    fprintf(fid,"Y = zeros(M, nfft);\n");
    // save channelized output signals
    for (i=0; i<M; i++) {
        spgramcf_get_psd(psd_1[i], psd);
        fprintf(fid,"Y(%u,:) = [", i+1);
        unsigned int k;
        for (k=0; k<nfft; k++) {
            fprintf(fid,"%6g,", psd[k]);
        }
        fprintf(fid,"];\n");
    }

    // plot results
    fprintf(fid,"\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"figure('color','white','position',[100 100 1200 600]);\n");
    fprintf(fid,"subplot(2,M,1:M);\n");
    fprintf(fid,"  plot(f, X, 'Color', [0 0.5 0.25], 'LineWidth', 2);\n");
    fprintf(fid,"  axis([-0.5 0.5 -50 30]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('Input PSD [dB]');\n");

    fprintf(fid,"for i=1:M\n");
    fprintf(fid,"  k = mod((i-1) + ceil(M/2),M) + 1;\n");
    fprintf(fid,"  subplot(2,M,M+i);\n");
    fprintf(fid,"    plot(f, Y(k,:)-10*log10(M),'Color',[0.25 0 0.25],'LineWidth', 1.5);\n");
    fprintf(fid,"    axis([-0.5 0.5 -50 30]);\n");
    fprintf(fid,"    grid on;\n");
    fprintf(fid,"    title(['Channel ' num2str(k-1)]);\n");
    fprintf(fid,"end;\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    // destroy objects
    msourcecf_destroy(gen);
    firpfbch_crcf_destroy(q);
    spgramcf_destroy(psd_0);
    for (i=0; i<M; i++)
        spgramcf_destroy(psd_1[i]);

    printf("done.\n");
    return 0;
}

