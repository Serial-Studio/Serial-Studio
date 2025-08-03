// test GMSK equalization
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME "gmsk_eqlms_example.m"

int main(int argc, char*argv[]) {
    // options
    unsigned int k      =     4;    // filter samples/symbol
    float        beta   =  0.3f;    // bandwidth-time product
    unsigned int p      =     3;    // equalizer length (symbols, hp_len = 2*k*p+1)
    float        mu     = 0.08f;    // learning rate
    unsigned int num_symbols = 2400;// number of symbols to simulate
    unsigned int nfft        = 1200;// number of symbols to simulate

    // create modulator
    gmskmod mod = gmskmod_create(k, 3, beta);

    // create equalizer
    eqlms_cccf eq = eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_GMSKRX, k, p, beta, 0.0f);
    eqlms_cccf_set_bw(eq, mu);

    // create spectral periodogram
    spgramcf q = spgramcf_create_default(nfft);

    // write results to output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"syms = zeros(1,num_symbols);\n");
    fprintf(fid,"psd  = zeros(1,nfft);\n");

    float complex buf[k];
    unsigned int i;
    for (i=0; i<num_symbols; i++)
    {
        // generate input GMSK signal
        gmskmod_modulate(mod, rand() & 1, buf);

        // write samples into equalizer
        eqlms_cccf_push_block(eq, buf, k);

        // compute equalizer output
        float complex d_hat;
        eqlms_cccf_execute(eq, &d_hat);

        spgramcf_write(q, buf, k);

        // write results to file
        fprintf(fid,"syms(%4u) = %12.4e + %12.4ej;\n", i+1, crealf(d_hat), cimagf(d_hat));

        // update equalizer appropriately
        if (i < p) continue;
        float complex d_prime = (crealf(d_hat) > 0 ? 1 : -1) * M_SQRT1_2 +
                                (cimagf(d_hat) > 0 ? 1 : -1) * M_SQRT1_2 * _Complex_I;
        eqlms_cccf_step(eq, d_prime, d_hat);
    }
    // get equalizer weights
    unsigned int hp_len = 2*k*p+1;   // equalizer filter length
    float complex hp[hp_len];           // equalizer filter coefficients
    eqlms_cccf_copy_coefficients(eq, hp);
    fprintf(fid,"hp = zeros(1,%u);\n", hp_len);
    for (i=0; i<hp_len; i++)
        fprintf(fid,"hp(%3u) = %12.4e + %12.4ej;\n", i+1, crealf(hp[i]), cimagf(hp[i]));
    float psd[nfft];
    spgramcf_get_psd(q, psd);
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%6u) = %12.4e;\n", i+1, psd[i]);

    fprintf(fid,"np = round(0.75*num_symbols);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(syms(1:np),  'x','Color',[1 1 1]*0.7,...\n");
    fprintf(fid,"     syms(np:end),'x','Color',[0 0.2 0.4]);\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.2); grid on; axis square;\n");
    fprintf(fid,"xlabel('I'); ylabel('Q'); legend('first half','last half');\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"plot(f, psd-10*log10(k), f, 20*log10(abs(fftshift(fft(hp,nfft)))));\n");
    fprintf(fid,"axis([-0.5 0.5 -50 10]); grid on;\n");
    fprintf(fid,"xlabel('Normalized Frequency'); ylabel('PSD [dB]');\n");
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    // destroy objects
    gmskmod_destroy(mod);
    eqlms_cccf_destroy(eq);
    spgramcf_destroy(q);

    return 0;
}
