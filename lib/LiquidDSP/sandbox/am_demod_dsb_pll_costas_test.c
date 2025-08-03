// Test double side-band (DSB) amplitude modulation (AM) using
// phase-locked loop (PLL) with Costas phase error detector
// without carrier transmission
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <complex.h>
#include "liquid.h"

int main(int argc, char*argv[])
{
    // options
    float        mod_index   = 1.0f;    // modulation index
    float        phi         = 0.8f;    // carrier phase offset [radians]
    float        dphi        = 0.10f;   // carrier frequency offset [radians/sample]
    float        SNRdB       = 30.0f;   // signal-to-noise ratio (set very high for testing)
    unsigned int num_samples = 2400;    // number of samples
    const char * filename    = "am_demod_dsb_pll_costas_test.m";

    // buffers
    unsigned int i;
    float         x[num_samples];
    float complex y[num_samples];
    float         z[num_samples];
    float         p[num_samples]; // phase error

    // generate 'audio' signal (simple windowed sum of tones)
    unsigned int nw = (unsigned int)(0.90*num_samples); // window length
    unsigned int nt = (unsigned int)(0.05*num_samples); // taper length
    for (i=0; i<num_samples; i++) {
        x[i] =  0.6f*cos(2*M_PI*0.0202*i);
        x[i] += 0.4f*cos(2*M_PI*0.0271*i);
        x[i] *= i < nw ? liquid_rcostaper_window(i,nw,nt) : 0;
    }

    // modulate signal (DSB without carrier)
    for (i=0; i<num_samples; i++)
        y[i] = mod_index*x[i];

    // add channel impairments
    float nstd = powf(10.0f,-SNRdB/20.0f);
    for (i=0; i<num_samples; i++) {
        y[i] *= cexpf(_Complex_I*phi);
        y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // update phase
        phi += dphi;
        while (phi >  M_PI) phi -= 2*M_PI;
        while (phi < -M_PI) phi += 2*M_PI;
    }

    // demodulate signal
    float delay = 0;
    nco_crcf mixer = nco_crcf_create(LIQUID_VCO);
    nco_crcf_pll_set_bandwidth(mixer,0.01f);
    for (i=0; i<num_samples; i++) {
        // mix signal down
        float complex v;
        nco_crcf_mix_down(mixer, y[i], &v);

        // compute phase error
        float phase_error = crealf(v)*cimagf(v);//cargf(v);
        p[i] = phase_error;

        // adjust nco, pll objects
        nco_crcf_pll_step(mixer, phase_error);

        // step NCO
        nco_crcf_step(mixer);

        // set output
        z[i] = crealf(v) / mod_index;
    }
    nco_crcf_destroy(mixer);

    // export results
    FILE * fid = fopen(filename,"w");
    fprintf(fid,"%% %s : auto-generated file\n", filename);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"n=%u;\n",num_samples);
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%3u) = %12.4e;\n",            i+1, x[i]);
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%3u) = %12.4e;\n",            i+1, z[i]);
        fprintf(fid,"p(%3u) = %12.4e;\n",            i+1, p[i]);
    }
    // plot results
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"delay = %f;\n", delay);
    fprintf(fid,"figure('position',[100 100 800 800]);\n");
    // message signals
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t,x,t-delay,z);\n");
    fprintf(fid,"  axis([0 n -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('audio signal');\n");
    fprintf(fid,"  legend('original','demodulated');\n");
    fprintf(fid,"  grid on;\n");
    // rf signal
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(t,real(y),t,imag(y));\n");
    fprintf(fid,"  axis([0 n -2 2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('rf signal');\n");
    fprintf(fid,"  legend('real','imag');\n");
    fprintf(fid,"  grid on;\n");
    // spectrum
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  nfft=2^nextpow2(n);\n");
    fprintf(fid,"  f=[0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"  Y = 20*log10(abs(fftshift(fft(y.*hamming(n)',nfft))));\n");
    fprintf(fid,"  Y = Y - max(Y);\n");
    fprintf(fid,"  plot(f,Y);\n");
    fprintf(fid,"  axis([-0.1 0.1 -70 10]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('normalized frequency');\n");
    fprintf(fid,"  ylabel('psd [dB]');\n");
    fclose(fid);
    printf("results written to %s\n", filename);
    return 0;
}
