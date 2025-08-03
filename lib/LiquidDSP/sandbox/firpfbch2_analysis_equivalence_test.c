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

#define DEBUG 1

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

    // generate filter
    // NOTE : these coefficients can be random; the purpose of this
    //        exercise is to demonstrate mathematical equivalence
#if 0
    unsigned int h_len = 2*m*num_channels;
    float h[h_len];
    for (i=0; i<h_len; i++) h[i] = randnf();
#else
    unsigned int h_len = 2*m*num_channels+1;
    float h[h_len];
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
    float complex Y1[2*num_symbols][num_channels];  // conventional output

    // generate input sequence
    for (i=0; i<num_samples; i++) {
        //y[i] = randnf() * cexpf(_Complex_I*randf()*2*M_PI);
        y[i] = (i==0) ? 1.0f : 0.0f;
        y[i] = cexpf(_Complex_I*sqrtf(2.0f)*i*i);
        printf("y[%3u] = %12.8f + %12.8fj\n", i, crealf(y[i]), cimagf(y[i]));
    }

    // 
    // run analysis filter bank
    //
#if 0
    unsigned int filter_index = 0;
#else
    unsigned int filter_index = num_channels/2-1;
#endif
    float complex y_hat;    // input sample
    float complex * r;      // buffer read pointer
    int toggle = 0;         // flag indicating buffer/filter alignment

    //
    for (i=0; i<2*num_symbols; i++) {

        // load buffers in blocks of num_channels/2
        for (j=0; j<num_channels/2; j++) {
            // grab sample
            y_hat = y[i*num_channels/2 + j];

            // push sample into buffer at filter index
            windowcf_push(w[filter_index], y_hat);

            // decrement filter index
            filter_index = (filter_index + num_channels - 1) % num_channels;
            //filter_index = (filter_index + 1) % num_channels;
        }

        // execute filter outputs
        // reversing order of output (not sure why this is necessary)
        unsigned int offset = toggle ? num_channels/2 : 0;
        toggle = 1-toggle;
        for (j=0; j<num_channels; j++) {
            unsigned int buffer_index  = (offset+j)%num_channels;
            unsigned int dotprod_index = j;

            windowcf_read(w[buffer_index], &r);
            //dotprod_crcf_execute(dp[dotprod_index], r, &X[num_channels-j-1]);
            dotprod_crcf_execute(dp[dotprod_index], r, &X[buffer_index]);
        }

        printf("***** i = %u\n", i);
        for (j=0; j<num_channels; j++)
            printf("  v2[%4u] = %12.8f + %12.8fj\n", j, crealf(X[j]), cimagf(X[j]));
        // execute DFT, store result in buffer 'x'
        fft_execute(fft);
        // scale fft output
        for (j=0; j<num_channels; j++)
            x[j] *= 1.0f / (num_channels);

        // move to output array
        for (j=0; j<num_channels; j++)
            Y0[i][j] = x[j];
    }
    // destroy objects
    for (i=0; i<num_channels; i++) {
        dotprod_crcf_destroy(dp[i]);
        windowcf_destroy(w[i]);
    }
    fft_destroy_plan(fft);


    // 
    // run traditional down-converter (inefficient)
    //
    // generate filter object
    firfilt_crcf f = firfilt_crcf_create(h, h_len);

    float dphi; // carrier frequency
    unsigned int n=0;
    for (i=0; i<num_channels; i++) {

        // reset filter
        firfilt_crcf_reset(f);

        // set center frequency
        dphi = 2.0f * M_PI * (float)i / (float)num_channels;

        // reset symbol counter
        n=0;

        for (j=0; j<num_samples; j++) {
            // push down-converted sample into filter
            firfilt_crcf_push(f, y[j]*cexpf(-_Complex_I*j*dphi));

            // compute output at the appropriate sample time
            assert(n<2*num_symbols);
            if ( ((j+1)%(num_channels/2))==0 ) {
                firfilt_crcf_execute(f, &Y1[n][i]);
                n++;
            }
        }
        assert(n==2*num_symbols);

    }
    firfilt_crcf_destroy(f);

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

#if 0
    // print traditional channelizer
    printf("\n");
    printf("traditional channelizer:\n");
    for (i=0; i<2*num_symbols; i++) {
        printf("%2u:", i);
        for (j=0; j<num_channels; j++) {
            printf("%6.3f+%6.3fj, ", crealf(Y1[i][j]), cimagf(Y1[i][j]));
        }
        printf("\n");
    }

    // 
    // compare results
    // 
    float mse[num_channels];
    float complex d;
    for (i=0; i<num_channels; i++) {
        mse[i] = 0.0f;
        for (j=0; j<2*num_symbols; j++) {
            d = Y0[j][i] - Y1[j][i];
            mse[i] += crealf(d*conjf(d));
        }

        mse[i] /= num_symbols;
    }
    printf("\n");
    printf(" e:");
    for (i=0; i<num_channels; i++)
        printf("%12.4e    ", sqrt(mse[i]));
    printf("\n");
#endif

    printf("done.\n");
    return 0;

}

