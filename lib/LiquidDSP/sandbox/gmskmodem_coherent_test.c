// 
// gmskmodem_coherent_test.c
//
// This example demonstrates the continuous phase frequency-shift keying
// (CP-FSK) modem in liquid. A message signal is modulated and the
// resulting signal is recovered using a demodulator object.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "gmskmodem_coherent_test.m"

// print usage/help message
void usage()
{
    printf("gmskmodem_coherent_test -- coherent GMSK demodulation example\n");
    printf("options:\n");
    printf("  h     : print help\n");
    printf("  k     : samples/symbol,           default:  8\n");
    printf("  m     : filter delay (symbols),   default:  3\n");
    printf("  b     : filter roll-off,          default:  0.3\n");
    printf("  n     : number of data symbols,   default: 80\n");
    printf("  s     : SNR [dB],                 default: 40\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int k           = 8;       // filter samples/symbol
    unsigned int m           = 3;       // filter delay (symbols)
    float        beta        = 0.25f;   // GMSK bandwidth-time factor
    unsigned int num_symbols = 80;      // number of data symbols
    float        SNRdB       = 40.0f;   // signal-to-noise ratio [dB]

    int dopt;
    while ((dopt = getopt(argc,argv,"hk:m:b:n:s:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'k': k           = atoi(optarg);   break;
        case 'm': m           = atoi(optarg);   break;
        case 'b': beta        = atof(optarg);   break;
        case 'n': num_symbols = atoi(optarg);   break;
        case 's': SNRdB       = atof(optarg);   break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int num_samples = k*num_symbols;
    float nstd = powf(10.0f, -SNRdB/20.0f);

    // arrays
    unsigned int  sym_in [num_symbols];     // input symbols
    float complex x      [num_samples];     // transmitted signal
    float complex y      [num_samples];     // received signal
    float complex z      [num_samples];     // received signal

    // create modem objects
    gmskmod mod = gmskmod_create(k, m, beta);

    // generate message signal
    for (i=0; i<num_symbols; i++)
        sym_in[i] = rand() % 2;

    // modulate signal
    for (i=0; i<num_symbols; i++)
        gmskmod_modulate(mod, sym_in[i], &x[k*i]);

    // destroy modem object
    gmskmod_destroy(mod);

    // add noise
    for (i=0; i<num_samples; i++) {
        // add noise
        y[i] = x[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }

    // run through equalizing 'matched' filter
    firfilt_crcf mf = firfilt_crcf_create_rnyquist(LIQUID_FIRFILT_GMSKRX,k,m,0.8,0);
    firfilt_crcf_set_scale(mf, 1.5f / (float)k);
    for (i=0; i<num_samples; i++) {
        firfilt_crcf_push(mf, y[i]);
        firfilt_crcf_execute(mf, &z[i]);
    }
    firfilt_crcf_destroy(mf);

    // compute power spectral density of transmitted signal
    unsigned int nfft = 1024;
    float psd[nfft];
    spgramcf_estimate_psd(nfft, x, num_samples, psd);

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"beta = %12.8f;\n", beta);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"nfft        = %u;\n", nfft);

    fprintf(fid,"x   = zeros(1,num_samples);\n");
    fprintf(fid,"y   = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(z[i]), cimagf(z[i]));
    }
    // save power spectral density
    fprintf(fid,"psd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%4u) = %12.8f;\n", i+1, psd[i]);

#if 0
    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"i = 1:k:num_samples;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,4,1:3);\n");
    fprintf(fid,"  plot(t,real(x),'-', t(i),real(x(i)),'ob',...\n");
    fprintf(fid,"       t,imag(x),'-', t(i),imag(x(i)),'og');\n");
    fprintf(fid,"  axis([0 num_symbols -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('x(t)');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,4,5:7);\n");
    fprintf(fid,"  plot(t,real(z),'-', t(i),real(z(i)),'ob',...\n");
    fprintf(fid,"       t,imag(z),'-', t(i),imag(z(i)),'og');\n");
    fprintf(fid,"  axis([0 num_symbols -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('y(t)');\n");
    fprintf(fid,"  grid on;\n");
    // plot I/Q constellations
    fprintf(fid,"subplot(3,4,4);\n");
    fprintf(fid,"  plot(real(x),imag(x),'-',real(x(i)),imag(x(i)),'rs','MarkerSize',4);\n");
    fprintf(fid,"  xlabel('I');\n");
    fprintf(fid,"  ylabel('Q');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,4,8);\n");
    fprintf(fid,"  plot(real(z),imag(z),'-',real(z(i)),imag(z(i)),'rs','MarkerSize',4);\n");
    fprintf(fid,"  xlabel('I');\n");
    fprintf(fid,"  ylabel('Q');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    // plot PSD
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"subplot(3,4,9:12);\n");
    fprintf(fid,"  plot(f,psd,'LineWidth',1.5);\n");
    fprintf(fid,"  axis([-0.5 0.5 -60 20]);\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('PSD [dB]');\n");
    fprintf(fid,"  grid on;\n");
#else
    fprintf(fid,"i = (1+2*k*m):k:num_samples;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  plot(real(z),imag(z),'-','Color',[1 1 1]*0.8);\n");
    fprintf(fid,"  plot(real(z(i)),imag(z(i)),'rs','MarkerSize',4);\n");
    fprintf(fid,"  plot([-1 -1 1 1]/sqrt(2), [-1 1 -1 1]/sqrt(2), 'x', 'MarkerSize',8);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"xlabel('real');\n");
    fprintf(fid,"ylabel('imag');\n");
    fprintf(fid,"grid on;\n");
#endif

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
