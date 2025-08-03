// Tests least mean-squares (LMS) equalizer (EQ) on a received GMSK
// signal. The equalizer is updated using decision-directed demodulator
// output samples.
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME "gmskmodem_equalizer_test.m"

int main(int argc, char*argv[]) {
    // options
    unsigned int k      =     4;    // filter samples/symbol
    float        beta   =  0.3f;    // bandwidth-time product
    unsigned int p      =     3;    // equalizer length (symbols, hp_len = 2*k*p+1)
    float        mu     = 0.50f;    // learning rate
    unsigned int num_symbols = 240000;// number of symbols to simulate
    unsigned int nfft        = 1200;// number of symbols to simulate

    // create modulator
    gmskmod mod = gmskmod_create(k, 3, beta);

    // add half-sample delay filter
    unsigned int i;
    float fc = 0.4f;
#if 0
    unsigned int m=12;
    float h[2*m];
    for (i=0; i<2*m; i++) {
        float t = (float)i - (float)m + 0.5f;
        h[i] = sincf(2*fc*t);
        printf("h(%2u) = %12.8f;\n", i+1, h[i]);
    }
    firfilt_crcf filt = firfilt_crcf_create(h,2*m);
#else
    firfilt_crcf filt = firfilt_crcf_create_kaiser(17, fc, 60.0f, 0.5f);
#endif
    firfilt_crcf_set_scale(filt, 2*fc);

    // create equalizer
    eqlms_cccf eq = eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, k, p, beta, 0.0f);
    eqlms_cccf_set_bw(eq, mu);

    // window for saving last few symbols
    unsigned int buf_len = 800; // buffer size
    windowcf sym_buf = windowcf_create(buf_len);

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
    fprintf(fid,"psd  = zeros(1,nfft);\n");

    float complex buf[k];
    for (i=0; i<num_symbols; i++)
    {
        // generate input GMSK signal
        gmskmod_modulate(mod, rand() & 1, buf);

        // apply half-sample delay filter
        firfilt_crcf_execute_block(filt, buf, k, buf);

        // write samples into equalizer
        eqlms_cccf_push_block(eq, buf, k);

        // compute equalizer output
        float complex d_hat;
        eqlms_cccf_execute(eq, &d_hat);

        spgramcf_write(q, buf, k);

        // write resulting symbols to window buffer for plotting
        windowcf_push(sym_buf, d_hat);

        // update equalizer appropriately (decision-directed)
        if (i < p) continue;
        float complex d_prime = (crealf(d_hat) > 0 ? 1 : -1) * M_SQRT1_2 +
                                (cimagf(d_hat) > 0 ? 1 : -1) * M_SQRT1_2 * _Complex_I;
        eqlms_cccf_step(eq, d_prime, d_hat);

        // reduce learning rate
        //mu *= 1.0f - 1e-5f;
    }
    printf("mu = %12.4e\n", mu);
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
    // read buffer and write last symbols to file
    float complex * rc;
    windowcf_read(sym_buf, &rc);
    fprintf(fid,"syms = zeros(1,%u);\n", buf_len);
    for (i=0; i<buf_len; i++)
        fprintf(fid,"syms(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(rc[i]), cimagf(rc[i]));

    fprintf(fid,"np = round(0.75*num_symbols);\n");
    fprintf(fid,"figure('color','white','position',[100 100 800 800]);\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"plot(f, psd-10*log10(k), f, 20*log10(abs(fftshift(fft(hp,nfft)))));\n");
    fprintf(fid,"axis([-0.5 0.5 -80 20]); grid on;\n");
    fprintf(fid,"xlabel('Normalized Frequency'); ylabel('PSD [dB]');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"k=%u; p=%u; th=[(-k*p):(k*p)]/k;\n", k, p);
    fprintf(fid,"plot(th, real(hp),'-x',th, imag(hp),'-x');\n");
    fprintf(fid,"axis([(-p) (p) -0.8 1.5]); grid on;\n");
    fprintf(fid,"%%xlabel('Normalized Frequency'); ylabel('PSD [dB]');\n");

    // plot symbols
    fprintf(fid,"figure('color','white','position',[100 100 600 600]);\n");
    fprintf(fid,"plot(real(syms),imag(syms),'x','MarkerSize',4);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.0);\n");
    fprintf(fid,"  xlabel('In-phase');\n");
    fprintf(fid,"  ylabel('Quadrature');\n");
    fprintf(fid,"  title('Last %u symbols');\n", buf_len);

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    // destroy objects
    gmskmod_destroy(mod);
    firfilt_crcf_destroy(filt);
    eqlms_cccf_destroy(eq);
    spgramcf_destroy(q);
    windowcf_destroy(sym_buf);

    return 0;
}
