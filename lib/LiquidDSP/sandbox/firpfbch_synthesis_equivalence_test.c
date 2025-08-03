//
// firpfbch_synthesis_equivalence_test.c
//

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG 1

int main() {
    // options
    unsigned int num_channels=4;    // number of channels
    unsigned int m=3;               // filter delay
    unsigned int num_symbols=5;     // number of symbols

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

    // generate inverse DFT object
    float complex x[num_channels];  // time-domain buffer
    float complex X[num_channels];  // freq-domain buffer
#if 1
    fftplan ifft = fft_create_plan(num_channels, X, x, LIQUID_FFT_BACKWARD, 0);
#else
    fftplan ifft = fft_create_plan(num_channels, X, x, LIQUID_FFT_FORWARD, 0);
#endif

    // generate filter object
    firfilt_crcf f = firfilt_crcf_create(h, h_len);

    float complex Y[num_symbols][num_channels];     // channelized input
    float complex y0[num_samples];                  // time-domain output
    float complex y1[num_samples];                  // time-domain output

    // generate input sequence (complex noise)
    for (i=0; i<num_symbols; i++) {
        for (j=0; j<num_channels; j++)
            Y[i][j] = randnf() * cexpf(_Complex_I*randf()*2*M_PI);

#if 0
        for (j=0; j<num_channels; j++)
            Y[i][j] = i==0 ? randnf() * cexpf(_Complex_I*randf()*2*M_PI) : 0.0f;
#endif
    }

    // 
    // run synthesis filter bank
    //

    float complex * r;      // read pointer
    for (i=0; i<num_symbols; i++) {

        // load buffers
        for (j=0; j<num_channels; j++) {
            X[j] = Y[i][j];
        }

        // execute inverse DFT, store result in buffer 'x'
        fft_execute(ifft);

        // push samples into filter bank and execute
        for (j=0; j<num_channels; j++) {
            windowcf_push(w[j], x[j]);
            windowcf_read(w[j], &r);
            dotprod_crcf_execute(dp[j], r, &y0[i*num_channels+j]);

            // normalize by DFT scaling factor
            //y0[i*num_channels+j] /= (float) num_channels;
        }
    }

    // 
    // run traditional up-converter (inefficient)
    //

    // clear output array
    for (i=0; i<num_samples; i++)
        y1[i] = 0.0f;

    unsigned int n;
    float dphi; // carrier frequency
    float complex y_hat;
    for (i=0; i<num_channels; i++) {
        // reset filter
        firfilt_crcf_reset(f);

        // set center frequency
        dphi = 2.0f * M_PI * (float)i / (float)num_channels;

        // reset input symbol counter
        n=0;

        for (j=0; j<num_samples; j++) {

            // interpolate sequence
            if ( (j%num_channels)==0 ) {
                assert(n<num_symbols);
                firfilt_crcf_push(f, Y[n][i]);
                n++;
            } else {
                firfilt_crcf_push(f, 0);
            }
            firfilt_crcf_execute(f, &y_hat);

            // accumulate up-converted sample
            y1[j] += y_hat * cexpf(_Complex_I*j*dphi);
        }
        assert(n==num_symbols);
    }

    // destroy objects
    for (i=0; i<num_channels; i++) {
        dotprod_crcf_destroy(dp[i]);
        windowcf_destroy(w[i]);
    }
    fft_destroy_plan(ifft);

    firfilt_crcf_destroy(f);


    // 
    // print channelizer outputs
    //
    printf("\n");
    printf("output: filterbank:             traditional:\n");
    for (i=0; i<num_samples; i++) {
        printf("%3u: %10.5f+%10.5fj  %10.5f+%10.5fj\n",
            i,
            crealf(y0[i]), cimagf(y0[i]),
            crealf(y1[i]), cimagf(y1[i]));
    }


    // 
    // compare results
    // 
    float mse = 0.0f;
    float complex d;
    for (i=0; i<num_samples; i++) {
        d = y0[i] - y1[i];
        mse += crealf(d*conjf(d));
    }
    mse /= num_samples;
    printf("\n");
    printf("rmse: %12.4e\n", sqrtf(mse));

    printf("done.\n");
    return 0;

}

