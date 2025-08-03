/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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

#include "autotest/autotest.h"
#include "liquid.internal.h"

void autotest_firdecim_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firdecim config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // design filter
    unsigned int M     =  4;
    unsigned int m     = 12;
    unsigned int h_len =  2*M*m+1;
    float        h[h_len];
    int          wtype = LIQUID_WINDOW_HAMMING;
    liquid_firdes_windowf(wtype, h_len, 0.2f, 0, h);

    // check that estimate methods return zero for invalid configs
    CONTEND_ISNULL(firdecim_crcf_create(0, h, h_len)); // M cannot be 0
    CONTEND_ISNULL(firdecim_crcf_create(M, h,     0)); // h_len cannot be 0

    CONTEND_ISNULL(firdecim_crcf_create_kaiser(1, 12, 60.0f)); // M too small
    CONTEND_ISNULL(firdecim_crcf_create_kaiser(4,  0, 60.0f)); // m too small
    CONTEND_ISNULL(firdecim_crcf_create_kaiser(4, 12, -2.0f)); // As too small

    CONTEND_ISNULL(firdecim_crcf_create_prototype(LIQUID_FIRFILT_UNKNOWN, 4, 12, 0.3f, 0.0f));
    CONTEND_ISNULL(firdecim_crcf_create_prototype(LIQUID_FIRFILT_RCOS,    1, 12, 0.3f, 0.0f));
    CONTEND_ISNULL(firdecim_crcf_create_prototype(LIQUID_FIRFILT_RCOS,    4,  0, 0.3f, 0.0f));
    CONTEND_ISNULL(firdecim_crcf_create_prototype(LIQUID_FIRFILT_RCOS,    4, 12, 7.2f, 0.0f));
    CONTEND_ISNULL(firdecim_crcf_create_prototype(LIQUID_FIRFILT_RCOS,    4, 12, 0.3f, 4.0f));

    // create valid object and test configuration
    firdecim_crcf decim = firdecim_crcf_create_kaiser(M, m, 60.0f);
    CONTEND_EQUALITY(firdecim_crcf_print(decim), LIQUID_OK);
    CONTEND_EQUALITY(firdecim_crcf_set_scale(decim, 8.0f), LIQUID_OK);
    float scale = 1.0f;
    CONTEND_EQUALITY(firdecim_crcf_get_scale(decim, &scale), LIQUID_OK);
    CONTEND_EQUALITY(scale, 8.0f);

    firdecim_crcf_destroy(decim);
}

// assert that block execution matches regular execute
void autotest_firdecim_block()
{
    unsigned int M =  4;
    unsigned int m = 12;
    float        beta = 0.3f;

    unsigned int num_blocks = 10 + m;
    float complex buf_0[M*num_blocks]; // input
    float complex buf_1[  num_blocks]; // output (regular)
    float complex buf_2[  num_blocks]; // output (block)

    firdecim_crcf decim = firdecim_crcf_create_prototype(
            LIQUID_FIRFILT_ARKAISER, M, m, beta, 0);

    // create random-ish input (does not really matter what the input is
    // so long as the outputs match, but systematic for repeatability)
    unsigned int i;
    for (i=0; i<M*num_blocks; i++)
        buf_0[i] = cexpf(_Complex_I*(0.2f*i + 1e-5f*i*i + 0.1*cosf(i)));

    // regular execute
    firdecim_crcf_reset(decim);
    for (i=0; i<num_blocks; i++)
        firdecim_crcf_execute(decim, buf_0+i*M, buf_1+i);

    // block execute
    firdecim_crcf_reset(decim);
    firdecim_crcf_execute_block(decim, buf_0, num_blocks, buf_2);

    // check results
    CONTEND_SAME_DATA(buf_1, buf_2, num_blocks);

    firdecim_crcf_destroy(decim);
}

// test copy method
void autotest_firdecim_copy()
{
    unsigned int M    =    4;
    unsigned int m    =   12;
    float        beta = 0.3f;

    // create base object
    firdecim_crcf q0 = firdecim_crcf_create_prototype(
            LIQUID_FIRFILT_ARKAISER, M, m, beta, 0);
    firdecim_crcf_set_scale(q0, 0.12345f);

    unsigned int num_blocks = 10 + m;
    float complex buf  [M*num_blocks]; // input
    float complex buf_0[  num_blocks]; // output (base)
    float complex buf_1[  num_blocks]; // output (copy)

    // create random-ish input (does not really matter what the input is
    // so long as the outputs match, but systematic for repeatability)
    unsigned int i;
    for (i=0; i<M*num_blocks; i++)
        buf[i] = cexpf(_Complex_I*(0.2f*i + 1e-5f*i*i + 0.1*cosf(i)));
    firdecim_crcf_execute_block(q0, buf, num_blocks, buf_0);

    // copy object and test basic properties
    firdecim_crcf q1 = firdecim_crcf_copy(q0);
    float scale_0 = 1.0f;
    float scale_1 = 0.0f;
    CONTEND_EQUALITY(firdecim_crcf_get_scale(q0, &scale_0), LIQUID_OK);
    CONTEND_EQUALITY(firdecim_crcf_get_scale(q1, &scale_1), LIQUID_OK);
    CONTEND_EQUALITY(scale_0, scale_1);

    // generate new buffer of input samples
    for (i=0; i<M*num_blocks; i++)
        buf[i] = cexpf(_Complex_I*(-0.2f*i + 2e-5f*i*i + 0.13*cosf(i*0.7f)));

    // run samples through both objects in parallel
    firdecim_crcf_execute_block(q0, buf, num_blocks, buf_0);
    firdecim_crcf_execute_block(q1, buf, num_blocks, buf_1);

    // check results
    CONTEND_SAME_DATA(buf_0, buf_1, num_blocks);

    // destroy objects
    firdecim_crcf_destroy(q0);
    firdecim_crcf_destroy(q1);
}

