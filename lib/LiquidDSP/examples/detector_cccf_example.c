// 
// detector_example.c
//
// This example demonstrates the binary pre-demodulator synchronizer. A random
// binary sequence is generated, modulated with BPSK, and then interpolated.
// The resulting sequence is used to generate a detector object which in turn
// is used to detect a signal in the presence of carrier frequency and timing
// offsets and additive white Gauss noise.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include "liquid.h"

#define OUTPUT_FILENAME "detector_example.m"

// print usage/help message
void usage()
{
    printf("detector_example -- test binary pre-demodulation synchronization\n");
    printf("options:\n");
    printf("  h     : print usage/help\n");
    printf("  n     : number of sync samples, default: 128\n");
    printf("  F     : carrier frequency offset, default: 0.02\n");
    printf("  T     : fractional sample offset dt in [-0.5, 0.5], default: 0\n");
    printf("  S     : SNR [dB], default: 20\n");
    printf("  t     : detection threshold, default: 0.3\n");
}

int main(int argc, char*argv[])
{
    //srand(time(NULL));

    // options
    unsigned int n    =  128;       // number of sync samples
    float dt          =  0.0f;      // fractional sample timing offset
    float noise_floor = -30.0f;     // noise floor [dB]
    float SNRdB       =  20.0f;     // signal-to-noise ratio [dB]
    float dphi        =  0.0f;      // carrier frequency offset
    float phi         =  0.0f;      // carrier phase offset
    float threshold   =  0.3f;      // detection threshold

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:T:F:S:t:")) != EOF) {
        switch (dopt) {
        case 'h': usage();              return 0;
        case 'n': n         = atoi(optarg); break;
        case 'F': dphi      = atof(optarg); break;
        case 'T': dt        = atof(optarg); break;
        case 'S': SNRdB     = atof(optarg); break;
        case 't': threshold = atof(optarg); break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // validate input
    if (dt < -0.5f || dt > 0.5f) {
        fprintf(stderr,"error: %s, fractional sample offset must be in [-0.5,0.5]\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int num_samples = 3*n;
    float nstd = powf(10.0f, noise_floor/20.0f);
    float gamma = powf(10.0f, (SNRdB + noise_floor)/20.0f);

    // arrays
    float complex s[n];             // synchronization pattern (samples)
    float complex x[num_samples];   // transmitted signal
    float complex y[num_samples];   // received signal

    // generate synchronization pattern (OFDM symbol, slightly over-sampled)
    float complex S[n];
    for (i=0; i<n; i++)
        S[i] = (i < 0.4*n || i > 0.6*n) ? randnf() + _Complex_I*randnf() : 0.0f;
    fft_run(n, S, s, LIQUID_FFT_BACKWARD, 0);
    float s2 = 0.0f;
    for (i=0; i<n; i++)
        s2 += crealf(s[i]*conjf(s[i]));
    for (i=0; i<n; i++)
        s[i] /= sqrtf(s2 / (float)n);

    // generate transmitted signal: 0 0 0 ... 0 s[0] s[1] ... s[n-1] 0 0 0 ... 0
    for (i=0; i<n; i++) {
        x[0*n+i] = 0.0f;
        x[1*n+i] = s[i];
        x[2*n+i] = 0.0f;
    }

    // generate received signal (add channel impairments)
    unsigned int d = 11;    // fractional sample filter delay
    firfilt_crcf finterp = firfilt_crcf_create_kaiser(2*d+1, 0.45f, 40.0f, dt);
    for (i=0; i<num_samples+d; i++) {
        // fractional sample timing offset
        if (i < num_samples) firfilt_crcf_push(finterp, x[i]);
        else                 firfilt_crcf_push(finterp, 0.0f);

        if (i < d) firfilt_crcf_execute(finterp, &y[0]);
        else       firfilt_crcf_execute(finterp, &y[i-d]);
    }
    firfilt_crcf_destroy(finterp);

    for (i=0; i<num_samples; i++) {
        // channel gain
        y[i] *= gamma;

        // carrier offset
        y[i] *= cexpf(_Complex_I*(dphi*i + phi));
        
        // noise
        y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }

    // create cross-correlator
    detector_cccf sync = detector_cccf_create(s, n, threshold, 0.03f);
    detector_cccf_print(sync);

    // push signal through detector
    float tau_hat   = 0.0f;
    float dphi_hat  = 0.0f;
    float gamma_hat = 1.0f;
    int signal_detected = 0;
    unsigned int index = 0;
    for (i=0; i<num_samples; i++) {
        
        // correlate
        int detected = detector_cccf_correlate(sync, y[i], &tau_hat, &dphi_hat, &gamma_hat);

        if (detected) {
            signal_detected = 1;
            printf("****** preamble found, tau_hat=%8.6f, dphi_hat=%8.6f, gamma_hat=%8.6f\n",
                    tau_hat, dphi_hat, gamma_hat);
            index = i;
        }
    }

    // destroy objects
    detector_cccf_destroy(sync);
    
    // print results
    printf("\n");
    printf("signal detected :   %s\n", signal_detected ? "yes" : "no");
    float delay_est = (float) index + tau_hat;
    float delay     = (float)(2*n) + dt; // actual delay (samples)
    printf("delay estimate  : %8.3f, actual=%8.3f (error=%8.3f) sample(s)\n", delay_est, delay, delay-delay_est);
    printf("dphi estimate   : %8.5f, actual=%8.5f (error=%8.5f) rad/sample\n",dphi_hat,  dphi,  dphi-dphi_hat);
    printf("gamma estimate  : %8.3f, actual=%8.3f (error=%8.3f) dB\n",        20*log10f(gamma_hat), 20*log10f(gamma), 20*log10(gamma/gamma_hat));
    printf("\n");

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"n           = %u;\n", n);
    fprintf(fid,"num_samples = %u;\n", num_samples);

    fprintf(fid,"s = zeros(1,n);\n");
    for (i=0; i<n; i++)
        fprintf(fid,"s(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(s[i]), cimagf(s[i]));

    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }

    fprintf(fid,"t=[0:(num_samples-1)];\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(x), t,imag(x));\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('transmitted signal');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,real(y), t,imag(y));\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('received signal');\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
