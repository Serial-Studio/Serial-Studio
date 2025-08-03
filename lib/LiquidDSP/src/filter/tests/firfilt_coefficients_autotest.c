/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.h"

void autotest_firfilt_cccf_coefficients_test()
{
    // create filter coefficients (semi-random)
    unsigned int h_len = 71;
    float complex * h0 = (float complex*) malloc(h_len*sizeof(float complex));
    float complex * h1 = (float complex*) malloc(h_len*sizeof(float complex));
    unsigned int i;
    for (i=0; i<h_len; i++)
        h0[i] = cexpf(_Complex_I*0.2f*i) * liquid_hamming(i,h_len);

    // design filter from external coefficients
    firfilt_cccf q = firfilt_cccf_create(h0, h_len);

    // set scale: note that this is not used when computing coefficients
    firfilt_cccf_set_scale(q, -0.4f + _Complex_I*0.7f);

    // copy coefficients from filter object
    firfilt_cccf_copy_coefficients(q, h1);

    // copy coefficients from filter object
    const float complex * h2 = firfilt_cccf_get_coefficients(q);

    // ensure values are equal; no need for tolerance as values should be exact
    for (i=0; i<h_len; i++) {
        CONTEND_EQUALITY(crealf(h0[i]), crealf(h1[i]));
        CONTEND_EQUALITY(cimagf(h0[i]), cimagf(h1[i]));

        CONTEND_EQUALITY(crealf(h0[i]), crealf(h2[i]));
        CONTEND_EQUALITY(cimagf(h0[i]), cimagf(h2[i]));
    }

    // destroy filter object and free memory
    firfilt_cccf_destroy(q);
    free(h0);
    free(h1);
}

