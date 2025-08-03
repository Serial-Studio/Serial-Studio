//
// qdetector_example.c
//
// This example demonstrates the functionality of the qdetector object
// to detect an arbitrary signal in time in the presence of noise,
// carrier frequency/phase offsets, and fractional-sample timing
// offsets.
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include "liquid.h"

#define OUTPUT_FILENAME "qdetector_cccf_example.m"

// print usage/help message
void usage()
{
    printf("qdetector_cccf_example\n");
    printf("options:\n");
    printf("  h     : print usage/help\n");
    printf("  n     : number of sync symbols,     default:  80\n");
    printf("  k     : samples/symbol,             default:  2\n");
    printf("  m     : filter delay,               default:  7 sybmols\n");
    printf("  b     : excess bandwidth factor,    default:  0.3\n");
    printf("  F     : carrier frequency offset,   default: -0.01\n");
    printf("  T     : fractional sample offset,   default:  0\n");
    printf("  S     : SNR [dB],                   default:  20 dB\n");
    printf("  t     : detection threshold,        default:  0.3\n");
    printf("  r     : carrier offset search range,default:  0.05\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int sequence_len =   80;   // number of sync symbols
    unsigned int k            =    2;   // samples/symbol
    unsigned int m            =    7;   // filter delay [symbols]
    float        beta         = 0.3f;   // excess bandwidth factor
    int          ftype        = LIQUID_FIRFILT_ARKAISER;
    float        tau          = -0.3f;  // fractional sample timing offset
    float        dphi         = -0.01f; // carrier frequency offset
    float        phi          =  0.5f;  // carrier phase offset
    float        noise_floor  = -30.0f; // noise floor [dB]
    float        SNRdB        = 20.0f;  // signal-to-noise ratio [dB]
    float        threshold    =  0.5f;  // detection threshold
    float        range        =  0.05f; // carrier offset search range [radians/sample]

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:k:m:b:F:T:S:t:r:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'n': sequence_len  = atoi(optarg); break;
        case 'k': k             = atoi(optarg); break;
        case 'm': m             = atoi(optarg); break;
        case 'b': beta          = atof(optarg); break;
        case 'F': dphi          = atof(optarg); break;
        case 'T': tau           = atof(optarg); break;
        case 'S': SNRdB         = atof(optarg); break;
        case 't': threshold     = atof(optarg); break;
        case 'r': range         = atof(optarg); break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // validate input
    if (tau < -0.5f || tau > 0.5f) {
        fprintf(stderr,"error: %s, fractional sample offset must be in [-0.5,0.5]\n", argv[0]);
        exit(1);
    }

    // derived values
    float nstd = powf(10.0f, noise_floor/20.0f);
    float gamma = powf(10.0f, (SNRdB + noise_floor)/20.0f);

    // generate synchronization sequence (QPSK symbols)
    float complex sequence[sequence_len];
    for (i=0; i<sequence_len; i++) {
        sequence[i] = (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 +
                      (rand() % 2 ? 1.0f : -1.0f) * M_SQRT1_2 * _Complex_I;
    }

    //
    float rxy       = 0.0f;
    float tau_hat   = 0.0f;
    float gamma_hat = 0.0f;
    float dphi_hat  = 0.0f;
    float phi_hat   = 0.0f;
    int   frame_detected = 0;

    // create detector
    qdetector_cccf q = qdetector_cccf_create_linear(sequence, sequence_len, ftype, k, m, beta);
    qdetector_cccf_set_threshold(q, threshold);
    qdetector_cccf_set_range    (q, range);
    qdetector_cccf_print(q);

    //
    unsigned int seq_len     = qdetector_cccf_get_seq_len(q);
    unsigned int buf_len     = qdetector_cccf_get_buf_len(q);
    unsigned int num_samples = 2*buf_len;   // double buffer length to ensure detection
    unsigned int num_symbols = buf_len;

    // arrays
    float complex y[num_samples];       // received signal
    float complex syms_rx[num_symbols]; // recovered symbols

    // get pointer to sequence and generate full sequence
    float complex * v = (float complex*) qdetector_cccf_get_sequence(q);
    unsigned int filter_delay = 15;
    firfilt_crcf filter = firfilt_crcf_create_kaiser(2*filter_delay+1, 0.4f, 60.0f, -tau);
    for (i=0; i<num_samples; i++) {
        // add delay
        firfilt_crcf_push(filter, i < seq_len ? v[i] : 0);
        firfilt_crcf_execute(filter, &y[i]);

        // channel gain
        y[i] *= gamma;

        // carrier offset
        y[i] *= cexpf(_Complex_I*(dphi*i + phi));

        // noise
        y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }
    firfilt_crcf_destroy(filter);

    // run detection on sequence
    for (i=0; i<num_samples; i++) {
        v = qdetector_cccf_execute(q,y[i]);

        if (v != NULL) {
            printf("\nframe detected!\n");
            frame_detected = 1;

            // get statistics
            rxy       = qdetector_cccf_get_rxy(q);
            tau_hat   = qdetector_cccf_get_tau(q);
            gamma_hat = qdetector_cccf_get_gamma(q);
            dphi_hat  = qdetector_cccf_get_dphi(q);
            phi_hat   = qdetector_cccf_get_phi(q);
            break;
        }
    }

    unsigned int num_syms_rx = 0;   // output symbol counter
    unsigned int counter     = 0;   // decimation counter
    if (frame_detected) {
        // recover symbols
        firfilt_crcf mf = firfilt_crcf_create_rnyquist(ftype, k, m, beta, tau_hat);
        firfilt_crcf_set_scale(mf, 1.0f / (float)(k*gamma_hat));
        nco_crcf     nco = nco_crcf_create(LIQUID_VCO);
        nco_crcf_set_frequency(nco, dphi_hat);
        nco_crcf_set_phase    (nco,  phi_hat);

        for (i=0; i<buf_len; i++) {
            //
            float complex sample;
            nco_crcf_mix_down(nco, v[i], &sample);
            nco_crcf_step(nco);

            // apply decimator
            firfilt_crcf_push(mf, sample);
            counter++;
            if (counter == k-1)
                firfilt_crcf_execute(mf, &syms_rx[num_syms_rx++]);
            counter %= k;
        }

        nco_crcf_destroy(nco);
        firfilt_crcf_destroy(mf);
    }

    // destroy objects
    qdetector_cccf_destroy(q);

    // print results
    printf("\n");
    printf("frame detected  :   %s\n", frame_detected ? "yes" : "no");
    if (frame_detected) {
        printf("  rxy           : %8.3f\n", rxy);
        printf("  gamma hat     : %8.3f, actual=%8.3f (error=%8.3f)\n",            gamma_hat, gamma, gamma_hat - gamma);
        printf("  tau hat       : %8.3f, actual=%8.3f (error=%8.3f) samples\n",    tau_hat,   tau,   tau_hat   - tau  );
        printf("  dphi hat      : %8.5f, actual=%8.5f (error=%8.5f) rad/sample\n", dphi_hat,  dphi,  dphi_hat  - dphi );
        printf("  phi hat       : %8.5f, actual=%8.5f (error=%8.5f) radians\n",    phi_hat,   phi,   phi_hat   - phi  );
        printf("  symbols rx    : %u\n", num_syms_rx);
    }
    printf("\n");

    //
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"sequence_len= %u;\n", sequence_len);
    fprintf(fid,"num_samples = %u;\n", num_samples);

    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));

    fprintf(fid,"num_syms_rx = %u;\n", num_syms_rx);
    fprintf(fid,"syms_rx     = zeros(1,num_syms_rx);\n");
    for (i=0; i<num_syms_rx; i++)
        fprintf(fid,"syms_rx(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(syms_rx[i]), cimagf(syms_rx[i]));

    fprintf(fid,"t=[0:(num_samples-1)];\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(4,1,1);\n");
    fprintf(fid,"  plot(t,real(y), t,imag(y));\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('time');\n");
    fprintf(fid,"  ylabel('received signal');\n");
    fprintf(fid,"subplot(4,1,2:4);\n");
    fprintf(fid,"  plot(real(syms_rx), imag(syms_rx), 'x');\n");
    fprintf(fid,"  axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"  axis square;\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('real');\n");
    fprintf(fid,"  ylabel('imag');\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
