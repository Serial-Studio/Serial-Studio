//
// firpfbch_analysis_equivalence_test.c
//

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG 1

int main() {
    // options
    unsigned int num_channels=4;    // number of channels
    unsigned int m=5;               // filter delay
    unsigned int num_symbols=12;    // number of symbols

    // derived values
    unsigned int num_samples = num_channels * num_symbols;

    unsigned int i;
    unsigned int j;

    // generate filter
    // NOTE : these coefficients can be random; the purpose of this
    //        exercise is to demonstrate mathematical equivalence
    unsigned int h_len = 2*m*num_channels;
    float h[h_len];
    for (i=0; i<h_len; i++) h[i] = randnf();
    //for (i=0; i<h_len; i++) h[i] = 0.1f*i;
    //for (i=0; i<h_len; i++) h[i] = (i<=m) ? 1.0f : 0.0f;
    //for (i=0; i<h_len; i++) h[i] = 1.0f;

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
        // sub-sample prototype filter, loading coefficients in
        // reverse order
#if 0
        for (j=0; j<h_sub_len; j++)
            h_sub[j] = h[j*num_channels+i];
#else
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
#if 0
    fftplan fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_BACKWARD, 0);
#else
    fftplan fft = fft_create_plan(num_channels, X, x, LIQUID_FFT_FORWARD, 0);
#endif

    // generate filter object
    firfilt_crcf f = firfilt_crcf_create(h, h_len);

    float complex y[num_samples];                   // time-domain input
    float complex Y0[num_symbols][num_channels];    // channelized output
    float complex Y1[num_symbols][num_channels];    // channelized output

    // generate input sequence (complex noise)
    for (i=0; i<num_samples; i++)
        y[i] = randnf() * cexpf(_Complex_I*randf()*2*M_PI);

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
    for (i=0; i<num_symbols; i++) {

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
            dotprod_crcf_execute(dp[j], r, &X[num_channels-j-1]);
        }

        // execute DFT, store result in buffer 'x'
        fft_execute(fft);

        // move to output array
        for (j=0; j<num_channels; j++)
            Y0[i][j] = x[j];
    }

    // 
    // run traditional down-converter (inefficient)
    //
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
            assert(n<num_symbols);
            if ( ((j+1)%num_channels)==0 ) {
                firfilt_crcf_execute(f, &Y1[n][i]);
                n++;
            }
        }
        assert(n==num_symbols);

    }

    // destroy objects
    for (i=0; i<num_channels; i++) {
        dotprod_crcf_destroy(dp[i]);
        windowcf_destroy(w[i]);
    }
    fft_destroy_plan(fft);

    firfilt_crcf_destroy(f);

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

    // print traditional channelizer
    printf("\n");
    printf("traditional channelizer:\n");
    for (i=0; i<num_symbols; i++) {
        printf("%3u: ", i);
        for (j=0; j<num_channels; j++) {
            printf("  %8.5f+j%8.5f, ", crealf(Y1[i][j]), cimagf(Y1[i][j]));
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
        for (j=0; j<num_symbols; j++) {
            d = Y0[j][i] - Y1[j][i];
            mse[i] += crealf(d*conjf(d));
        }

        mse[i] /= num_symbols;
    }
    printf("\n");
    printf("rmse: ");
    for (i=0; i<num_channels; i++)
        printf("%12.4e          ", sqrt(mse[i]));
    printf("\n");

    printf("done.\n");
    return 0;

}

