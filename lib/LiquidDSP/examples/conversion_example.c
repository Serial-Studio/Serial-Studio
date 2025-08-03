//
// conversion_example.c
//
// This example demonstrates conversion from complex baseband to a real-valued
// signal, and then down-conversion back to complex baseband while removing the
// negative image.
//
//  STEP 1: A signal is generated at complex baseband consisting of narrow-band
//          filtered noise and an offset tone (to show asymmetry in the transmit
//          spectrum).
//
//  STEP 2: The signal is mixed up to a carrier 'fc' (relative to the sampling
//          frequency) and the real-component of the result is retained. This is
//          the DAC output. The spectrum of this signal has two images: one at
//          +fc, the other at -fc.
//
//  STEP 3: The DAC output is mixed back down to complex baseband and the lower
//          image is (mostly) filtered off. Reminants of the lower frequency
//          component are still visible due to the wide-band and low-order
//          filter on the receiver. The received complex baseband signal also
//          has a reduction in power by 2 because half the signal's energy (the
//          negative image) is filtered off.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "conversion_example.m"

int main()
{
    // spectral periodogram options
    unsigned int nfft        =   1200;  // spectral periodogram FFT size
    unsigned int num_samples =  64000;  // number of samples
    float        fc          =   0.20f; // carrier (relative to sampling rate)

    // create objects
    iirfilt_crcf   filter_tx    = iirfilt_crcf_create_lowpass(15, 0.05);
    nco_crcf       mixer_tx     = nco_crcf_create(LIQUID_VCO);
    nco_crcf       mixer_rx     = nco_crcf_create(LIQUID_VCO);
    iirfilt_crcf   filter_rx    = iirfilt_crcf_create_lowpass(7, 0.2);

    // set carrier frequencies
    nco_crcf_set_frequency(mixer_tx, fc * 2*M_PI);
    nco_crcf_set_frequency(mixer_rx, fc * 2*M_PI);

    // create objects for measuring power spectral density
    spgramcf spgram_tx  = spgramcf_create_default(nfft);
    spgramf  spgram_dac = spgramf_create_default(nfft);
    spgramcf spgram_rx  = spgramcf_create_default(nfft);

    // run through loop one step at a time
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        // STEP 1: generate input signal (filtered noise with offset tone)
        float complex v1 = (randnf() + randnf()*_Complex_I) + 3.0f*cexpf(-_Complex_I*0.2f*i);
        iirfilt_crcf_execute(filter_tx, v1, &v1);

        // save spectrum
        spgramcf_push(spgram_tx, v1);

        // STEP 2: mix signal up and save real part (DAC output)
        nco_crcf_mix_up(mixer_tx, v1, &v1);
        float v2 = crealf(v1);
        nco_crcf_step(mixer_tx);

        // save spectrum
        spgramf_push(spgram_dac, v2);

        // STEP 3: mix signal down and filter off image
        float complex v3;
        nco_crcf_mix_down(mixer_rx, v2, &v3);
        iirfilt_crcf_execute(filter_rx, v3, &v3);
        nco_crcf_step(mixer_rx);

        // save spectrum
        spgramcf_push(spgram_rx, v3);
    }

    // compute power spectral density output
    float   psd_tx  [nfft];
    float   psd_dac [nfft];
    float   psd_rx  [nfft];
    spgramcf_get_psd(spgram_tx,  psd_tx);
    spgramf_get_psd( spgram_dac, psd_dac);
    spgramcf_get_psd(spgram_rx,  psd_rx);

    // destroy objects
    spgramcf_destroy(spgram_tx);
    spgramf_destroy(spgram_dac);
    spgramcf_destroy(spgram_rx);

    iirfilt_crcf_destroy(filter_tx);
    nco_crcf_destroy(mixer_tx);
    nco_crcf_destroy(mixer_rx);
    iirfilt_crcf_destroy(filter_rx);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");

    fprintf(fid,"nfft   = %u;\n", nfft);
    fprintf(fid,"f      = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"psd_tx = zeros(1,nfft);\n");
    fprintf(fid,"psd_dac= zeros(1,nfft);\n");
    fprintf(fid,"psd_rx = zeros(1,nfft);\n");
    
    for (i=0; i<nfft; i++) {
        fprintf(fid,"psd_tx (%6u) = %12.4e;\n", i+1, psd_tx [i]);
        fprintf(fid,"psd_dac(%6u) = %12.4e;\n", i+1, psd_dac[i]);
        fprintf(fid,"psd_rx (%6u) = %12.4e;\n", i+1, psd_rx [i]);
    }

    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  plot(f, psd_tx,  '-', 'LineWidth',1.5,'Color',[0.7 0.7 0.7]);\n");
    fprintf(fid,"  plot(f, psd_dac, '-', 'LineWidth',1.5,'Color',[0.0 0.5 0.3]);\n");
    fprintf(fid,"  plot(f, psd_rx,  '-', 'LineWidth',1.5,'Color',[0.0 0.3 0.5]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-0.5 0.5 -100 60]);\n");
    fprintf(fid,"legend('transmit (complex)','DAC output (real)','receive (complex)','location','northeast');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

