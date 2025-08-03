// Demonstrate partial-band reconstruction using firpfbch2_crcf
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "firpfbch2_crcf_reconstruct_example.m"

int main(int argc, char*argv[])
{
    // options
    unsigned int M  = 64;           // number of channels in analysis filterbank
    unsigned int P  = 22;           // number of channels in synthesis filterbank
    unsigned int m  =  5;           // filter semi-length (symbols)
    float        As = 60.0f;        // filter stop-band attenuation
    float        fc = 0.23;         // frequency in band to center synthesis bank
    unsigned int num_blocks=1<<14;  // number of blocks to generate

    // derived values
    unsigned int M2=M/2, P2=P/2;    // channelizer half sizes, for convenience
    int channel_id = (int)(roundf(fc*M)) % M; // index corresponding to fc
    unsigned int i;

    // create filterbank objects from prototype
    firpfbch2_crcf qa = firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER,    M, m, As);
    firpfbch2_crcf qs = firpfbch2_crcf_create_kaiser(LIQUID_SYNTHESIZER, P, m, As);

    // create multi-signal source generator
    msourcecf gen = msourcecf_create_default();

    // add signals     (gen,  fc,    bw,    gain, {options})
    msourcecf_add_noise(gen,  0.00f, 1.0f, -60);   // wide-band noise
    msourcecf_add_noise(gen, -0.30f, 0.1f, -20);   // narrow-band noise
    msourcecf_add_tone (gen,  0.08f, 0.0f,   0);   // tone
    // modulated data
    msourcecf_add_modem(gen,
       0.18f,                      // center frequency
       0.080f,                     // bandwidth (symbol rate)
       -20,                        // gain
       LIQUID_MODEM_QPSK,          // modulation scheme
       12,                         // filter semi-length
       0.3f);                      // modem parameters
    msourcecf_add_tone (gen,  0.24f, 0.0f, -40);   // another tone

    // create spectral periodogoram
    unsigned int nfft = 2400;
    spgramcf     p0   = spgramcf_create_default(nfft); // original spectrum
    spgramcf     p1   = spgramcf_create_default(nfft); // reconstructed spectrum

    // run channelizer
    float complex buf_a_time[M2];   // analysis, time
    float complex buf_a_freq[M];    // analysis, frequency
    float complex buf_s_freq[P];    // synthesis, frequency
    float complex buf_s_time[P2];   // synthesis, time
    for (i=0; i<num_blocks; i++) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf_a_time, M2);

        // run analysis filterbank
        firpfbch2_crcf_execute(qa, buf_a_time, buf_a_freq);

        // pull relevant samples, applying FFT shift on the input
        for (unsigned int k=0; k<P; k++)
            buf_s_freq[(k+P2)%P] = buf_a_freq[ (M+channel_id+k-P2) % M ];
        //buf_s_freq[P2] = 0.0f; // optionally disable high-frequency crossover point

        // run synthesis filterbank
        firpfbch2_crcf_execute(qs, buf_s_freq, buf_s_time);

        // push results through periodograms
        spgramcf_write(p0, buf_a_time, M2);
        spgramcf_write(p1, buf_s_time, P2);
    }
    spgramcf_print(p0);
    spgramcf_print(p1);

    // compute power spectral density output
    float psd_0[nfft];
    float psd_1[nfft];
    spgramcf_get_psd(p0, psd_0);
    spgramcf_get_psd(p1, psd_1);

    // destroy objects
    firpfbch2_crcf_destroy(qa);
    firpfbch2_crcf_destroy(qs);
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
    fprintf(fid,"f1 = f*P/M + %d/M;\n", channel_id < M/2 ? channel_id : (int)channel_id - (int)M);
    fprintf(fid,"X  = zeros(1,nfft);\n");
    fprintf(fid,"Y  = zeros(1,nfft);\n");

    // save input and output arrays
    for (i=0; i<nfft; i++) {
        fprintf(fid,"X(%4u) = %12.4e;\n", i+1, psd_0[i]);
        fprintf(fid,"Y(%4u) = %12.4e;\n", i+1, psd_1[i]);
    }
    fprintf(fid,"Y = Y + 10*log10(M/P); %% scale by channelizer gain\n");

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"title('composite');\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(f0, X, '-', 'LineWidth',1.5,...\n");
    fprintf(fid,"       f1, Y, '-', 'LineWidth',2.5);\n");
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
