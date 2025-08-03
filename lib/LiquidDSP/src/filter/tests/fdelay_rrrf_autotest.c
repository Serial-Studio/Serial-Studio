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

#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

// test setting filter delay
void testbench_fdelay_rrrf(unsigned int _nmax,
                           unsigned int _m,
                           unsigned int _npfb,
                           float        _delay)
{
    float tol = 0.01f;
    unsigned int num_samples = _nmax + 2*_m;  // number of samples to run

    // create delay object and split delay between set and adjust methods
    fdelay_rrrf q = fdelay_rrrf_create(_nmax, _m, _npfb);
    fdelay_rrrf_set_delay   (q, _delay*0.7f);
    fdelay_rrrf_adjust_delay(q, _delay*0.3f);

    // ensure object is configured properly
    CONTEND_EQUALITY(fdelay_rrrf_get_nmax (q), _nmax);
    CONTEND_EQUALITY(fdelay_rrrf_get_m    (q), _m);
    CONTEND_EQUALITY(fdelay_rrrf_get_npfb (q), _npfb);
    CONTEND_DELTA   (fdelay_rrrf_get_delay(q), _delay, 1e-6f);

    // generate impulse and propagate through object
    float x[num_samples];
    float y[num_samples];
    unsigned int i;
    // generate input
    for (i=0; i<num_samples; i++)
        x[i] = i==0 ? 1.0f : 0.0f;

    // run filter
    fdelay_rrrf_execute_block(q, x, num_samples, y);

    // destroy filter object
    fdelay_rrrf_destroy(q);

    // estimate delay; assumes input is impulse and uses phase at
    // single point of frequency estimate evaluation
    float fc = 0.1f / (float)num_samples; // sufficiently small
    float complex v = 0.0f;
    for (i=0; i<num_samples; i++)
        v += y[i] * cexpf(_Complex_I*2*M_PI*fc*i);
    float delay_est = cargf(v) / (2*M_PI*fc) - (float)_m;
    if (liquid_autotest_verbose) {
        printf("delay: measured=%g, expected=%g (error=%g)\n",
                delay_est, _delay, delay_est - _delay);
    }

    // verify delay
    CONTEND_DELTA(delay_est, _delay, tol);
}

// nominal delays
void autotest_fdelay_rrrf_0()  { testbench_fdelay_rrrf(200, 12, 64,   0.0f   ); }
void autotest_fdelay_rrrf_1()  { testbench_fdelay_rrrf(200, 12, 64,   0.0001f); }
void autotest_fdelay_rrrf_2()  { testbench_fdelay_rrrf(200, 12, 64,   0.1f   ); }
void autotest_fdelay_rrrf_3()  { testbench_fdelay_rrrf(200, 12, 64,   0.9f   ); }
void autotest_fdelay_rrrf_4()  { testbench_fdelay_rrrf(200, 12, 64,   0.9999f); }
void autotest_fdelay_rrrf_5()  { testbench_fdelay_rrrf(200, 12, 64,  16.99f  ); }
void autotest_fdelay_rrrf_6()  { testbench_fdelay_rrrf(200, 12, 64,  17.00f  ); }
void autotest_fdelay_rrrf_7()  { testbench_fdelay_rrrf(200, 12, 64,  17.01f  ); }
void autotest_fdelay_rrrf_8()  { testbench_fdelay_rrrf(200, 12, 64, 199.9f   ); }
void autotest_fdelay_rrrf_9()  { testbench_fdelay_rrrf(200, 12, 64, 200.0f   ); }

void autotest_fdelay_rrrf_config()
{
#ifdef LIQUID_STRICT_EXIT
    AUTOTEST_WARN("delay_rrrf test not run with strict mode enabled");
    return;
#endif
    AUTOTEST_WARN("testing delay_rrrf invalid configurations; ignore printed errors");
    // default configurations
    unsigned int nmax  = 200;
    unsigned int m     =  12;
    unsigned int npfb  =  64;

    // test invalid configurations, normal construction
    CONTEND_ISNULL(fdelay_rrrf_create(   0, m, npfb))
    CONTEND_ISNULL(fdelay_rrrf_create(nmax, 0, npfb))
    CONTEND_ISNULL(fdelay_rrrf_create(nmax, m,    0))

    // test invalid configurations, default construction
    CONTEND_ISNULL(fdelay_rrrf_create_default(   0))

    // create proper object but test invalid internal configurations
    fdelay_rrrf q = fdelay_rrrf_create_default(nmax);

    CONTEND_INEQUALITY(LIQUID_OK, fdelay_rrrf_set_delay   (q, -1))
    CONTEND_INEQUALITY(LIQUID_OK, fdelay_rrrf_set_delay   (q, nmax+1))
    CONTEND_INEQUALITY(LIQUID_OK, fdelay_rrrf_adjust_delay(q, -1))

    // test valid configurations, methods
    CONTEND_EQUALITY(LIQUID_OK, fdelay_rrrf_print(q))

    // destroy object
    fdelay_rrrf_destroy(q);
}

// compare push vs write methods
void autotest_fdelay_rrrf_push_write()
{
    // create two identical objects
    fdelay_rrrf q0 = fdelay_rrrf_create_default(200);
    fdelay_rrrf q1 = fdelay_rrrf_create_default(200);

    // set identical delays
    fdelay_rrrf_set_delay(q0, 7.2280f);
    fdelay_rrrf_set_delay(q1, 7.2280f);

    // generate pseudo-random inputs, and compare outputs
    float buf[8] = {-1,3,5,-3,5,1,-3,-4};
    unsigned int trial, i;
    for (trial=0; trial<20; trial++) {
        unsigned int n = trial % 8;

        // push/write samples
        for (i=0; i<n; i++)
            fdelay_rrrf_push(q0, buf[i]);
        fdelay_rrrf_write(q1, buf, n);

        // compute outputs and compare
        float v0, v1;
        fdelay_rrrf_execute(q0, &v0);
        fdelay_rrrf_execute(q1, &v1);
        CONTEND_EQUALITY(v0, v1);
    }

    // destroy objects
    fdelay_rrrf_destroy(q0);
    fdelay_rrrf_destroy(q1);
}

