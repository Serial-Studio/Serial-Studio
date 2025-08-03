/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <assert.h>
#include "autotest/autotest.h"
#include "liquid.h"

//
// AUTOTEST: validate analysis correctness
//
void autotest_firpfbch_crcf_analysis()
{
    float tol = 1e-4f;              // error tolerance
    unsigned int num_channels=4;    // number of channels
    unsigned int p=5;               // filter length (symbols)
    unsigned int num_symbols=40;    // number of symbols

    // derived values
    unsigned int num_samples = num_channels * num_symbols;

    unsigned int i;
    unsigned int j;

    // generate filter
    // NOTE : these coefficients can be random; the purpose of this
    //        exercise is to demonstrate mathematical equivalence.
    //        For the sake of consistency, use pseudo-random values
    //        chosen from m-sequences
    unsigned int h_len = p*num_channels;
    float h[h_len];
    msequence ms = msequence_create_default(6);
    for (i=0; i<h_len; i++)
        h[i] = (float)msequence_generate_symbol(ms, 2) - 1.5f; // (-1.5, -0.5, 0.5, 1.5)
    msequence_destroy(ms);

    // create filterbank object
    firpfbch_crcf q = firpfbch_crcf_create(LIQUID_ANALYZER, num_channels, p, h);

    // generate filter object
    firfilt_crcf f = firfilt_crcf_create(h, h_len);

    // allocate memory for arrays
    float complex y[num_samples];                   // time-domain input
    float complex Y0[num_symbols][num_channels];    // channelized output
    float complex Y1[num_symbols][num_channels];    // channelized output

    // generate input sequence (complex noise)
    ms = msequence_create_default(7);
    for (i=0; i<num_samples; i++) {
        y[i] = 0.1f * M_SQRT1_2 * ((float)msequence_generate_symbol(ms,2) - 1.5f) +
               0.1f * M_SQRT1_2 * ((float)msequence_generate_symbol(ms,2) - 1.5f)*_Complex_I;
    }
    msequence_destroy(ms);

    // 
    // run analysis filter bank
    //

    for (i=0; i<num_symbols; i++)
        firpfbch_crcf_analyzer_execute(q, &y[i*num_channels], &Y0[i][0]);


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
    firfilt_crcf_destroy(f);
    firpfbch_crcf_destroy(q);

    if (liquid_autotest_verbose) {
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
    }

    // compare results
    for (i=0; i<num_symbols; i++) {
        for (j=0; j<num_channels; j++) {
            CONTEND_DELTA( crealf(Y0[i][j]), crealf(Y1[i][j]), tol );
            CONTEND_DELTA( cimagf(Y0[i][j]), cimagf(Y1[i][j]), tol );
        }
    }

}


