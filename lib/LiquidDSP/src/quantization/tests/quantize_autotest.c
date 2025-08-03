/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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

void autotest_quantize_float_n8() {
    float x = -1.0f;
    unsigned int num_steps=30;
    unsigned int num_bits=8;

    float dx = 2/(float)(num_steps);
    unsigned int q;
    float x_hat;
    float tol = 1.0f / (float)(1<<num_bits);

    unsigned int i;
    for (i=0; i<num_steps; i++) {
        q = quantize_adc(x,num_bits);

        // ensure only num_bits written to value q
        CONTEND_EQUALITY(q>>num_bits, 0);

        x_hat = quantize_dac(q,num_bits);

        if (liquid_autotest_verbose)
            printf("%8.4f > 0x%2.2x > %8.4f\n", x, q, x_hat);

        // ensure original value is recovered within tolerance
        CONTEND_DELTA(x,x_hat,tol);

        x += dx;
        x = (x > 1.0f) ? 1.0f : x;
    }
}

