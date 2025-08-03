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

#include "autotest/autotest.h"
#include "liquid.h"

// test copy method
void autotest_iirdecim_copy()
{
    // create base object
    iirdecim_crcf q0 = iirdecim_crcf_create_default(3, 7);

    // run samples through filter
    unsigned int i, j;
    float complex buf[3], y0, y1;
    for (i=0; i<20; i++) {
        for (j=0; j<3; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        iirdecim_crcf_execute(q0, buf, &y0);
    }

    // copy object
    iirdecim_crcf q1 = iirdecim_crcf_copy(q0);

    // run samples through both filters in parallel
    for (i=0; i<60; i++) {
        for (j=0; j<3; j++)
            buf[j] = randnf() + _Complex_I*randnf();
        iirdecim_crcf_execute(q0, buf, &y0);
        iirdecim_crcf_execute(q1, buf, &y1);

        CONTEND_EQUALITY( y0, y1 );
    }

    // destroy objects
    iirdecim_crcf_destroy(q0);
    iirdecim_crcf_destroy(q1);
}

