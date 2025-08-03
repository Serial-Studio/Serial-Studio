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

// Helper function to keep code base small
void firpfbch2_crcf_runtest(unsigned int _M,
                            unsigned int _m,
                            float        _as)
{
    float tol = 1e-3f;
    unsigned int i;

    // derived values
    unsigned int num_symbols = 8*_m;    // number of symbols
    unsigned int num_samples = _M * num_symbols;

    // allocate arrays
    float complex x[num_samples];
    float complex y[num_samples];

    // generate pseudo-random sequence
    unsigned int s = 1;         // seed
    unsigned int p = 524287;    // large prime number
    unsigned int g =   1031;    // another large prime number
    for (i=0; i<num_samples; i++) {
        s = (s * p) % g;
        x[i] = (float)s / (float)g - 0.5f;
        //x[i] = (i==0) ? 1.0f : 0.0f;  // impulse response
    }

    // create filterbank objects from prototype
    firpfbch2_crcf qa = firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER,    _M, _m, _as);
    firpfbch2_crcf qs = firpfbch2_crcf_create_kaiser(LIQUID_SYNTHESIZER, _M, _m, _as);

    // run channelizer
    float complex Y[_M];
    for (i=0; i<num_samples; i+=_M/2) {
        // run analysis filterbank
        firpfbch2_crcf_execute(qa, &x[i], Y);

        // run synthesis filterbank
        firpfbch2_crcf_execute(qs, Y, &y[i]);
    }

    // destroy filterbank objects
    firpfbch2_crcf_destroy(qa); // analysis filterbank
    firpfbch2_crcf_destroy(qs); // synthesis filterbank

    // validate output
    unsigned int delay = 2*_M*_m - _M/2 + 1;
    float rmse = 0.0f;
    for (i=0; i<num_samples; i++) {
        //printf("%3u : %12.8f + %12.8fj\n", i, crealf(y[i]), cimagf(y[i]));
        if (i < delay) {
            CONTEND_DELTA( crealf(y[i]), 0.0f, tol );
            CONTEND_DELTA( cimagf(y[i]), 0.0f, tol );
        } else {
            CONTEND_DELTA( crealf(y[i]), crealf(x[i-delay]), tol );
            CONTEND_DELTA( cimagf(y[i]), cimagf(x[i-delay]), tol );
        }

        // compute rmse
        float complex err = y[i] - (i < delay ? 0.0f : x[i-delay]);
        rmse += crealf(err * conjf(err));
    }

    rmse = sqrtf(rmse / (float)num_samples);
    if (liquid_autotest_verbose)
        printf("firpfbch2:  M=%3u, m=%2u, as=%8.2f dB, rmse=%12.4e\n", _M, _m, _as, rmse);
}

// analysis
void autotest_firpfbch2_crcf_n8()    { firpfbch2_crcf_runtest(   8, 5, 60.0f); }
void autotest_firpfbch2_crcf_n16()   { firpfbch2_crcf_runtest(  16, 5, 60.0f); }
void autotest_firpfbch2_crcf_n32()   { firpfbch2_crcf_runtest(  32, 5, 60.0f); }
void autotest_firpfbch2_crcf_n64()   { firpfbch2_crcf_runtest(  64, 5, 60.0f); }

void autotest_firpfbch2_crcf_copy()
{
    // create channelizer
    unsigned int M  = 72;
    unsigned int m  = 12;
    float        as = 80.0f;
    firpfbch2_crcf q_orig = firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER, M, m, as);

    float complex buf_0     [M/2];
    float complex buf_1_orig[M  ];
    float complex buf_1_copy[M  ];

    // start running input through filter
    unsigned int num_blocks = 32;
    unsigned int i, j;
    for (i=0; i<num_blocks; i++) {
        for (j=0; j<M/2; j++)
            buf_0[j] = randnf() + _Complex_I*randnf();

        firpfbch2_crcf_execute(q_orig, buf_0, buf_1_orig);
    }

    // copy object
    firpfbch2_crcf q_copy = firpfbch2_crcf_copy(q_orig);

    // continue running through both objects
    for (i=0; i<num_blocks; i++) {
        for (j=0; j<M/2; j++)
            buf_0[j] = randnf() + _Complex_I*randnf();

        // run filters in parallel
        firpfbch2_crcf_execute(q_orig, buf_0, buf_1_orig);
        firpfbch2_crcf_execute(q_copy, buf_0, buf_1_copy);

        CONTEND_SAME_DATA(buf_1_orig, buf_1_copy, M*sizeof(float complex));
    }

    // destroy filter objects
    firpfbch2_crcf_destroy(q_orig);
    firpfbch2_crcf_destroy(q_copy);
}

void autotest_firpfbch2_crcf_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firpfbch2_crcf config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check invalid function calls
    CONTEND_ISNULL(firpfbch2_crcf_create(             77, 76, 12, NULL)) // invalid type
    CONTEND_ISNULL(firpfbch2_crcf_create(LIQUID_ANALYZER,  0, 12, NULL)) // invalid number of channels
    CONTEND_ISNULL(firpfbch2_crcf_create(LIQUID_ANALYZER, 17, 12, NULL)) // invalid number of channels
    CONTEND_ISNULL(firpfbch2_crcf_create(LIQUID_ANALYZER, 76,  0, NULL)) // invalid filter semi-length

    CONTEND_ISNULL(firpfbch2_crcf_create_kaiser(             77, 76, 12, 60.0f)) // invalid type
    CONTEND_ISNULL(firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER,  0, 12, 60.0f)) // invalid number of channels
    CONTEND_ISNULL(firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER, 17, 12, 60.0f)) // invalid number of channels
    CONTEND_ISNULL(firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER, 76,  0, 60.0f)) // invalid filter semi-length

    CONTEND_ISNULL(firpfbch2_crcf_copy(NULL))

    // create proper object and test configurations
    firpfbch2_crcf q = firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER, 76, 12, 60.0f);

    CONTEND_EQUALITY(LIQUID_OK,       firpfbch2_crcf_print(q))
    CONTEND_EQUALITY(LIQUID_ANALYZER, firpfbch2_crcf_get_type(q))
    CONTEND_EQUALITY(             76, firpfbch2_crcf_get_M(q))
    CONTEND_EQUALITY(             12, firpfbch2_crcf_get_m(q))

    firpfbch2_crcf_destroy(q);
}

