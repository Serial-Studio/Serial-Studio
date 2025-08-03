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
void autotest_ordfilt_copy()
{
    // create base object
    ordfilt_rrrf q0 = ordfilt_rrrf_create(17, 5);

    // run samples through filter
    unsigned int i;
    float y0, y1;
    for (i=0; i<20; i++) {
        float v = randnf();
        ordfilt_rrrf_execute_one(q0, v, &y0);
    }

    // copy object
    ordfilt_rrrf q1 = ordfilt_rrrf_copy(q0);

    // run samples through both filters in parallel
    for (i=0; i<60; i++) {
        float v = randnf();
        ordfilt_rrrf_execute_one(q0, v, &y0);
        ordfilt_rrrf_execute_one(q1, v, &y1);

        CONTEND_EQUALITY(y0, y1);
    }

    // destroy objects
    ordfilt_rrrf_destroy(q0);
    ordfilt_rrrf_destroy(q1);
}

