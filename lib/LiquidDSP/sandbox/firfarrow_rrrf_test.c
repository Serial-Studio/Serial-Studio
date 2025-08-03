//
// firfarrow_rrrf_test.c
//
// Demonstrates the functionality of the finite impulse response Farrow
// filter for arbitrary fractional sample group delay.
//

#include <stdio.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firfarrow_rrrf_test.m"

int main() {
    // options
    unsigned int h_len=19;  // filter length
    unsigned int p=5;       // polynomial order
    float fc=0.45f;         // filter cutoff
    float As=60.0f;         // stop-band attenuation [dB]
    unsigned int m=9;       // number of delays to evaluate

    // coefficients array
    float h[h_len];
    float tao = ((float)h_len-1)/2.0f;  // nominal filter delay

    firfarrow_rrrf f = firfarrow_rrrf_create(h_len, p, fc, As);

    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"h_len = %u;\n", h_len);
    fprintf(fid,"h = zeros(%u,%u);\n", m, h_len);
    fprintf(fid,"mu = zeros(1,%u);\n", m);
    fprintf(fid,"tao = %12.8f;\n", tao);

    unsigned int i, j;
    float mu_vect[m];
    for (i=0; i<m; i++) {
        mu_vect[i] = ((float)i)/((float)m-1) - 0.5f;
        //printf("mu[%3u] = %12.8f\n", i, mu_vect[i]);
        fprintf(fid,"mu(%3u) = %12.8f;\n", i+1, mu_vect[i]);
    }

    firfarrow_rrrf_print(f);
    printf("Farrow filter group delay error for certain frequencies:\n");
    printf("%8s %8s  : %8.4f   %8.4f   %8.4f\n", "mu", "delay", 0.0f, 0.2f, 0.4f);
    for (i=0; i<m; i++) {
        firfarrow_rrrf_set_delay(f,mu_vect[i]);

        firfarrow_rrrf_get_coefficients(f,h);
        for (j=0; j<h_len; j++)
            fprintf(fid,"  h(%3u,%3u) = %12.4e;\n", i+1, j+1, h[j]);

        //printf("group delay (mu = %12.8f) : %12.8f\n", mu_vect[i], fir_group_delay(h,h_len,0.0f));
        printf("%8.4f ", mu_vect[i]);
        printf("%8.4f  : ", tao + mu_vect[i]);
        printf("%8.1e   ", tao + mu_vect[i] - firfarrow_rrrf_groupdelay(f,0.0f));
        printf("%8.1e   ", tao + mu_vect[i] - firfarrow_rrrf_groupdelay(f,0.2f));
        printf("%8.1e   ", tao + mu_vect[i] - firfarrow_rrrf_groupdelay(f,0.4f));
        printf("\n");
    }

   
    // compute delay, print results
    fprintf(fid,"nfft=512;\n");
    fprintf(fid,"f = [0:(nfft-1)]/(2*nfft);\n");
    fprintf(fid,"g = 0:(h_len-1);\n");
    fprintf(fid,"D = zeros(m,nfft);\n");
    fprintf(fid,"D_ideal = zeros(m,2);\n");
    fprintf(fid,"H = zeros(m,nfft);\n");
    fprintf(fid,"for i=1:m,\n");
    fprintf(fid,"    d = real( fft(h(i,:).*g,2*nfft) ./ fft(h(i,:),2*nfft) );\n");
    fprintf(fid,"    D(i,:) = d(1:nfft);\n");
    fprintf(fid,"    D_ideal(i,:) = tao + mu(i);\n");
    fprintf(fid,"    H_prime = 20*log10(abs(fftshift(fft(h(i,:),2*nfft))));\n");
    fprintf(fid,"    H(i,:) = H_prime([nfft+1]:[2*nfft]);\n");
    fprintf(fid,"end;\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,H,'-');\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,D,'-k','LineWidth',2,[0 0.5],D_ideal,'-k');\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('Group Delay [samples]');\n");
    fprintf(fid,"axis([0 0.5 (tao-1) (tao+1)]);\n");
    fprintf(fid,"for i=1:m,\n");
    fprintf(fid,"    text(0.3,tao+mu(i)+0.025,['\\mu =' num2str(mu(i))]);\n");
    fprintf(fid,"end;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    firfarrow_rrrf_destroy(f);

    printf("done.\n");
    return 0;
}

