// 
// ampmodem_test.c
//
// Tests simple modulation/demodulation of the ampmodem (analog
// amplitude modulator/demodulator) with noise, carrier phase,
// and carrier frequency offsets.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "ampmodem_example.m"

// print usage/help message
void usage()
{
    printf("ampmodem_example [options]\n");
    printf("  -h         : print usage\n");
    printf("  -m <index> : modulation index,          default: 0.8\n");
    printf("  -f <freq>  : freq. offset [rad/sample], default: 0.05\n");
    printf("  -p <phase> : phase offset,              default: 2.8\n");
    printf("  -n <num>   : number of samples,         default: 2400\n");
    printf("  -S <snr>   : SNR [dB],                  default: 30\n");
    printf("  -t <type>  : AM type (dsb/usb/lsb),     default: usb\n");
    printf("  -s         : suppress the carrier,      default: off (carrier enabled)\n");
}

int main(int argc, char*argv[])
{
    // options
    float        mod_index          = 0.8f;     // modulation index (bandwidth)
    float        dphi               = 0.05f;    // carrier frequency offset [radians/sample]
    float        phi                = 2.8f;     // carrier phase offset [radians]
    float        SNRdB              = 30.0f;    // signal-to-noise ratio (set very high for testing)
    unsigned int num_samples        = 2400;     // number of samples
    liquid_ampmodem_type type       = LIQUID_AMPMODEM_USB;
    int          suppressed_carrier = 0;

    int dopt;
    while ((dopt = getopt(argc,argv,"hm:f:p:n:S:t:s")) != EOF) {
        switch (dopt) {
        case 'h': usage();                    return 0;
        case 'm': mod_index   = atof(optarg); break;
        case 'f': dphi        = atof(optarg); break;
        case 'p': phi         = atof(optarg); break;
        case 'n': num_samples = atoi(optarg); break;
        case 'S': SNRdB       = atof(optarg); break;
        case 't':
            if (strcmp(optarg,"dsb")==0) {
                type = LIQUID_AMPMODEM_DSB;
            } else if (strcmp(optarg,"usb")==0) {
                type = LIQUID_AMPMODEM_USB;
            } else if (strcmp(optarg,"lsb")==0) {
                type = LIQUID_AMPMODEM_LSB;
            } else {
                fprintf(stderr,"error: %s, invalid AM type: %s\n", argv[0], optarg);
                return 1;
            }
            break;
        case 's': suppressed_carrier = 1;     break;
        default:                              return 1;
        }
    }

    // create mod/demod objects
    ampmodem mod   = ampmodem_create(mod_index, type, suppressed_carrier);
    ampmodem demod = ampmodem_create(mod_index, type, suppressed_carrier);
    unsigned int delay = ampmodem_get_delay_mod(mod) + ampmodem_get_delay_demod(demod);
    ampmodem_print(mod);

    unsigned int i;
    float         x[num_samples];
    float complex y[num_samples];
    float         z[num_samples];

    // generate 'audio' signal (simple windowed sum of tones)
    unsigned int nw = (unsigned int)(0.90*num_samples); // window length
    unsigned int nt = (unsigned int)(0.05*num_samples); // taper length
    for (i=0; i<num_samples; i++) {
        x[i] =  0.6f*cos(2*M_PI*0.0202*i);
        x[i] += 0.4f*cos(2*M_PI*0.0271*i);
        x[i] *= i < nw ? liquid_rcostaper_window(i,nw,nt) : 0;
    }

    // modulate signal
    for (i=0; i<num_samples; i++)
        ampmodem_modulate(mod, x[i], &y[i]);

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
    for (i=0; i<num_samples; i++)
        ampmodem_demodulate(demod, y[i], &z[i]);

    // destroy objects
    ampmodem_destroy(mod);
    ampmodem_destroy(demod);

    // compute demodulation error
    float rmse = 0.0f;
    for (i=delay; i<num_samples; i++)
        rmse += (x[i-delay] - z[i]) * (x[i-delay] - z[i]);
    rmse = 10*log10( rmse / (float)(num_samples-delay) );
    printf("rms error : %.3f dB\n", rmse);

    // export results
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"n=%u;\n",num_samples);
    fprintf(fid,"delay=%u;\n", delay);
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%3u) = %12.4e;\n", i+1, x[i]);
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%3u) = %12.4e;\n", i+1, z[i]);
    }
    // plot results
    fprintf(fid,"t=0:(n-1);\n");
    fprintf(fid,"figure('position',[100 100 800 600]);\n");
    // message signals
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(t,x,t-delay,z);\n");
    fprintf(fid,"  axis([-delay n -1.2 1.2]);\n");
    fprintf(fid,"  xlabel('Time [sample index]');\n");
    fprintf(fid,"  ylabel('Message Signal');\n");
    fprintf(fid,"  legend('original','demodulated');\n");
    fprintf(fid,"  grid on;\n");
    // rf signal
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(t,real(y),t,imag(y));\n");
    fprintf(fid,"  axis([-delay n -1.8 1.8]);\n");
    fprintf(fid,"  xlabel('Time [sample index]');\n");
    fprintf(fid,"  ylabel('RF Signal');\n");
    fprintf(fid,"  legend('real','imag');\n");
    fprintf(fid,"  grid on;\n");
    // spectrum
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  nfft=2^nextpow2(n);\n");
    fprintf(fid,"  f=[0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"  Y = 20*log10(abs(fftshift(fft(y,nfft))));\n");
    fprintf(fid,"  Y = Y - max(Y);\n");
    fprintf(fid,"  plot(f,Y);\n");
    fprintf(fid,"  axis([-0.1 0.1 -60 10]);\n");
    fprintf(fid,"  xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('Received PSD [dB]');\n");
    fprintf(fid,"  grid on;\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);
    return 0;
}
