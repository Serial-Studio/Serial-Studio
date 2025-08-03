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

#include <stdio.h>
#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.h"

// test that the complete internal state of one detector can be copied to a new
// object, and that both return identical results when provided the same input
void autotest_qdetector_cccf_copy()
{
    // generate random-ish sequence
    unsigned int  sequence_len = 64;
    float complex sequence[sequence_len];
    unsigned int i;
    for (i=0; i<sequence_len; i++) {
        float r = (float)i - 0.5f*(float)sequence_len;
        sequence[i] = cexpf(_Complex_I*0.02*r*r);
    }

    // create initial detector
    qdetector_cccf q0 = qdetector_cccf_create(sequence, sequence_len);

    // run on random-ish samples
    for (i=0; i<347; i++)
        qdetector_cccf_execute(q0, cexpf(_Complex_I*i));

    // create new object (copied)
    qdetector_cccf q1 = qdetector_cccf_copy(q0);

    // try to detect frame
    float complex * v0 = NULL;
    float complex * v1 = NULL;
    unsigned int frames_detected = 0;
    for (i=0; i<sequence_len + 80; i++) {
        // pull sample and run through both detectors
        float complex s = i < sequence_len ? sequence[i] : cexpf(_Complex_I*i);
        v0 = qdetector_cccf_execute(q0,s);
        v1 = qdetector_cccf_execute(q1,s);

        // debugging
        if (v0 != NULL) printf(" [%3u] frame detected on detector 0\n", i);
        if (v1 != NULL) printf(" [%3u] frame detected on detector 1\n", i);

        // check results
        if (v0 != NULL && v1 != NULL) {
            printf(" [%3u] frame detected on both detectors\n", i);
            frames_detected++;

            // get statistics
            CONTEND_EQUALITY(qdetector_cccf_get_tau  (q0), qdetector_cccf_get_tau  (q1));
            CONTEND_EQUALITY(qdetector_cccf_get_gamma(q0), qdetector_cccf_get_gamma(q1));
            CONTEND_EQUALITY(qdetector_cccf_get_dphi (q0), qdetector_cccf_get_dphi (q1));
            CONTEND_EQUALITY(qdetector_cccf_get_phi  (q0), qdetector_cccf_get_phi  (q1));
        } else if (v0 != NULL && v1 == NULL) {
            AUTOTEST_FAIL("frame detected on detector 0 but not detector 1");
        } else if (v0 == NULL && v1 != NULL) {
            AUTOTEST_FAIL("frame detected on detector 1 but not detector 0");
        }
    }
    CONTEND_EQUALITY(frames_detected, 1);

    // destroy objects
    qdetector_cccf_destroy(q0);
    qdetector_cccf_destroy(q1);
}


