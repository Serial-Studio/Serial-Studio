//
// firpfbch_analysis_test.c
//

#include <stdio.h>
#include <math.h>

#include "liquid.internal.h"

int main() {
    // options
    unsigned int num_channels=8;    // number of channels
    unsigned int m=2;               // filter delay
    float As=60;                    // stop-band attenuation [dB]
    unsigned int num_symbols=8;     // number of symbols

    // create filterbank objects
    firpfbch ca0 = firpfbch_create(num_channels, m, As, 0, FIRPFBCH_NYQUIST, 0);
    firpfbch ca1 = firpfbch_create(num_channels, m, As, 0, FIRPFBCH_NYQUIST, 0);

    unsigned int i, j;
    float complex x[num_channels];  // time-domain input
    float complex y0[num_channels]; // channelized output
    float complex y1[num_channels]; // channelized output

#if 0
    // push samples channelizer 1
    float complex x_hat;
    for (i=0; i<delay; i++) {
        // generate random sample
        x_hat = randnf() * cexpf(_Complex_I*randf()*2*M_PI);

        // push sample into channelizer
        firpfbch_analyzer_push(ca0, x_hat);

        // run analyzer
        firpfbch_analyzer_run(ca0, y0);

        // save run state
        firpfbch_analyzer_saverunstate(ca0);
    }
#endif

    // run analyzers
    for (i=0; i<num_symbols; i++) {

        // generate input sequence (complex noise)
        for (j=0; j<num_channels; j++)
            x[j] = randnf() * cexpf(_Complex_I*randf()*2*M_PI);

        // 
        // analyzer 0 : run individual sample execution
        //

        // push samples into channelizer 1
        for (j=0; j<num_channels; j++) {
            firpfbch_analyzer_push(ca0, x[j]);
    
            // run analyzer every time, just for kicks
            firpfbch_analyzer_run(ca0, y0);
        }

        // run analyzer as many times as desired
        firpfbch_analyzer_run(ca0, y0);
        firpfbch_analyzer_run(ca0, y0);
        firpfbch_analyzer_run(ca0, y0);

        // save run state ONLY at the appropriate time
        firpfbch_analyzer_saverunstate(ca0);
        
        // 
        // analyzer 1 : run regular execution
        //
        firpfbch_analyzer_execute(ca1, x, y1);

        for (j=0; j<num_channels; j++) {
            printf("      %3u [%8.5f + j%8.5f, %8.5f + j%8.5f]\n",
                    j,
                    crealf(y0[j]),
                    cimagf(y0[j]),
                    crealf(y1[j]),
                    cimagf(y1[j]));
        }

        // 
        // compare result
        // 
        float mse = 0.0f;
        for (j=0; j<num_channels; j++) {
            float complex d = y0[j] - y1[j];
            mse += crealf( d*conjf(d) );
        }
        mse /= num_channels;
        printf("  %4u : mse = %12.4e\n", i, mse);
    }

    // destroy objects
    firpfbch_destroy(ca0);
    firpfbch_destroy(ca1);

    printf("done.\n");
    return 0;
}

