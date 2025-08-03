// 
// predemod_sync_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include "liquid.h"

#define OUTPUT_FILENAME "predemod_sync_test.m"

// print usage/help message
void usage()
{
    printf("predemod_sync_test -- test pre-demodulation synchronization\n");
    printf("options:\n");
    printf("  h     : print usage/help\n");
    printf("  k     : samples/symbol, default: 2\n");
    printf("  m     : filter delay [symbols], default: 4\n");
    printf("  n     : number of data symbols, default: 64\n");
    printf("  b     : bandwidth-time product, (0,1), default: 0.3\n");
    printf("  t     : fractional sample offset, (-0.5,0.5), default: 0\n");
    printf("  F     : frequency offset, default: 0\n");
    printf("  P     : phase offset, default: 0\n");
    printf("  s     : SNR [dB], default: 30\n");
}

int main(int argc, char*argv[])
{
    srand(time(NULL));
    // options
    unsigned int k=2;                   // filter samples/symbol
    unsigned int m=4;                   // filter delay (symbols)
    float beta=0.3f;                    // bandwidth-time product
    float dt = 0.0f;                    // fractional sample timing offset
    unsigned int num_sync_symbols = 64; // number of data symbols
    float SNRdB = 30.0f;                // signal-to-noise ratio [dB]
    float dphi = 0.0f;                  // carrier frequency offset
    float phi  = 0.0f;                  // carrier phase offset
    
    unsigned int num_delay_symbols = 12;
    unsigned int num_dphi_hat = 21;     // number of frequency offset estimates
    float dphi_hat_step = 0.01f;        // frequency offset step size

    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:n:b:t:F:P:s:")) != EOF) {
        switch (dopt) {
        case 'h': usage();              return 0;
        case 'k': k     = atoi(optarg); break;
        case 'm': m     = atoi(optarg); break;
        case 'n': num_sync_symbols = atoi(optarg); break;
        case 'b': beta  = atof(optarg); break;
        case 't': dt    = atof(optarg); break;
        case 'F': dphi  = atof(optarg); break;
        case 'P': phi   = atof(optarg); break;
        case 's': SNRdB = atof(optarg); break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // validate input
    if (beta <= 0.0f || beta >= 1.0f) {
        fprintf(stderr,"error: %s, bandwidth-time product must be in (0,1)\n", argv[0]);
        exit(1);
    } else if (dt < -0.5 || dt > 0.5) {
        fprintf(stderr,"error: %s, fractional sample offset must be in (0,1)\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int num_symbols = num_delay_symbols + num_sync_symbols + 2*m;
    unsigned int num_samples = k*num_symbols;
    unsigned int num_sync_samples = k*num_sync_symbols;
    float nstd = powf(10.0f, -SNRdB/20.0f);

    // arrays
    float complex seq[num_sync_symbols];    // data sequence (symbols)
    float complex s0[num_sync_samples];     // data sequence (interpolated samples)
    float complex x[num_samples];           // transmitted signal
    float complex y[num_samples];           // received signal
    float rxy[num_dphi_hat][num_samples];   // pre-demod output matrix

    // generate sequence
    for (i=0; i<num_sync_symbols; i++) {
        float sym_i = rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2;
        float sym_q = rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2;
        seq[i] = sym_i + _Complex_I*sym_q;
    }

    // create interpolated sequence, compensating for filter delay
    firinterp_crcf interp_seq = firinterp_crcf_create_prototype(LIQUID_FIRFILT_RRC,k,m,beta,0.0f);
    for (i=0; i<num_sync_symbols+m; i++) {
        if      (i < m)                firinterp_crcf_execute(interp_seq, seq[i], &s0[0]);
        else if (i < num_sync_symbols) firinterp_crcf_execute(interp_seq, seq[i], &s0[k*(i-m)]);
        else                           firinterp_crcf_execute(interp_seq,      0, &s0[k*(i-m)]);
    }
    firinterp_crcf_destroy(interp_seq);
    
    // compute g = E{ |s0|^2 }
    float g = 0.0f;
    for (i=0; i<num_sync_samples; i++)
        g += crealf( s0[i]*conjf(s0[i]) );

    // create transmit interpolator and generate sequence
    firinterp_crcf interp_tx = firinterp_crcf_create_prototype(LIQUID_FIRFILT_RRC,k,m,beta,dt);
    unsigned int n=0;
    for (i=0; i<num_delay_symbols; i++) {
        firinterp_crcf_execute(interp_tx, 0, &x[k*n]);
        n++;
    }
    for (i=0; i<num_sync_symbols; i++) {
        firinterp_crcf_execute(interp_tx, seq[i], &x[k*n]);
        n++;
    }
    for (i=0; i<2*m; i++) {
        firinterp_crcf_execute(interp_tx, 0, &x[k*n]);
        n++;
    }
    assert(n==num_symbols);
    firinterp_crcf_destroy(interp_tx);

    // add channel impairments
    for (i=0; i<num_samples; i++) {
        y[i] = x[i]*cexp(_Complex_I*(dphi*i + phi)) + nstd*( randnf() + _Complex_I*randnf() );
    }

    float complex z;    // filter output sample
    for (n=0; n<num_dphi_hat; n++) {
        float dphi_hat = ((float)n - 0.5*(float)(num_dphi_hat-1)) * dphi_hat_step;
        printf("  dphi_hat : %12.8f\n", dphi_hat);

        // create flipped, conjugated coefficients
        float complex s1[num_sync_samples];
        for (i=0; i<num_sync_samples; i++)
            s1[i] = conjf( s0[num_sync_samples-i-1]*cexpf(_Complex_I*(dphi_hat*i)) );

        // create matched filter and detect signal
        firfilt_cccf fsync = firfilt_cccf_create(s1, num_sync_samples);
        for (i=0; i<num_samples; i++) {
            firfilt_cccf_push(fsync, y[i]);
            firfilt_cccf_execute(fsync, &z);

            rxy[n][i] = cabsf(z) / g;
        }
        // destroy filter
        firfilt_cccf_destroy(fsync);
    }
    
    // print results
    //printf("rxy (max) : %12.8f\n", rxy_max);

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"beta = %f;\n", beta);
    fprintf(fid,"num_sync_symbols = %u;\n", num_sync_symbols);
    fprintf(fid,"num_sync_samples = k*num_sync_symbols;\n");
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"num_dphi_hat = %u;\n", num_dphi_hat);
    fprintf(fid,"dphi_hat_step = %f;\n", dphi_hat_step);

    // save sequence symbols
    fprintf(fid,"seq = zeros(1,num_sync_symbols);\n");
    for (i=0; i<num_sync_symbols; i++)
        fprintf(fid,"seq(%4u)   = %12.8f + j*%12.8f;\n", i+1, crealf(seq[i]), cimagf(seq[i]));

    // save interpolated sequence
    fprintf(fid,"s   = zeros(1,num_sync_samples);\n");
    for (i=0; i<num_sync_samples; i++)
        fprintf(fid,"s(%4u)     = %12.8f + j*%12.8f;\n", i+1, crealf(s0[i]), cimagf(s0[i]));

    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%6u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]),   cimagf(x[i]));
        fprintf(fid,"y(%6u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]),   cimagf(y[i]));
    }

    // save cross-correlation output
    fprintf(fid,"rxy = zeros(num_dphi_hat,num_samples);\n");
    for (n=0; n<num_dphi_hat; n++) {
        for (i=0; i<num_samples; i++) {
            fprintf(fid,"rxy(%6u,%6u) = %12.8f;\n", n+1, i+1, rxy[n][i]);
        }
    }
    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(1:length(s),real(s), 1:length(s),imag(s));\n");
    
    fprintf(fid,"dphi_hat = ( [0:(num_dphi_hat-1)] - (num_dphi_hat-1)/2 ) * dphi_hat_step;\n");
    fprintf(fid,"mesh(dphi_hat, t, rxy');\n");
    
#if 0
    fprintf(fid,"z = abs( z );\n");
    fprintf(fid,"[zmax i] = max(z);\n");
    fprintf(fid,"plot(1:length(z),z,'-x');\n");
    fprintf(fid,"axis([(i-8*k) (i+8*k) 0 zmax*1.2]);\n");
    fprintf(fid,"grid on\n");
#endif

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
