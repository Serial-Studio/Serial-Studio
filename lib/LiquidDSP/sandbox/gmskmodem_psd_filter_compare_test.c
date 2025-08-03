// compare GMSK psd with tx filter
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME "gmskmodem_psd_filter_compare_test.m"

int main(int argc, char*argv[]) {
    // options
    unsigned int k      =     2;    // filter samples/symbol
    unsigned int m      =     3;    // filter delay [symbols]
    float        beta   =  0.3f;    // bandwidth-time product
    unsigned int num_symbols = 5000000;// number of symbols to simulate
    unsigned int nfft        = 1200;// fft size

    // create modulator
    gmskmod mod = gmskmod_create(k, m, beta);

    // create spectral periodogram
    spgramcf q = spgramcf_create_default(nfft);

    float complex buf[k];
    unsigned int i;
    for (i=0; i<num_symbols; i++)
    {
        // generate input GMSK signal
        gmskmod_modulate(mod, rand() & 1, buf);

        // update spectrum plot
        spgramcf_write(q, buf, k);
    }
    // write results to output file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"P = zeros(1,nfft);\n");

    // compute gmsk tx filter response
    unsigned int ht_len = 2*k*m+1;
    float        ht[ht_len];
    liquid_firdes_prototype(LIQUID_FIRFILT_GMSKTX, k, m, 0.8f*beta, 0, ht);
    fprintf(fid,"ht = zeros(1,%u);\n", ht_len);
    for (i=0; i<ht_len; i++)
        fprintf(fid,"ht(%3u) = %12.4e;\n", i+1, ht[i]);
    fprintf(fid,"Ht = 20*log10(abs(fftshift(fft(ht/k,nfft))));\n");
    float psd[nfft];
    spgramcf_get_psd(q, psd);
    for (i=0; i<nfft; i++)
        fprintf(fid,"P(%6u) = %12.4e;\n", i+1, psd[i]);
    fprintf(fid,"P = P - 10*log10(k);\n");

    fprintf(fid,"figure('color','white','position',[100 100 800 600]);\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"fc = 0.70/k; idx=find(abs(f)<fc);\n");
    fprintf(fid,"p = polyfit(f(idx),P(idx),6);\n");
    fprintf(fid,"p(2:2:end) = 0; p.' %% force odd elements to be zero (know we have even symmetry)\n");
    fprintf(fid,"Hp = polyval(p,f);\n");
    fprintf(fid,"subplot(2,1,1),\n");
    fprintf(fid,"  plot(f,P,'LineWidth',2, f,Hp,'LineWidth',2);\n"); //, f, Ht);\n");
    fprintf(fid,"  legend('PSD Measurement','Polyfit','location','northwest');\n");
    fprintf(fid,"  axis([-0.5 0.5 -80 20]); grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency'); ylabel('PSD [dB]');\n");
    fprintf(fid,"subplot(2,1,2),\n");
    fprintf(fid,"  plot(f, Hp-P, 'LineWidth', 2);\n");
    fprintf(fid,"  axis([-1.2*fc 1.2*fc -1 1]); grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency'); ylabel('Polyfit Error [dB]');\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    // destroy objects
    gmskmod_destroy(mod);
    spgramcf_destroy(q);
    return 0;
}
