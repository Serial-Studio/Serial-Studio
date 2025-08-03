/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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

//
// fftfilt_xxxf_autotest.c : test floating-point filters
//

#include "autotest/autotest.h"
#include "liquid.h"

// autotest data definitions
#include "src/filter/tests/fftfilt_autotest.h"

// 
// AUTOTEST: fftfilt_rrrf tests
//
void autotest_fftfilt_rrrf_data_h4x256()
{
    fftfilt_rrrf_test(fftfilt_rrrf_data_h4x256_h, 4,
                      fftfilt_rrrf_data_h4x256_x, 256,
                      fftfilt_rrrf_data_h4x256_y, 256);
}
void autotest_fftfilt_rrrf_data_h7x256()
{
    fftfilt_rrrf_test(fftfilt_rrrf_data_h7x256_h, 7,
                      fftfilt_rrrf_data_h7x256_x, 256,
                      fftfilt_rrrf_data_h7x256_y, 256);
}
void autotest_fftfilt_rrrf_data_h13x256()
{
    fftfilt_rrrf_test(fftfilt_rrrf_data_h13x256_h, 13,
                      fftfilt_rrrf_data_h13x256_x, 256,
                      fftfilt_rrrf_data_h13x256_y, 256);
}
void autotest_fftfilt_rrrf_data_h23x256()
{
    fftfilt_rrrf_test(fftfilt_rrrf_data_h23x256_h, 23,
                      fftfilt_rrrf_data_h23x256_x, 256,
                      fftfilt_rrrf_data_h23x256_y, 256);
}


// 
// AUTOTEST: fftfilt_crcf tests
//
void autotest_fftfilt_crcf_data_h4x256()
{
    fftfilt_crcf_test(fftfilt_crcf_data_h4x256_h, 4,
                      fftfilt_crcf_data_h4x256_x, 256,
                      fftfilt_crcf_data_h4x256_y, 256);
}
void autotest_fftfilt_crcf_data_h7x256()
{
    fftfilt_crcf_test(fftfilt_crcf_data_h7x256_h, 7,
                      fftfilt_crcf_data_h7x256_x, 256,
                      fftfilt_crcf_data_h7x256_y, 256);
}
void autotest_fftfilt_crcf_data_h13x256()
{
    fftfilt_crcf_test(fftfilt_crcf_data_h13x256_h, 13,
                      fftfilt_crcf_data_h13x256_x, 256,
                      fftfilt_crcf_data_h13x256_y, 256);
}
void autotest_fftfilt_crcf_data_h23x256()
{
    fftfilt_crcf_test(fftfilt_crcf_data_h23x256_h, 23,
                      fftfilt_crcf_data_h23x256_x, 256,
                      fftfilt_crcf_data_h23x256_y, 256);
}


// 
// AUTOTEST: fftfilt_cccf tests
//
void autotest_fftfilt_cccf_data_h4x256()
{
    fftfilt_cccf_test(fftfilt_cccf_data_h4x256_h, 4,
                      fftfilt_cccf_data_h4x256_x, 256,
                      fftfilt_cccf_data_h4x256_y, 256);
}
void autotest_fftfilt_cccf_data_h7x256()
{
    fftfilt_cccf_test(fftfilt_cccf_data_h7x256_h, 7,
                      fftfilt_cccf_data_h7x256_x, 256,
                      fftfilt_cccf_data_h7x256_y, 256);
}
void autotest_fftfilt_cccf_data_h13x256()
{
    fftfilt_cccf_test(fftfilt_cccf_data_h13x256_h, 13,
                      fftfilt_cccf_data_h13x256_x, 256,
                      fftfilt_cccf_data_h13x256_y, 256);
}
void autotest_fftfilt_cccf_data_h23x256()
{
    fftfilt_cccf_test(fftfilt_cccf_data_h23x256_h, 23,
                      fftfilt_cccf_data_h23x256_x, 256,
                      fftfilt_cccf_data_h23x256_y, 256);
}

void autotest_fftfilt_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping fftfilt config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check that object returns NULL for invalid configurations
    float h[9] = {0,1,2,3,4,5,6,7,8,};
    CONTEND_ISNULL(fftfilt_crcf_create(h,0,64)); // filter length too small
    CONTEND_ISNULL(fftfilt_crcf_create(h,9, 7)); // block length too small

    // create proper object and test configurations
    fftfilt_crcf filt = fftfilt_crcf_create(h, 9, 64);

    CONTEND_EQUALITY(LIQUID_OK, fftfilt_crcf_print(filt));
    CONTEND_EQUALITY(LIQUID_OK, fftfilt_crcf_set_scale(filt, 3.0f));
    float scale = 0.0f;
    CONTEND_EQUALITY(LIQUID_OK, fftfilt_crcf_get_scale(filt, &scale));
    CONTEND_EQUALITY(scale, 3.0f);
    CONTEND_EQUALITY(9, fftfilt_crcf_get_length(filt));

    fftfilt_crcf_destroy(filt);
}

void autotest_fftfilt_copy()
{
    // generate random filter coefficients
    unsigned int i, j, h_len = 31;
    float h[h_len];
    for (i=0; i<h_len; i++)
        h[i] = randnf();

    // determine appropriate block size
    // NOTE: this number can be anything at least _h_len-1
    unsigned int n = 96;

    // create object
    fftfilt_crcf q0 = fftfilt_crcf_create(h, h_len, n);

    // compute output in blocks of size 'n'
    float complex buf[n], buf_0[n], buf_1[n];
    for (i=0; i<10; i++) {
        for (j=0; j<n; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        fftfilt_crcf_execute(q0, buf, buf_0);
    }

    // copy object
    fftfilt_crcf q1 = fftfilt_crcf_copy(q0);

    // run filters in parallel and compare results
    for (i=0; i<10; i++) {
        for (j=0; j<n; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        fftfilt_crcf_execute(q0, buf, buf_0);
        fftfilt_crcf_execute(q1, buf, buf_1);

        CONTEND_SAME_DATA( buf_0, buf_1, n*sizeof(float complex));
    }
    
    // destroy objects
    fftfilt_crcf_destroy(q0);
    fftfilt_crcf_destroy(q1);
}

