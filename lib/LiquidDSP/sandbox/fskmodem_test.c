// 
// fskmodem_test.c
//
// This example demonstrates the M-ary frequency-shift keying
// (MFSK) modem in liquid. A message signal is modulated and the
// resulting signal is recovered using a demodulator object.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "fskmodem_test.m"

// print usage/help message
void usage()
{
    printf("fskmodem_example -- frequency-shift keying test\n");
    printf("options:\n");
    printf("  h     : print help\n");
    printf("  m     : bits/symbol,              default:  1\n");
    printf("  k     : samples/symbol,           default:  2*2^m\n");
    printf("  b     : signal bandwidth          default:  0.2\n");
    printf("  n     : number of data symbols,   default: 80\n");
    printf("  s     : SNR [dB],                 default: 40\n");
    printf("  F     : carrier frequency offset, default:  0\n");
    printf("  P     : carrier phase offset,     default:  0\n");
    printf("  T     : fractional symbol offset, default:  0\n");
}

int main(int argc, char*argv[])
{
    // options
    unsigned int m           =   3;     // number of bits/symbol
    unsigned int k           =   0;     // filter samples/symbol
    unsigned int num_symbols = 200;     // number of data symbols
    float        SNRdB       = 40.0f;   // signal-to-noise ratio [dB]
    float        cfo         = 0.0f;    // carrier frequency offset
    float        cpo         = 0.0f;    // carrier phase offset
    float        tau         = 0.0f;    // fractional symbol timing offset
    float        bandwidth   = 0.20;    // frequency spacing

    int dopt;
    while ((dopt = getopt(argc,argv,"hm:k:b:n:s:F:P:T:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'm': m           = atoi(optarg);   break;
        case 'k': k           = atoi(optarg);   break;
        case 'b': bandwidth   = atof(optarg);   break;
        case 'n': num_symbols = atoi(optarg);   break;
        case 's': SNRdB       = atof(optarg);   break;
        case 'F': cfo         = atof(optarg);   break;
        case 'P': cpo         = atof(optarg);   break;
        case 'T': tau         = atof(optarg);   break;
        default:
            exit(1);
        }
    }
    printf("channel offsets: tau=%.3f, cfo=%.3f, cpo=%.3f\n", tau, cfo, cpo);

    unsigned int i;
    unsigned int j;

    // derived values
    if (k == 0) k = 2 << m; // set samples per symbol if not otherwise specified
    unsigned int num_samples = k*num_symbols;
    unsigned int M           = 1 << m;
    float        nstd        = powf(10.0f, -SNRdB/20.0f);
    float        M2          = 0.5f*(float)(M-1);

    // validate input
    if (k < M) {
        fprintf(stderr,"errors: %s, samples/symbol must be at least modulation size (M=%u)\n", __FILE__,M);
        exit(1);
    } else if (k > 2048) {
        fprintf(stderr,"errors: %s, samples/symbol exceeds maximum (2048)\n", __FILE__);
        exit(1);
    } else if (M > 1024) {
        fprintf(stderr,"errors: %s, modulation size (M=%u) exceeds maximum (1024)\n", __FILE__, M);
        exit(1);
    } else if (bandwidth <= 0.0f || bandwidth >= 0.5f) {
        fprintf(stderr,"errors: %s, bandwidth must be in (0,0.5)\n", __FILE__);
        exit(1);
    }

    // compute demodulation FFT size such that FFT output bin frequencies are
    // as close to modulated frequencies as possible
    unsigned int K = 0;                 // demodulation FFT size
    float        df = bandwidth / M2;   // frequency spacing
    float        err_min = 1e9f;
    unsigned int K_min = k;                     // minimum FFT size
    unsigned int K_max = k*4 < 16 ? 16 : k*4;   // maximum FFT size
    unsigned int K_hat;
    for (K_hat=K_min; K_hat<=K_max; K_hat++) {
        // compute candidate FFT size
        float v     = 0.5f*df * (float)K_hat;   // bin spacing
        float err = fabsf( roundf(v) - v );     // fractional bin spacing

        // print results
        printf("  K_hat = %4u : v = %12.8f, err=%12.8f %s\n", K_hat, v, err, err < err_min ? "*" : "");

        // save best result
        if (K_hat==K_min || err < err_min) {
            K = K_hat;
            err_min = err;
        }

        // perfect match; no need to continue searching
        if (err < 1e-6f)
            break;
    }

    // arrays
    unsigned int  sym_in[num_symbols];      // input symbols
    float complex x[num_samples];           // transmitted signal
    float complex y[num_samples];           // received signal
    unsigned int  sym_out[num_symbols];     // output symbols

    // determine demodulation mapping between tones and frequency bins
    // TODO: use gray coding
    unsigned int demod_map[M];
    for (i=0; i<M; i++) {
        // print frequency bins
        float freq = ((float)i - M2) * bandwidth / M2;
        float idx  = freq * (float)K;
        unsigned int index = (unsigned int) (idx < 0 ? roundf(idx + K) : roundf(idx));
        demod_map[i] = index;
        printf("  s=%3u, f = %12.8f, index=%3u\n", i, freq, index);
    }

    // check for uniqueness
    for (i=1; i<M; i++) {
        if (demod_map[i] == demod_map[i-1]) {
            fprintf(stderr,"warning: demod map is not unique; consider increasing bandwidth\n");
            break;
        }
    }

    // generate message symbols and modulate
    // TODO: use gray coding
    for (i=0; i<num_symbols; i++) {
        // generate random symbol
        sym_in[i] = rand() % M;

        // compute frequency
        float dphi = 2*M_PI*((float)sym_in[i] - M2) * bandwidth / M2;

        // generate random phase
        float phi  = randf() * 2 * M_PI;
        
        // modulate symbol
        for (j=0; j<k; j++)
            x[i*k+j] = cexpf(_Complex_I*phi + _Complex_I*j*dphi);
    }

    // push through channel
    for (i=0; i<num_samples; i++)
        y[i] = x[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

#if 0
    // demodulate signal: high SNR method
    float complex buf_time[k];
    unsigned int n = 0;
    j = 0;
    for (i=0; i<num_samples; i++) {
        // start filling time buffer with samples (assume perfect symbol timing)
        buf_time[n++] = y[i];

        // demodulate symbol
        if (n==k) {
            // reset counter
            n = 0;

            // estimate frequency
            float complex metric = 0;
            unsigned int s;
            for (s=1; s<k; s++)
                metric += buf_time[s] * conjf(buf_time[s-1]);
            float dphi_hat = cargf(metric) / (2*M_PI);
            unsigned int v=( (unsigned int) roundf(dphi_hat*M2/bandwidth + M2) ) % M;
            sym_out[j++] = v;
            printf("%3u : %12.8f : %u\n", j, dphi_hat, v);
        }
    }
#else
    // demodulate signal: least-squares method
    float complex buf_time[K];
    float complex buf_freq[K];
    fftplan fft = fft_create_plan(K, buf_time, buf_freq, LIQUID_FFT_FORWARD, 0);

    for (i=0; i<K; i++)
        buf_time[i] = 0.0f;
    unsigned int n = 0;
    j = 0;
    for (i=0; i<num_samples; i++) {
        // start filling time buffer with samples (assume perfect symbol timing)
        buf_time[n++] = y[i];

        // demodulate symbol
        if (n==k) {
            // reset counter
            n = 0;

            // compute transform, storing result in 'buf_freq'
            fft_execute(fft);

            // find maximum by looking at particular bins
            float vmax = 0;
            unsigned int s;
            unsigned int s_opt = 0;
            for (s=0; s<M; s++) {
                float v = cabsf( buf_freq[demod_map[s]] );
                if (s==0 || v > vmax) {
                    s_opt = s;
                    vmax  =v;
                }
            }

            // save best result
            sym_out[j++] = s_opt;
        }
    }
    // destroy fft object
    fft_destroy_plan(fft);
#endif

    // count errors
    unsigned int num_symbol_errors = 0;
    for (i=0; i<num_symbols; i++)
        num_symbol_errors += (sym_in[i] == sym_out[i]) ? 0 : 1;

    printf("symbol errors: %u / %u\n", num_symbol_errors, num_symbols);

    // compute power spectral density of received signal
    unsigned int nfft = 1200;
    float psd[nfft];
    spgramcf_estimate_psd(nfft, y, num_samples, psd);

    // 
    // export results
    //
    
    // truncate to at most 10 symbols
    if (num_symbols > 10)
        num_symbols = 10;
    num_samples = k*num_symbols;
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"M = %u;\n", M);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"nfft        = %u;\n", nfft);

    fprintf(fid,"x   = zeros(1,num_samples);\n");
    fprintf(fid,"y   = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
    }
    // save power spectral density
    fprintf(fid,"psd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%4u) = %12.8f;\n", i+1, psd[i]);

    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"i = 1:k:num_samples;\n");
    fprintf(fid,"figure;\n");

    // plot time signal
    fprintf(fid,"subplot(2,1,1),\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  plot(t,real(y),'-', 'Color',[0 0.3 0.5]);\n");
    fprintf(fid,"  plot(t,imag(y),'-', 'Color',[0 0.5 0.3]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"ymax = ceil(max(abs(y))*5)/5;\n");
    fprintf(fid,"axis([0 num_symbols -ymax ymax]);\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('x(t)');\n");
    fprintf(fid,"grid on;\n");

    // plot PSD
    fprintf(fid,"subplot(2,1,2),\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"plot(f,psd,'LineWidth',1.5,'Color',[0.5 0 0]);\n");
    fprintf(fid,"axis([-0.5 0.5 -40 40]);\n");
    fprintf(fid,"xlabel('Normalized Frequency [f/F_s]');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
