// 
// nco_crcf_mix_example.c
//
// This example demonstrates how to mix a signal down at complex
// baseband.
//
// SEE ALSO: nco_pll_example.c
//           nco_pll_modem_example.c
//

#include <stdio.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "nco_crcf_mix_example.m"

int main()
{
    // options
    int          type        = LIQUID_NCO;  // nco type
    float        fc          =      0.05f;  // signal offset frequency
    unsigned int num_samples =     240000;  // number of samples to run
    unsigned int nfft        =       1200;  // spectral periodogram FFT size

    // create stream generator and add some sources
    msourcecf gen = msourcecf_create_default();
    msourcecf_add_noise(gen,  0.0f, 1.00f, -60);               // wide-band noise
    msourcecf_add_noise(gen,  0.0f, 0.20f, -40);               // narrow-band noise
    msourcecf_add_tone (gen, -0.4f, 0.0f,  -20);               // tone
    msourcecf_add_modem(gen,  0.2f, 0.1f,  -30, LIQUID_MODEM_QPSK, 12, 0.25f); // modem

    // create the NCO object
    nco_crcf q = nco_crcf_create(type);
    nco_crcf_set_frequency(q, 2*M_PI*fc);
    nco_crcf_print(q);

    // create spectral periodograms
    spgramcf p0 = spgramcf_create_default(nfft);    // before mixing down
    spgramcf p1 = spgramcf_create_default(nfft);    // after mixing down

    //
    unsigned int  buf_len = 1024;
    float complex buf[buf_len];
    unsigned int  total_samples = 0;
    while (total_samples < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(p0, buf, buf_len);

        // mix signal down (in place)
        nco_crcf_mix_block_down(q, buf, buf, buf_len);

        // push resulting sample through periodogram
        spgramcf_write(p1, buf, buf_len);

        // accumulated samples
        total_samples += buf_len;
    }
    printf("total samples: %u\n", total_samples);

    // compute power spectral density output
    float psd0[nfft];
    float psd1[nfft];
    spgramcf_get_psd(p0, psd0);
    spgramcf_get_psd(p1, psd1);

    // destroy objects
    spgramcf_destroy(p0);
    spgramcf_destroy(p1);
    nco_crcf_destroy(q);
    msourcecf_destroy(gen);

    // export output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"f    = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"psd0 = zeros(1,nfft);\n");
    fprintf(fid,"psd1 = zeros(1,nfft);\n");
    unsigned int i;
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd0(%6u) = %12.4e; psd1(%6u) = %12.4e;\n", i+1, psd0[i], i+1, psd1[i]);
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  plot(f, psd0, '-', 'LineWidth',1.5,'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  plot(f, psd1, '-', 'LineWidth',1.5,'Color',[0 0.2 0.5]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"legend('Before mixing','After mixing');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-0.5 0.5 -70 -10]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);
    return 0;
}
