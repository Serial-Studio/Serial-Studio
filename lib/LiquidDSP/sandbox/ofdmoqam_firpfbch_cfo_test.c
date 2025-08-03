//
// sandbox/ofdmoqam_firpfbch_cfo_test.c
//
// Tests the validity of OFDM/OQAM carrier frequency offset
// recovery using firpfbch_cccf channelizer objects.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <complex.h>

#include "liquid.h"

#define OUTPUT_FILENAME "ofdmoqam_firpfbch_cfo_test.m"

#define DEBUG 1


int main() {
    // options
    unsigned int num_channels=64;   // must be even number
    unsigned int num_symbols=16;    // number of symbols
    unsigned int m=3;               // filter delay (symbols)
    float beta = 0.9f;              // filter excess bandwidth factor
    float phi = 0.0f;               // carrier phase offset;
    float dphi = 0.04f;            // carrier frequency offset

    // number of frames (compensate for filter delay)
    unsigned int num_frames = num_symbols + 2*m;
    unsigned int num_samples = num_channels * num_frames;
    unsigned int i;
    unsigned int j;

    // create filter prototype
    unsigned int h_len = 2*num_channels*m + 1;
    float h[h_len];
    float complex hc[h_len];
    float complex gc[h_len];
    liquid_firdes_rkaiser(num_channels, m, beta, 0.0f, h);
    unsigned int g_len = 2*num_channels*m;
    for (i=0; i<g_len; i++) {
        hc[i] = h[i];
        gc[i] = h[g_len-i-1] * cexpf(_Complex_I*dphi*i);
    }

    // data arrays
    float complex s[num_channels];                  // input symbols
    float complex y[num_samples];                   // time-domain samples
    float complex Y0[num_frames][num_channels];     // channelized output
    float complex Y1[num_frames][num_channels];     // channelized output

    // create ofdm/oqam generator object and generate data
    ofdmoqam qs = ofdmoqam_create(num_channels, m, beta, 0.0f, LIQUID_SYNTHESIZER, 0);
    for (i=0; i<num_frames; i++) {
        for (j=0; j<num_channels; j++) {
            if (i<num_symbols) {
#if 0
                // QPSK on all subcarriers
                s[j] = (rand() % 2 ? 1.0f : -1.0f) +
                       (rand() % 2 ? 1.0f : -1.0f) * _Complex_I;
                s[j] *= 1.0f / sqrtf(2.0f);
#else
                // BPSK on even subcarriers
                s[j] =  rand() % 2 ? 1.0f : -1.0f;
                s[j] *= (j%2)==0 ? 1.0f : 0.0f;
#endif
            } else {
                s[j] = 0.0f;
            }
        }

        // run synthesizer
        ofdmoqam_execute(qs, s, &y[i*num_channels]);
    }
    ofdmoqam_destroy(qs);

    // channel
    for (i=0; i<num_samples; i++)
        y[i] *= cexpf(_Complex_I*(phi + dphi*i));


    //
    // analysis filterbank (receiver)
    //

    // create filterbank manually
    dotprod_cccf dp[num_channels];  // vector dot products
    windowcf w[num_channels];       // window buffers

#if DEBUG
    // print coefficients
    printf("h_prototype:\n");
    for (i=0; i<h_len; i++)
        printf("  h[%3u] = %12.8f\n", i, h[i]);
#endif

    // create objects
    unsigned int gc_sub_len = 2*m;
    float complex gc_sub[gc_sub_len];
    for (i=0; i<num_channels; i++) {
        // sub-sample prototype filter, loading coefficients in
        // reverse order
#if 0
        for (j=0; j<gc_sub_len; j++)
            gc_sub[j] = h[j*num_channels+i];
#else
        for (j=0; j<gc_sub_len; j++)
            gc_sub[gc_sub_len-j-1] = gc[j*num_channels+i];
#endif

        // create window buffer and dotprod objects
        dp[i] = dotprod_cccf_create(gc_sub, gc_sub_len);
        w[i]  = windowcf_create(gc_sub_len);

#if DEBUG
        printf("gc_sub[%u] : \n", i);
        for (j=0; j<gc_sub_len; j++)
            printf("  g[%3u] = %12.8f + %12.8f\n", j, crealf(gc_sub[j]), cimagf(gc_sub[j]));
#endif
    }

    // generate DFT object
    float complex x[num_channels];  // time-domain buffer
    float complex X[num_channels];  // freq-domain buffer
#if 0
    fftplan fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_BACKWARD, 0);
