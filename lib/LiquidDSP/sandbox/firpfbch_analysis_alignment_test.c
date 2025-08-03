//
// sandbox/firpfbch_anayalis_sync_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firpfbch_analysis_sync_test.m"

// forward declaration of internal methods

void firpfbch_crcf_analyzer_push(firpfbch_crcf _q,
                                 float complex _x);

void firpfbch_crcf_analyzer_run(firpfbch_crcf   _q,
                                unsigned int    _k,
                                float complex * _X);

int main() {
    // options
    unsigned int num_channels=4;    // must be even number
    unsigned int num_symbols=12;    // number of symbols
    unsigned int m=3;               // filter delay (symbols)
    float beta = 0.9f;              // filter excess bandwidth factor
    unsigned int delay = 3;         // initial sampling delay
    int ftype = LIQUID_FIRFILT_ARKAISER;

    // derived values
    unsigned int num_frames = num_symbols + 2*m;            // compensate for filter delay
    unsigned int num_samples = num_channels * num_frames;   // total number of samples

    // data arrays
    float complex x[num_samples];
    float complex Y0[num_frames][num_channels];
    float complex Y1[num_frames][num_channels];

    // create analyzer objects
    firpfbch_crcf ca0 = firpfbch_crcf_create_rnyquist(LIQUID_ANALYZER, num_channels, m, beta, ftype);
    firpfbch_crcf ca1 = firpfbch_crcf_create_rnyquist(LIQUID_ANALYZER, num_channels, m, beta, ftype);

    unsigned int i;
    unsigned int j;

    // generate random input sequence
    for (i=0; i<num_samples; i++)
        x[i] = randnf()*cexpf(_Complex_I*2*M_PI*randf());

    // push several dummy samples into first analyzer (emulates
    // timing offset)
    float complex x_hat;
    for (i=0; i<delay; i++) {
        x_hat = randnf()*cexpf(_Complex_I*2*M_PI*randf());

        firpfbch_crcf_analyzer_push(ca0, x_hat);
    }

    // run analyzers
    for (i=0; i<num_frames; i++) {
#if 1
        for (j=0; j<num_channels; j++) {
            // push samples into analyzers
            firpfbch_crcf_analyzer_push(ca0, x[i*num_channels+j]);
            firpfbch_crcf_analyzer_push(ca1, x[i*num_channels+j]);
        }
        firpfbch_crcf_analyzer_run(ca0,     0, &Y0[i][0]);
        firpfbch_crcf_analyzer_run(ca1, delay, &Y1[i][0]);
#else
        // execute analysis filterbank channelizers
        firpfbch_crcf_analyzer_execute(ca0, &x[i*num_channels], &Y0[i][0]);
        firpfbch_crcf_analyzer_execute(ca1, &x[i*num_channels], &Y1[i][0]);
#endif
    }

    // destroy objects
    firpfbch_crcf_destroy(ca0);
    firpfbch_crcf_destroy(ca1);

    // print channelizer 0
    printf("\n");
    printf("filterbank channelizer:\n");
    for (i=0; i<num_symbols; i++) {
        printf("%c %3u: ", i<2*m ? ' ' : '*', i);
        for (j=0; j<num_channels; j++) {
            printf("  %8.5f+j%8.5f, ", crealf(Y0[i][j]), cimagf(Y0[i][j]));
        }
        printf("\n");
    }

    // print channelizer 1
    printf("\n");
    printf("traditional channelizer:\n");
    for (i=0; i<num_symbols; i++) {
        printf("%c %3u: ", i<2*m ? ' ' : '*', i);
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

        // compute MSE for this channel, removing initial
        // timing delay
        for (j=2*m; j<num_frames; j++) {
            d = Y0[j][i] - Y1[j][i];
            mse[i] += crealf(d*conjf(d));
        }

        mse[i] /= num_symbols;
    }
    printf("\n");
    printf("  rmse: ");
    for (i=0; i<num_channels; i++)
        printf("%12.4e          ", sqrt(mse[i]));
    printf("\n");



    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"num_channels=%u;\n", num_channels);
    fprintf(fid,"num_symbols=%u;\n", num_symbols);
    fprintf(fid,"num_samples=%u;\n", num_samples);

    fprintf(fid,"X = zeros(%u,%u);\n", num_channels, num_frames);
    fprintf(fid,"y = zeros(1,%u);\n",  num_samples);
    fprintf(fid,"Y = zeros(%u,%u);\n", num_channels, num_frames);

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

