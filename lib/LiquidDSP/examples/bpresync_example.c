// 
// bpresync_example.c
//
// This example demonstrates the binary pre-demodulator synchronizer. A random
// binary sequence is generated, modulated with BPSK, and then interpolated.
// The resulting sequence is used to generate a bpresync object which in turn
// is used to detect a signal in the presence of carrier frequency and timing
// offsets and additive white Gauss noise.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include "liquid.h"

#define OUTPUT_FILENAME "bpresync_example.m"

// print usage/help message
void usage()
{
    printf("bpresync_example -- test binary pre-demodulation synchronization\n");
    printf("options (default values in <>):\n");
    printf("  h     : print usage/help\n");
    printf("  k     : samples/symbol, default: 2\n");
    printf("  m     : filter delay [symbols], default: 5\n");
    printf("  n     : number of data symbols, default: 64\n");
    printf("  b     : bandwidth-time product beta in (0,1), default: 0.3\n");
    printf("  F     : carrier frequency offset, default: 0.02\n");
    printf("  t     : fractional sample offset dt in [-0.5, 0.5], default: 0\n");
    printf("  S     : SNR [dB], default: 20\n");
}

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // options
    unsigned int k=2;                   // filter samples/symbol
    unsigned int m=5;                   // filter delay (symbols)
    float beta=0.3f;                    // bandwidth-time product
    float dt = 0.0f;                    // fractional sample timing offset
    unsigned int num_sync_symbols = 64; // number of synchronization symbols
    float SNRdB = 20.0f;                // signal-to-noise ratio [dB]
    float dphi = 0.02f;                 // carrier frequency offset
    float phi  = 2*M_PI*randf();        // carrier phase offset

    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:n:b:t:F:t:S:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                          return 0;
        case 'k': k = atoi(optarg);                 break;
        case 'm': m = atoi(optarg);                 break;
        case 'n': num_sync_symbols = atoi(optarg);  break;
        case 'b': beta = atof(optarg);              break;
        case 'F': dphi = atof(optarg);              break;
        case 't': dt = atof(optarg);                break;
        case 'S': SNRdB = atof(optarg);             break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // validate input
    if (beta <= 0.0f || beta >= 1.0f) {
        fprintf(stderr,"error: %s, bandwidth-time product must be in (0,1)\n", argv[0]);
        exit(1);
    } else if (dt < -0.5f || dt > 0.5f) {
        fprintf(stderr,"error: %s, fractional sample offset must be in [-0.5,0.5]\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int num_symbols = num_sync_symbols + 2*m + 10;
    unsigned int num_samples = k*num_symbols;
    float nstd = powf(10.0f, -SNRdB/20.0f);

    // arrays
    float complex seq[num_sync_symbols];    // synchronization pattern (symbols)
    float complex s0[k*num_sync_symbols];   // synchronization pattern (samples)
    float complex x[num_samples];           // transmitted signal
    float complex y[num_samples];           // received signal
    float complex rxy[num_samples];         // pre-demod correlation output
    float dphi_hat[num_samples];            // carrier offset estimate

    // create transmit/receive interpolator/decimator
    firinterp_crcf interp = firinterp_crcf_create_prototype(LIQUID_FIRFILT_RRC,k,m,beta,dt);

    // generate synchronization pattern (BPSK) and interpolate
    for (i=0; i<num_sync_symbols + 2*m; i++) {
        float complex sym = 0.0f;
    
        if (i < num_sync_symbols) {
            sym = rand() % 2 ? -1.0f : 1.0f;
            seq[i] = sym;
        }

        if (i < 2*m) firinterp_crcf_execute(interp, sym, s0);
        else         firinterp_crcf_execute(interp, sym, &s0[k*(i-2*m)]);
    }

    // reset interpolator
    firinterp_crcf_reset(interp);

    // interpolate input
    for (i=0; i<num_symbols; i++) {
        float complex sym = i < num_sync_symbols ? seq[i] : 0.0f;

        firinterp_crcf_execute(interp, sym, &x[k*i]);
    }

    // push through channel
    for (i=0; i<num_samples; i++)
        y[i] = x[i]*cexpf(_Complex_I*(dphi*i + phi)) + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

    // create cross-correlator
    bpresync_cccf sync = bpresync_cccf_create(s0, k*num_sync_symbols, 0.05f, 11);
    bpresync_cccf_print(sync);

    // push signal through cross-correlator
    float rxy_max  = 0.0f;  // maximum cross-correlation
    float dphi_est = 0.0f;  // carrier frequency offset estimate
    int delay_est  = 0;     // delay estimate
    for (i=0; i<num_samples; i++) {
        
        // correlate
        bpresync_cccf_push(sync, y[i]);
        bpresync_cccf_execute(sync, &rxy[i], &dphi_hat[i]);

        // detect...
        if (cabsf(rxy[i]) > 0.6f) {
            printf("****** preamble found, rxy = %12.8f (dphi-hat: %12.8f), i=%3u ******\n",
                    cabsf(rxy[i]), dphi_hat[i], i);
        }
        
        // retain maximum
        if (cabsf(rxy[i]) > rxy_max) {
            rxy_max   = cabsf(rxy[i]);
            dphi_est  = dphi_hat[i];
            delay_est = (int)i - (int)2*k*m + 1;
        }
    }

    // destroy objects
    firinterp_crcf_destroy(interp);
    bpresync_cccf_destroy(sync);
    
    // print results
    printf("\n");
    printf("rxy (max) : %12.8f\n", rxy_max);
    printf("dphi est. : %12.8f ,error=%12.8f\n",      dphi_est, dphi-dphi_est);
    printf("delay est.: %12d ,error=%3d sample(s)\n", delay_est, k*num_sync_symbols - delay_est);
    printf("\n");

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"k           = %u;\n", k);

    fprintf(fid,"x   = zeros(1,num_samples);\n");
    fprintf(fid,"y   = zeros(1,num_samples);\n");
    fprintf(fid,"rxy = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u)     = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]),   cimagf(x[i]));
        fprintf(fid,"y(%4u)     = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]),   cimagf(y[i]));
        fprintf(fid,"rxy(%4u)   = %12.8f + j*%12.8f;\n", i+1, crealf(rxy[i]), cimagf(rxy[i]));
    }

    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(t,real(y), t,imag(y));\n");
    fprintf(fid,"  axis([0 num_symbols -2 2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('received signal');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(t,abs(rxy));\n");
    fprintf(fid,"  axis([0 num_symbols 0 1.5]);\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('correlator output');\n");
    fprintf(fid,"  grid on;\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