#else
    fftplan fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_FORWARD, 0);
#endif

    // 
    // run analysis filter bank
    //
#if 0
    unsigned int filter_index = 0;
#else
    unsigned int filter_index = num_channels-1;
#endif
    float complex y_hat;    // input sample
    float complex * r;      // read pointer
    for (i=0; i<num_frames; i++) {

        // load buffers
        for (j=0; j<num_channels; j++) {
            // grab sample
            y_hat = y[i*num_channels + j];

            // push sample into buffer at filter index
            windowcf_push(w[filter_index], y_hat);

            // decrement filter index
            filter_index = (filter_index + num_channels - 1) % num_channels;
            //filter_index = (filter_index + 1) % num_channels;
        }

        // execute filter outputs, reversing order of output (not
        // sure why this is necessary)
        for (j=0; j<num_channels; j++) {
            windowcf_read(w[j], &r);
            dotprod_cccf_execute(dp[j], r, &X[num_channels-j-1]);
        }

#if 1
        // compensate for carrier frequency offset (before transform)
        for (j=0; j<num_channels; j++) {
            X[j] *= cexpf(-_Complex_I*(dphi*i*num_channels));
        }
#endif

        // execute DFT, store result in buffer 'x'
        fft_execute(fft);

#if 0
        // compensate for carrier frequency offset (after transform)
        for (j=0; j<num_channels; j++) {
            x[j] *= cexpf(-_Complex_I*(dphi*i*num_channels));
        }
#endif

        // move to output array
        for (j=0; j<num_channels; j++)
            Y0[i][j] = x[j];
    }


    // destroy objects
    for (i=0; i<num_channels; i++) {
        dotprod_cccf_destroy(dp[i]);
        windowcf_destroy(w[i]);
    }
    fft_destroy_plan(fft);

#if 0
    // print filterbank channelizer
    printf("\n");
    printf("filterbank channelizer:\n");
    for (i=0; i<num_symbols; i++) {
        printf("%3u: ", i);
        for (j=0; j<num_channels; j++) {
            printf("  %8.5f+j%8.5f, ", crealf(Y0[i][j]), cimagf(Y0[i][j]));
        }
        printf("\n");
    }
#endif

    // 
    // export data
    //
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"num_channels=%u;\n", num_channels);
    fprintf(fid,"num_symbols=%u;\n", num_symbols);
    fprintf(fid,"num_frames = %u;\n", num_frames);
    fprintf(fid,"num_samples = num_frames*num_channels;\n");

    fprintf(fid,"y = zeros(1,%u);\n",  num_samples);
    fprintf(fid,"Y0 = zeros(%u,%u);\n", num_frames, num_channels);
    fprintf(fid,"Y1 = zeros(%u,%u);\n", num_frames, num_channels);
    
    for (i=0; i<num_frames; i++) {
        for (j=0; j<num_channels; j++) {
            fprintf(fid,"Y0(%4u,%4u) = %12.4e + j*%12.4e;\n", i+1, j+1, crealf(Y0[i][j]), cimagf(Y0[i][j]));
            fprintf(fid,"Y1(%4u,%4u) = %12.4e + j*%12.4e;\n", i+1, j+1, crealf(Y1[i][j]), cimagf(Y1[i][j]));
        }
    }

    // plot BPSK results
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(Y0(:,1:2:end),'x');\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.2*sqrt(num_channels));\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

