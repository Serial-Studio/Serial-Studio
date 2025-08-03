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

// test filter copy operation

#include "autotest/autotest.h"
#include "liquid.h"

// NOTE: this will never pass since the random sources are different; need to
//       implement random number generator first
void xautotest_channel_copy()
{
    // create default channel object and set properties
    channel_cccf q0 = channel_cccf_create();
    channel_cccf_add_awgn          (q0, -100, 20);
    channel_cccf_add_carrier_offset(q0, 1e-5f, 1.2f);
    channel_cccf_add_multipath     (q0, NULL, 21);
    channel_cccf_add_shadowing     (q0, 1.0f, 0.1f);
    channel_cccf_print(q0);

    // run samples through original object
    unsigned int i;
    float complex x, y0, y1;
    for (i=0; i<120; i++) {
        x = randnf() + _Complex_I*randnf();
        channel_cccf_execute(q0, x, &y0);
    }

    // copy filter
    channel_cccf q1 = channel_cccf_copy(q0);

    // continue running through both objects
    for (i=0; i<120; i++) {
        // run objects in parallel
        x = randnf() + _Complex_I*randnf();
        channel_cccf_execute(q0, x, &y0);
        channel_cccf_execute(q1, x, &y1);

        CONTEND_EQUALITY(y0, y1);
    }

    // destroy filter objects
    channel_cccf_destroy(q0);
    channel_cccf_destroy(q1);
}

