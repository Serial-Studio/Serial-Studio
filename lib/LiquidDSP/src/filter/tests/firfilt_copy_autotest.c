/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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

void autotest_firfilt_crcf_copy()
{
    // design filter from prototype
    firfilt_crcf filt_orig = firfilt_crcf_create_kaiser(21, 0.345f, 60.0f, 0.0f);
    firfilt_crcf_set_scale(filt_orig, 2.0f);
    firfilt_crcf_print(filt_orig);

    // start running input through filter
    unsigned int n = 32;
    unsigned int i;
    float complex x, y_orig, y_copy;
    for (i=0; i<n; i++) {
        x = randnf() + _Complex_I*randnf();
        firfilt_crcf_push(filt_orig, x);
    }

    // copy filter
    firfilt_crcf filt_copy = firfilt_crcf_copy(filt_orig);

    // continue running through both filters
    for (i=0; i<n; i++) {
        // run filters in parallel
        x = randnf() + _Complex_I*randnf();
        firfilt_crcf_execute_one(filt_orig, x, &y_orig);
        firfilt_crcf_execute_one(filt_copy, x, &y_copy);

        if (liquid_autotest_verbose) {
            float error = cabsf( y_orig - y_copy );
            printf(" [%3u] orig: %12.8f + j%12.8f, copy: %12.8f + j%12.8f, err: %8g\n",
                    i+n,
                    crealf(y_orig), cimagf(y_orig),
                    crealf(y_copy), cimagf(y_copy),
                    error);
        }
        CONTEND_EQUALITY(y_orig, y_copy);
    }

    // destroy filter objects
    firfilt_crcf_destroy(filt_orig);
    firfilt_crcf_destroy(filt_copy);
}

