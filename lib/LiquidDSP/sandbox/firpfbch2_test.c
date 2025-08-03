//
// firpfbch2_analysis_equivalence_test.c
//
// This script tests the equivalence of a finite impulse response (fir)
// polyphase filter bank (pfb) channelizer with two outputs per branch
// per input symbol.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "liquid.internal.h"

// print debug status
#define DEBUG 0

// use pseudo-random coefficients; this is useful for determining
// functional correctness (no symmetry in filter)
#define RANDOM_COEFFICIENTS 0

int main(int argc, char*argv[])
{
    // options
    unsigned int num_channels=6;    // number of channels (must be even)
    unsigned int m=4;               // filter delay
    unsigned int num_symbols=4*m;   // number of symbols

    // validate input
    if (num_channels%2) {
        fprintf(stderr,"error: %s, number of channels must be even\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int num_samples = num_channels * num_symbols;

    unsigned int i;
    unsigned int j;

    //
    // ANALYSIS FILTERBANK
    //

    // generate filter
    // NOTE : these coefficients can be random; the purpose of this
    //        exercise is to demonstrate mathematical equivalence
    unsigned int h_len = 2*m*num_channels+1;
    float h[h_len];
#if RANDOM_COEFFICIENTS
    // testing: set filter coefficients to random values
    unsigned int s0 = 1;
    unsigned int p0 = 524287;   // large prime number
    unsigned int g0 =   1031;   // another large prime number
    for (i=0; i<h_len; i++) {
        s0 = (s0 * p0) % g0;
        h[i] = (float)s0 / (float)g0 - 0.5f;
    }
#else
    // NOTE: 81.29528 dB > beta = 8.00000 (6 channels, m=4)
    liquid_firdes_kaiser(h_len, 1.0f/(float)num_channels, 81.29528f, 0.0f, h);
#endif
    // normalize
    float hsum = 0.0f;
    for (i=0; i<h_len; i++) hsum += h[i];
    for (i=0; i<h_len; i++) h[i] = h[i] * num_channels / hsum;

    // sub-sampled filters for M=6 channels, m=4, beta=8.0
    //  -3.2069e-19  -6.7542e-04  -1.3201e-03   2.2878e-18   3.7613e-03   5.8033e-03
    //  -7.2899e-18  -1.2305e-02  -1.7147e-02   1.6510e-17   3.1187e-02   4.0974e-02
    //  -3.0032e-17  -6.8026e-02  -8.6399e-02   4.6273e-17   1.3732e-01   1.7307e-01
    //  -6.2097e-17  -2.8265e-01  -3.7403e-01   7.3699e-17   8.0663e-01   1.6438e+00
    //   2.0001e+00   1.6438e+00   8.0663e-01   7.3699e-17  -3.7403e-01  -2.8265e-01
    //  -6.2097e-17   1.7307e-01   1.3732e-01   4.6273e-17  -8.6399e-02  -6.8026e-02
    //  -3.0032e-17   4.0974e-02   3.1187e-02   1.6510e-17  -1.7147e-02  -1.2305e-02
    //  -7.2899e-18   5.8033e-03   3.7613e-03   2.2878e-18  -1.3201e-03  -6.7542e-04

    // create filterbank manually
    dotprod_crcf dp[num_channels];  // vector dot products
    windowcf w[num_channels];       // window buffers

#if DEBUG
    // print coefficients
    printf("h_prototype:\n");
    for (i=0; i<h_len; i++)
        printf("  h[%3u] = %12.8f\n", i, h[i]);
#endif

    // create objects
    unsigned int h_sub_len = 2*m;
    float h_sub[h_sub_len];
    for (i=0; i<num_channels; i++) {
        // sub-sample prototype filter
#if 0
        for (j=0; j<h_sub_len; j++)
            h_sub[j] = h[j*num_channels+i];
#else
        // load coefficients in reverse order
        for (j=0; j<h_sub_len; j++)
            h_sub[h_sub_len-j-1] = h[j*num_channels+i];
#endif

        // create window buffer and dotprod objects
        dp[i] = dotprod_crcf_create(h_sub, h_sub_len);
        w[i]  = windowcf_create(h_sub_len);

#if DEBUG
        printf("h_sub[%u] : \n", i);
        for (j=0; j<h_sub_len; j++)
            printf("  h[%3u] = %12.8f\n", j, h_sub[j]);
#endif
    }

    // generate DFT object
    float complex x[num_channels];  // time-domain buffer
    float complex X[num_channels];  // freq-domain buffer
#if 1
    fftplan fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_BACKWARD, 0);
#else
    fftplan fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_FORWARD, 0);
#endif

    float complex y[num_samples];                   // time-domain input
    float complex Y0[2*num_symbols][num_channels];  // channelizer output
    float complex z[num_samples];                   // time-domain output

    // generate input sequence
    for (i=0; i<num_samples; i++) {
        //y[i] = randnf() * cexpf(_Complex_I*randf()*2*M_PI);
        //y[i] = (i==0) ? 1.0f : 0.0f;
        //y[i] = expf(-0.1f*i);
        y[i] = cexpf( (0.1f*_Complex_I - 0.1f)*i );
        printf("y[%3u] = %12.8f + %12.8fj\n", i, crealf(y[i]), cimagf(y[i]));
    }

    // 
    // run analysis filter bank
    //
    float complex y_hat;    // input sample
    float complex * r;      // buffer read pointer
    int toggle = 0;         // flag indicating buffer/filter alignment

    //
    for (i=0; i<2*num_symbols; i++) {

        // load buffers in blocks of num_channels/2 starting
        // in the middle of the filter bank and moving in the
        // negative direction
        unsigned int base_index = toggle ? num_channels : num_channels/2;
        for (j=0; j<num_channels/2; j++) {
            // grab sample
            y_hat = y[i*num_channels/2 + j];

            // push sample into buffer at filter index
            windowcf_push(w[base_index-j-1], y_hat);
        }

        // execute filter outputs
        // reversing order of output (not sure why this is necessary)
        unsigned int offset = toggle ? num_channels/2 : 0;
        for (j=0; j<num_channels; j++) {
            unsigned int buffer_index  = (offset+j)%num_channels;
            unsigned int dotprod_index = j;

            windowcf_read(w[buffer_index], &r);
            //dotprod_crcf_execute(dp[dotprod_index], r, &X[num_channels-j-1]);
            dotprod_crcf_execute(dp[dotprod_index], r, &X[buffer_index]);
        }

#if DEBUG
        printf("***** i = %u\n", i);
        for (j=0; j<num_channels; j++)
            printf("  v2[%4u] = %12.8f + %12.8fj\n", j, crealf(X[j]), cimagf(X[j]));
#endif
        // execute DFT, store result in buffer 'x'
        fft_execute(fft);
        // scale result by 1/num_channels (C transform)
        for (j=0; j<num_channels; j++)
            x[j] *= 1.0f / (num_channels);

        // move to output array
        for (j=0; j<num_channels; j++)
            Y0[i][j] = x[j];
        toggle = 1-toggle;
    }

    // print filterbank channelizer
    printf("\n");
    printf("filterbank channelizer:\n");
    for (i=0; i<2*num_symbols; i++) {
        printf("%2u:", i);
        for (j=0; j<num_channels; j++) {
            printf("%6.3f+%6.3fj, ", crealf(Y0[i][j]), cimagf(Y0[i][j]));
        }
        printf("\n");
    }
    // destroy objects
    for (i=0; i<num_channels; i++) {
        dotprod_crcf_destroy(dp[i]);
        windowcf_destroy(w[i]);
    }
    fft_destroy_plan(fft);


    //
    // SYNTHESIS FILTERBANK
    //

    // generate synthesis filter
    unsigned int f_len = 2*m*num_channels+1;
    float f[f_len];
#if RANDOM_COEFFICIENTS
    // testing: set filter coefficients to random values
    unsigned int s1 = 1;
    unsigned int p1 = 131071;   // large prime number
    unsigned int g1 =   1031;   // another large prime number
    for (i=0; i<f_len; i++) {
        s1 = (s1 * p1) % g1;
        f[i] = (float)s1 / (float)g1 - 0.5f;
    }
#else
    // NOTE: 81.29528 dB > beta = 8.00000 (6 channels, m=4)
    liquid_firdes_kaiser(f_len, 0.5f/(float)num_channels, 81.29528f, 0.0f, f);
#endif
    // normalize
    float fsum = 0.0f;
    for (i=0; i<f_len; i++) fsum += f[i];
    for (i=0; i<f_len; i++) f[i] = f[i] * num_channels / fsum;

    // sub-sampled filters for M=6 channels, m=4, beta=8.0
    //  -1.6032e-19  -3.8990e-04  -1.3199e-03  -2.6684e-03  -3.7608e-03  -3.3501e-03
    //   3.6445e-18   7.1034e-03   1.7145e-02   2.6960e-02   3.1183e-02   2.3653e-02
    //  -1.5014e-17  -3.9269e-02  -8.6387e-02  -1.2593e-01  -1.3730e-01  -9.9906e-02
    //   3.1044e-17   1.6316e-01   3.7398e-01   6.0172e-01   8.0652e-01   9.4890e-01
    //   9.9990e-01   9.4890e-01   8.0652e-01   6.0172e-01   3.7398e-01   1.6316e-01
    //   3.1044e-17  -9.9906e-02  -1.3730e-01  -1.2593e-01  -8.6387e-02  -3.9269e-02
    //  -1.5014e-17   2.3653e-02   3.1183e-02   2.6960e-02   1.7145e-02   7.1034e-03
    //   3.6445e-18  -3.3501e-03  -3.7608e-03  -2.6684e-03  -1.3199e-03  -3.8990e-04

    // create filterbank manually
    //dotprod_crcf dp[num_channels];  // vector dot products
    windowcf w0[num_channels];       // window buffers
    windowcf w1[num_channels];       // window buffers

#if DEBUG
    // print coefficients
    printf("f_prototype:\n");
    for (i=0; i<f_len; i++)
        printf("  f[%3u] = %12.8f\n", i, f[i]);
#endif

    // create objects
    unsigned int f_sub_len = 2*m;
    float f_sub[f_sub_len];
    for (i=0; i<num_channels; i++) {
        // sub-sample prototype filter
#if 0
        for (j=0; j<f_sub_len; j++)
            f_sub[j] = f[j*num_channels+i];
#else
        // load coefficients in reverse order
        for (j=0; j<f_sub_len; j++)
            f_sub[f_sub_len-j-1] = f[j*num_channels+i];
#endif

        // create window buffer and dotprod objects
        dp[i] = dotprod_crcf_create(f_sub, f_sub_len);
        w0[i] = windowcf_create(f_sub_len);
        w1[i] = windowcf_create(f_sub_len);

#if DEBUG
        printf("f_sub[%u] : \n", i);
        for (j=0; j<f_sub_len; j++)
            printf("  f[%3u] = %12.8f\n", j, f_sub[j]);
#endif
    }

    // generate DFT object
#if 1
    fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_BACKWARD, 0);
