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

void autotest_iirfiltsos_impulse_n2()
{
    // initialize filter with 2nd-order low-pass butterworth filter
    float a[3] = {
        1.000000000000000,
       -0.942809041582063,
        0.333333333333333};

    float b[3] = {
        0.0976310729378175,
        0.1952621458756350,
        0.0976310729378175};

    // create identical objects
    iirfiltsos_rrrf q0 = iirfiltsos_rrrf_create(b,a);
    iirfiltsos_rrrf q1 = iirfiltsos_rrrf_create(b,a);

    // initialize oracle; expected output (generated with octave)
    float test[15] = {
       9.76310729378175e-02,
       2.87309604180767e-01,
       3.35965474513536e-01,
       2.20981418970514e-01,
       9.63547883225231e-02,
       1.71836926400291e-02,
      -1.59173219853878e-02,
      -2.07348926322729e-02,
      -1.42432702548109e-02,
      -6.51705310050832e-03,
      -1.39657983602602e-03,
       8.55642936806248e-04,
       1.27223450919543e-03,
       9.14259886013424e-04,
       4.37894317157432e-04};

    unsigned int i;
    float v, y;
    float tol=1e-4f;

    // hit filter with impulse, compare output
    for (i=0; i<15; i++) {
        // generate input
        v = (i==0) ? 1.0f : 0.0f;

        // run direct-form I
        iirfiltsos_rrrf_execute_df1(q0, v, &y);
        CONTEND_DELTA(test[i], y, tol);

        // run direct-form II
        iirfiltsos_rrrf_execute_df2(q1, v, &y);
        CONTEND_DELTA(test[i], y, tol);
    }

    iirfiltsos_rrrf_destroy(q0);
    iirfiltsos_rrrf_destroy(q1);
}


void autotest_iirfiltsos_step_n2()
{
    // initialize filter with 2nd-order low-pass butterworth filter
    float a[3] = {
        1.000000000000000,
       -0.942809041582063,
        0.333333333333333};

    float b[3] = {
        0.0976310729378175,
        0.1952621458756350,
        0.0976310729378175};

    // create identical objects
    iirfiltsos_rrrf q0 = iirfiltsos_rrrf_create(b,a);
    iirfiltsos_rrrf q1 = iirfiltsos_rrrf_create(b,a);

    float test[15] = {
       0.0976310729378175,
       0.3849406771185847,
       0.7209061516321208,
       0.9418875706026352,
       1.0382423589251584,
       1.0554260515651877,
       1.0395087295798000,
       1.0187738369475272,
       1.0045305666927162,
       0.9980135135922078,
       0.9966169337561817,
       0.9974725766929878,
       0.9987448112021832,
       0.9996590710881966,
       1.0000969654053542};

    unsigned int i;
    float y;
    float tol=1e-4f;

    // hit filter with step, compare output
    for (i=0; i<15; i++) {
        // run direct-form I
        iirfiltsos_rrrf_execute_df1(q0, 1, &y);
        CONTEND_DELTA(test[i], y, tol);

        // run direct-form II
        iirfiltsos_rrrf_execute_df2(q1, 1, &y);
        CONTEND_DELTA(test[i], y, tol);
    }

    iirfiltsos_rrrf_destroy(q0);
    iirfiltsos_rrrf_destroy(q1);
}

void autotest_iirfiltsos_copy()
{
    // initialize filter with 2nd-order low-pass butterworth filter
    float a[3] = {1.0000000000000000f, -0.942809041582063f, 0.3333333333333333f};
    float b[3] = {0.0976310729378175f,  0.195262145875635f, 0.0976310729378175f};

    // create base object
    iirfiltsos_crcf q0 = iirfiltsos_crcf_create(b,a);

    // start running input through filter
    unsigned int i, num_samples = 80;
    float complex y0, y1;
    for (i=0; i<num_samples; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        iirfiltsos_crcf_execute(q0, v, &y0);
    }

    // copy filter
    iirfiltsos_crcf q1 = iirfiltsos_crcf_copy(q0);

    // continue running through both filters
    for (i=0; i<num_samples; i++) {
        float complex v = randnf() + _Complex_I*randnf();
        iirfiltsos_crcf_execute(q0, v, &y0);
        iirfiltsos_crcf_execute(q1, v, &y1);

        // compare result
        CONTEND_EQUALITY(y0, y1);
    }

    // destroy filter objects
    iirfiltsos_crcf_destroy(q0);
    iirfiltsos_crcf_destroy(q1);
}

// test errors and invalid configuration
void autotest_iirfiltsos_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping iirfilt config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // test copying/creating invalid objects
    CONTEND_ISNULL( iirfiltsos_crcf_copy(NULL) );

    // create valid object and test configuration
    //iirfiltsos_crcf filter = iirfiltsos_crcf_create(...);
    //iirfiltsos_crcf_destroy(filter);
}

