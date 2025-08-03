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

#include <math.h>
#include "autotest/autotest.h"
#include "liquid.h"

// Help function to keep code base small
void firfilt_cccf_notch_test_harness(unsigned int _m,
                                     float        _as,
                                     float        _f0)
{
    unsigned int num_samples = 600;     // number of samples
    unsigned int h_len       = 2*_m+1;  // filter length

    // design filter from prototype
    firfilt_cccf q = firfilt_cccf_create_notch(_m,_as,_f0);

    // generate input signal
    unsigned int i;
    float x2 = 0.0f;
    float y2 = 0.0f;
    for (i=0; i<num_samples+h_len; i++) {
        // compute input: tone at f0
        float complex x = cexpf(_Complex_I*2*M_PI*_f0*i);

        // filter input
        float complex y;
        firfilt_cccf_push   (q,  x);
        firfilt_cccf_execute(q, &y);

        // accumulate, compensating for filter delay
        if (i >= h_len) {
            x2 += cabsf(x)*cabsf(x);
            y2 += cabsf(y)*cabsf(y);
        }
    }

    // compare result
    x2 = sqrtf(x2 / (float)num_samples);
    y2 = sqrtf(y2 / (float)num_samples);
    if (liquid_autotest_verbose) {
        firfilt_cccf_print(q);
        printf("f0 = %.8f, x2: %f, y2: %f\n", _f0, x2, y2);
    }
    float tol = 1e-3f;
    CONTEND_DELTA(x2, 1.0f, tol);
    CONTEND_DELTA(y2, 0.0f, tol);

    // destroy filter object
    firfilt_cccf_destroy(q);
}

// AUTOTESTS: 
void autotest_firfilt_cccf_notch_0() { firfilt_cccf_notch_test_harness(20,60.0f, 0.000f); }
void autotest_firfilt_cccf_notch_1() { firfilt_cccf_notch_test_harness(20,60.0f, 0.100f); }
void autotest_firfilt_cccf_notch_2() { firfilt_cccf_notch_test_harness(20,60.0f, 0.456f); }
void autotest_firfilt_cccf_notch_3() { firfilt_cccf_notch_test_harness(20,60.0f, 0.500f); }
void autotest_firfilt_cccf_notch_4() { firfilt_cccf_notch_test_harness(20,60.0f,-0.250f); }
void autotest_firfilt_cccf_notch_5() { firfilt_cccf_notch_test_harness(20,60.0f,-0.389f); }

