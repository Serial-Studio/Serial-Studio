// Compute equivalent noise bandwidth of window functions
#include <stdio.h>
#include <string.h>
#include "liquid.h"

#define OUTPUT_FILENAME "window_enbw_example.m"

int main() {
    // options
    unsigned int m    = 20;     // window semi-length
    float        beta = 10.0f;  // Kaiser beta factor
    unsigned int nfft = 1200;   // transform size
    float        th   =  0.95f; // threshold for spectral energy

    // derived values
    unsigned int i;
    unsigned int wlen = 2*m+1;  // window length

    float w[wlen];
    float w2 = 0.0f;
    for (i=0; i<wlen; i++) {
        w[i] = liquid_kaiser(i,wlen,beta);
        w2   += w[i]*w[i]; // window energy
    }

    // compute transform, ensuring symmetry
    float complex buf_time[nfft];
    float complex buf_freq[nfft];
    memset(buf_time, 0x00, nfft*sizeof(float complex));
    buf_time[0] = w[m];
    for (i=1; i<=m; i++) {
        buf_time[i]      = w[m+i];
        buf_time[nfft-i] = w[m-i];
    }
    fft_run(nfft, buf_time, buf_freq, LIQUID_FFT_FORWARD, 0);

    // integrate spectral energy starting from DC and moving evenly outward
    // in both directions
    float energy_ratio[nfft];
    float e2_sum = 0.0f;
    unsigned int i_threshold = 0;
    for (i=0; i<nfft; i++) {
        unsigned int r = i % 2;
        unsigned int L = (i-r)/2;
        int k = (i==0) ? 0 : (r==0 ? -(int)(L+r) : (int)(L+r));
        unsigned int j = (k+nfft) % nfft;
        float e2 = cabsf(buf_freq[j])*cabsf(buf_freq[j]);
        // normalize to transform size
        energy_ratio[i] = e2_sum / (nfft * w2);
        if (i_threshold == 0 && energy_ratio[i] > th)
            i_threshold = i;
        e2_sum += e2;
    }
    float enbw = (float)i_threshold / (float)nfft;
    printf("equivalent noise bandwidth: %.6f Fs\n", enbw);

    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"m=%u;\n",m);
    fprintf(fid,"enbw=%f;\n",enbw);
    fprintf(fid,"th=%f;\n",th);
    fprintf(fid,"nfft=%u;\n",nfft);
    for (i=0; i<wlen; i++)
        fprintf(fid,"w(%4u) = %12.4e;\n", i+1, w[i]);

    for (i=0; i<nfft; i++) {
        fprintf(fid,"W(%4u) = %12.4e;\n", i+1, crealf(buf_freq[i]));
        fprintf(fid,"R(%4u) = %12.4e;\n", i+1, energy_ratio[i]);
    }

    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"t=-m:m;\n");
    fprintf(fid,"figure('position',[50 50 600 1200]);\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t,w,'Color',[0 0.25 0.5],'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Sample Index');\n");
    fprintf(fid,"  ylabel('Window Value');\n");
    fprintf(fid,"  axis([-m m -0.1 1.1]);\n");
    fprintf(fid,"  title(sprintf('Equivalent Noise Bandwidth: %%.6f Fs (%%.1f %%%% Energy)', enbw, th*100));\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(f,20*log10(abs(fftshift(W/sum(w)))),'Color',[0 0.5 0.25],'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('normalized frequency');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fprintf(fid,"  axis([-0.5 0.5 -140 20]);\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot([0:(nfft-1)]/nfft, R, 'Color',[0.5 0.2 0], 'LineWidth',2,...\n");
    fprintf(fid,"       enbw, th, 'ok', 'MarkerSize', 3, 'LineWidth',2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Bandwidth');\n");
    fprintf(fid,"  ylabel('Accumulated Energy Ratio');\n");
    fprintf(fid,"  axis([0 0.1 -0.1 1.1]);\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

