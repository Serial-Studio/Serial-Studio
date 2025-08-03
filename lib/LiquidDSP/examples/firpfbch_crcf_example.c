//
// firpfbch_crcf_example.c
//
// Finite impulse response (FIR) polyphase filter bank (PFB)
// channelizer example.  This example demonstrates the functionality
// of the polyphase filter bank channelizer and how its output
// is mathematically equivalent to a series of parallel down-
// converters (mixers/decimators). Both the synthesis and analysis
// filter banks are presented.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "firpfbch_crcf_example.m"

int main() {
    // options
    unsigned int num_channels=4;    // number of channels
    unsigned int p=3;               // filter length (symbols)
    unsigned int num_symbols=6;     // number of symbols

    // derived values
    unsigned int num_samples = num_channels * num_symbols;

    unsigned int i;
    unsigned int j;

    // generate synthesis filter
    // NOTE : these coefficients can be random; the purpose of this
    //        exercise is to demonstrate mathematical equivalence
    unsigned int h_len = p*num_channels;
    float h[h_len];
    for (i=0; i<h_len; i++) h[i] = randnf();

    // generate analysis filter
    unsigned int g_len = p*num_channels;
    float g[g_len];
    for (i=0; i<g_len; i++)
        g[i] = h[g_len-i-1];

    // create synthesis/analysis filter objects
    firfilt_crcf fs = firfilt_crcf_create(h, h_len);
    firfilt_crcf fa = firfilt_crcf_create(g, g_len);

    // create synthesis/analysis filterbank channelizer objects
    firpfbch_crcf qs = firpfbch_crcf_create(LIQUID_SYNTHESIZER, num_channels, p, h);
    firpfbch_crcf qa = firpfbch_crcf_create(LIQUID_ANALYZER,    num_channels, p, g);

    float complex x[num_samples];                   // random input (noise)
    float complex Y0[num_symbols][num_channels];    // channelized output (filterbank)
    float complex Y1[num_symbols][num_channels];    // channelized output
    float complex z0[num_samples];                  // time-domain output (filterbank)
    float complex z1[num_samples];                  // time-domain output

    // generate input sequence (complex noise)
    for (i=0; i<num_samples; i++)
        x[i] = randnf() * cexpf(_Complex_I*randf()*2*M_PI);

    // 
    // ANALYZERS
    //
    
    // 
    // run analysis filter bank
    //
    for (i=0; i<num_symbols; i++)
        firpfbch_crcf_analyzer_execute(qa, &x[i*num_channels], &Y0[i][0]);


    // 
    // run traditional down-converter (inefficient)
    //
    float dphi; // carrier frequency
    unsigned int n=0;
    for (i=0; i<num_channels; i++) {

        // reset filter
        firfilt_crcf_reset(fa);

        // set center frequency
        dphi = 2.0f * M_PI * (float)i / (float)num_channels;

        // reset symbol counter
        n=0;

        for (j=0; j<num_samples; j++) {
            // push down-converted sample into filter
            firfilt_crcf_push(fa, x[j]*cexpf(-_Complex_I*j*dphi));

            // compute output at the appropriate sample time
            assert(n<num_symbols);
            if ( ((j+1)%num_channels)==0 ) {
                firfilt_crcf_execute(fa, &Y1[n][i]);
                n++;
            }
        }
        assert(n==num_symbols);

    }


    // 
    // SYNTHESIZERS
    //

    // 
    // run synthesis filter bank
    //
    for (i=0; i<num_symbols; i++)
        firpfbch_crcf_synthesizer_execute(qs, &Y0[i][0], &z0[i*num_channels]);

    // 
    // run traditional up-converter (inefficient)
    //

    // clear output array
    for (i=0; i<num_samples; i++)
        z1[i] = 0.0f;

    float complex y_hat;
    for (i=0; i<num_channels; i++) {
        // reset filter
        firfilt_crcf_reset(fs);

        // set center frequency
        dphi = 2.0f * M_PI * (float)i / (float)num_channels;

        // reset input symbol counter
        n=0;

        for (j=0; j<num_samples; j++) {

            // interpolate sequence
            if ( (j%num_channels)==0 ) {
                assert(n<num_symbols);
                firfilt_crcf_push(fs, Y1[n][i]);
                n++;
            } else {
                firfilt_crcf_push(fs, 0);
            }
            firfilt_crcf_execute(fs, &y_hat);

            // accumulate up-converted sample
            z1[j] += y_hat * cexpf(_Complex_I*j*dphi);
        }
        assert(n==num_symbols);
    }

    // destroy objects
    firfilt_crcf_destroy(fs);
    firfilt_crcf_destroy(fa);
    firpfbch_crcf_destroy(qs);
    firpfbch_crcf_destroy(qa);


    //
    // RESULTS
    //

    // 
    // analyzers
    //

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


    float mse_analyzer[num_channels];
    float complex d;
    for (i=0; i<num_channels; i++) {
        mse_analyzer[i] = 0.0f;
        for (j=0; j<num_symbols; j++) {
            d = Y0[j][i] - Y1[j][i];
            mse_analyzer[i] += crealf(d*conjf(d));
        }

        mse_analyzer[i] /= num_symbols;
    }
    printf("\n");
    printf("rmse: ");
    for (i=0; i<num_channels; i++)
        printf("%12.4e          ", sqrt(mse_analyzer[i]));
    printf("\n");


    // 
    // synthesizers
    //

    printf("\n");
    printf("output: filterbank:             traditional:\n");
    for (i=0; i<num_samples; i++) {
        printf("%3u: %10.5f+%10.5fj  %10.5f+%10.5fj\n",
            i,
            crealf(z0[i]), cimagf(z0[i]),
            crealf(z1[i]), cimagf(z1[i]));
    }

    float mse_synthesizer = 0.0f;
    for (i=0; i<num_samples; i++) {
        d = z0[i] - z1[i];
        mse_synthesizer += crealf(d*conjf(d));
    }
    mse_synthesizer /= num_samples;
    printf("\n");
    printf("rmse: %12.4e\n", sqrtf(mse_synthesizer));

    //
    // EXPORT DATA TO FILE
    //

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"num_channels=%u;\n", num_channels);
    fprintf(fid,"num_symbols=%u;\n", num_symbols);
    fprintf(fid,"num_samples = num_channels*num_symbols;\n");

    fprintf(fid,"x  = zeros(1,num_samples);\n");
    fprintf(fid,"y0 = zeros(num_symbols,num_channels);\n");
    fprintf(fid,"y1 = zeros(num_symbols,num_channels);\n");
    fprintf(fid,"z0 = zeros(1,num_samples);\n");
    fprintf(fid,"z1 = zeros(1,num_samples);\n");

    // input
    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimag(x[i]));

    // analysis
    for (i=0; i<num_symbols; i++) {
        for (j=0; j<num_channels; j++) {
            fprintf(fid,"y0(%4u,%4u) = %12.4e + j*%12.4e;\n", i+1, j+1, crealf(Y0[i][j]), cimag(Y0[i][j]));
            fprintf(fid,"y1(%4u,%4u) = %12.4e + j*%12.4e;\n", i+1, j+1, crealf(Y1[i][j]), cimag(Y1[i][j]));
        }
    }

    // synthesis
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"z0(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(z0[i]), cimag(z0[i]));
        fprintf(fid,"z1(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(z1[i]), cimag(z1[i]));
    }
    fprintf(fid,"z0 = z0 / num_channels;\n");
    fprintf(fid,"z1 = z1 / num_channels;\n");

    // plot results
    fprintf(fid,"\n\n");
    fprintf(fid,"ts = 0:(num_symbols-1);\n");
    fprintf(fid,"for i=1:num_channels,\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"title(['channel ' num2str(i)]);\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"    plot(ts,real(y0(:,i)),'-x', ts,real(y1(:,i)),'s');\n");
    //fprintf(fid,"    axis([0 (num_symbols-1) -2 2]);\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"    plot(ts,imag(y0(:,i)),'-x', ts,imag(y1(:,i)),'s');\n");
    //fprintf(fid,"    axis([0 (num_symbols-1) -2 2]);\n");
    fprintf(fid,"end;\n");

    fprintf(fid,"t  = 0:(num_samples-1);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"title('composite');\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"    plot(t,real(z0),'-x', t,real(z1),'s');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"    plot(t,imag(z0),'-x', t,imag(z1),'s');\n");


    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