#else
    fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_FORWARD, 0);
#endif
    
    // 
    // run synthesis filter bank
    //
    toggle = 0;
    unsigned int n=0;
    for (i=0; i<2*num_symbols; i++) {
        // load ifft input
        // TODO: select frequency-band filtering
        for (j=0; j<num_channels; j++) {
            X[j] = Y0[i][j];
        }
        // execute inverse DFT, store result in buffer 'x'
        fft_execute(fft);

        // scale result by 1/num_channels (C transform)
        for (j=0; j<num_channels; j++)
            x[j] *= 1.0f / (num_channels);
        // scale result by num_channels/2
        for (j=0; j<num_channels; j++)
            x[j] *= (float)num_channels / 2.0f;

#if DEBUG
        // print result
        printf("***** i = %u\n", i);
        for (j=0; j<num_channels; j++)
            printf("  v5[%4u] = %12.8f + %12.8fj\n", j, crealf(x[j]), cimagf(x[j]));
#endif

        // push samples into appropriate buffer
        windowcf * buffer = (toggle == 0 ? w1 : w0);
        for (j=0; j<num_channels; j++)
            windowcf_push(buffer[j], x[j]);

        // compute filter outputs
        float complex * r0;
        float complex * r1;
        float complex z0;
        float complex z1;
        for (j=0; j<num_channels/2; j++) {
            // buffer index
            unsigned int b = (toggle == 0) ? j : j+num_channels/2;

            windowcf_read(w0[b], &r0);
            windowcf_read(w1[b], &r1);

            // buffer pointers
            float complex * p0 = toggle ? r0 : r1;
            float complex * p1 = toggle ? r1 : r0;

#if DEBUG
            // plot registers
            unsigned int k;
            printf("reg[0] : ");
            for (k=0; k<f_sub_len; k++)
                printf("(%9.2e,%9.2e),", crealf(r0[k]), cimagf(r0[k]));
            printf("\n");
            printf("reg[1] : ");
            for (k=0; k<f_sub_len; k++)
                printf("(%9.2e,%9.2e),", crealf(r1[k]), cimagf(r1[k]));
            printf("\n");
#endif

            // run dot products
            dotprod_crcf_execute(dp[j],                p0, &z0);
            dotprod_crcf_execute(dp[j+num_channels/2], p1, &z1);

#if DEBUG
            printf("  z(%3u) = %12.8f + %12.8fj\n", i, crealf(z0+z1), cimagf(z0+z1));
#endif
            z[n++] = z0 + z1;
        }
        toggle = 1-toggle;
    }
    assert( n == num_samples );
    
    // print output
    printf("\n");
    printf("filterbank synthesizer:\n");
    for (i=0; i<num_samples; i++) {
        printf("%6u : %12.8f+%12.8fj\n", i, crealf(z[i]), cimagf(z[i]));
    }

    // destroy objects
    for (i=0; i<num_channels; i++) {
        dotprod_crcf_destroy(dp[i]);
        windowcf_destroy(w0[i]);
        windowcf_destroy(w1[i]);
    }
    fft_destroy_plan(fft);

    printf("done.\n");
    return 0;

}

