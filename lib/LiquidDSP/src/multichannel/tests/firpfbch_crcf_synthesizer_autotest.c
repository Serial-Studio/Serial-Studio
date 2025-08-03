/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// AUTOTEST: validate synthesis correctness
//
void autotest_firpfbch_crcf_synthesis()
{
    // options
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

    // create filter object
    firfilt_crcf f = firfilt_crcf_create(h, h_len);

    // create filterbank channelizer object
    firpfbch_crcf q = firpfbch_crcf_create(LIQUID_SYNTHESIZER, num_channels, p, h);

    float complex Y[num_symbols][num_channels];     // channelized input
    float complex y0[num_samples];                  // time-domain output
    float complex y1[num_samples];                  // time-domain output

    // generate input sequence (complex noise)
    ms = msequence_create_default(7);
    for (i=0; i<num_symbols; i++) {
        for (j=0; j<num_channels; j++) {
            Y[i][j] = 0.1f * M_SQRT1_2 * ((float)msequence_generate_symbol(ms,2) - 1.5f) +
                      0.1f * M_SQRT1_2 * ((float)msequence_generate_symbol(ms,2) - 1.5f)*_Complex_I;
        }
    }
    msequence_destroy(ms);

    // 
    // run synthesis filter bank
    //

    for (i=0; i<num_symbols; i++)
        firpfbch_crcf_synthesizer_execute(q, &Y[i][0], &y0[i*num_channels]);

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
    firfilt_crcf_destroy(f);
    firpfbch_crcf_destroy(q);

    // 
    // compare results
    // 
    for (i=0; i<num_samples; i++) {

        // print channelizer outputs
        if (liquid_autotest_verbose) {
            printf("%3u: old:%8.5f+j%8.5f, firpfbch:%8.5f+j%8.5f, err:%12.4e+j%12.4e\n",
                i,
                crealf(y0[i]),       cimagf(y0[i]),
                crealf(y1[i]),       cimagf(y1[i]),
                crealf(y1[i]-y0[i]), cimagf(y1[i]-y0[i]));
        }


        CONTEND_DELTA( crealf(y0[i]), crealf(y1[i]), tol );
        CONTEND_DELTA( cimagf(y0[i]), cimagf(y1[i]), tol );
    }
}

