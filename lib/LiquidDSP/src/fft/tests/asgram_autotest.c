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

// test ASCII spectral periodogram (asgram) objects

#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

// check copy method
void autotest_asgramcf_copy()
{
    // options
    unsigned int nfft = 70; // transform size
    unsigned int num_samples = 844;
    float nstd = 0.1f;

    // create object
    asgramcf q0 = asgramcf_create(nfft);

    // set parameters
    asgramcf_set_scale  (q0, -20.8f, 3.1f);
    asgramcf_set_display(q0, "abcdeFGHIJ");

    // generate a bunch of random noise samples
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        float complex v = 0.1f + nstd * (randnf() + _Complex_I*randnf());
        asgramcf_push(q0, v);
    }

    // copy object and push same samples through both
    asgramcf q1 = asgramcf_copy(q0);
    for (i=0; i<num_samples; i++) {
        float complex v = 0.1f + nstd * (randnf() + _Complex_I*randnf());
        asgramcf_push(q0, v);
        asgramcf_push(q1, v);
    }
    if (liquid_autotest_verbose) {
        printf("q0:"); asgramcf_print(q0);
        printf("q1:"); asgramcf_print(q1);
    }

    // get spectrum and compare outputs
    char a0[nfft], a1[nfft];
    float pv0, pv1, pf0, pf1;
    asgramcf_execute(q0, a0, &pv0, &pf0);
    asgramcf_execute(q1, a1, &pv1, &pf1);
    CONTEND_SAME_DATA(a0, a1, nfft*sizeof(char));
    CONTEND_EQUALITY (pv0, pv1);
    CONTEND_EQUALITY (pf0, pf1);

    // destroy objects
    asgramcf_destroy(q0);
    asgramcf_destroy(q1);
}

