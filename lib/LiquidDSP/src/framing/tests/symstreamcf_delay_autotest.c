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

#include <stdio.h>
#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.h"

// autotest helper functions
void testbench_symstreamcf_delay(unsigned int _k,
                                 unsigned int _m)
{
    // create object and get expected delay
    int          ftype  = LIQUID_FIRFILT_ARKAISER;
    float        beta   = 0.30f;
    int          ms     = LIQUID_MODEM_QPSK;
    symstreamcf  gen    = symstreamcf_create_linear(ftype,_k,_m,beta,ms);
    unsigned int delay  = symstreamcf_get_delay(gen);
    float        tol    = 2.0f + _k; // error tolerance (fairly wide due to random signal)

    unsigned int i;
    for (i=0; i<1000 + delay; i++) {
        // generate a single sample
        float complex sample;
        symstreamcf_write_samples(gen, &sample, 1);

        // check to see if value exceeds 1
        if (cabsf(sample) > 0.9f)
            break;
    }

    if (liquid_autotest_verbose)
        printf("expected delay: %u, approximate delay: %u, tol: %.0f\n", delay, i, tol);

    // verify delay is relatively close to expected
    CONTEND_DELTA((float)delay, (float)i, tol);

    // destroy objects
    symstreamcf_destroy(gen);
}

void autotest_symstreamcf_delay_00() { testbench_symstreamcf_delay( 2, 4); }
void autotest_symstreamcf_delay_01() { testbench_symstreamcf_delay( 2, 5); }
void autotest_symstreamcf_delay_02() { testbench_symstreamcf_delay( 2, 6); }
void autotest_symstreamcf_delay_03() { testbench_symstreamcf_delay( 2, 7); }
void autotest_symstreamcf_delay_04() { testbench_symstreamcf_delay( 2, 8); }
void autotest_symstreamcf_delay_05() { testbench_symstreamcf_delay( 2, 9); }
void autotest_symstreamcf_delay_06() { testbench_symstreamcf_delay( 2,10); }
void autotest_symstreamcf_delay_07() { testbench_symstreamcf_delay( 2,14); }
void autotest_symstreamcf_delay_08() { testbench_symstreamcf_delay( 2,20); }
void autotest_symstreamcf_delay_09() { testbench_symstreamcf_delay( 2,31); }

void autotest_symstreamcf_delay_10() { testbench_symstreamcf_delay( 3,12); }
void autotest_symstreamcf_delay_11() { testbench_symstreamcf_delay( 4,12); }
void autotest_symstreamcf_delay_12() { testbench_symstreamcf_delay( 5,12); }
void autotest_symstreamcf_delay_13() { testbench_symstreamcf_delay( 6,12); }
void autotest_symstreamcf_delay_14() { testbench_symstreamcf_delay( 7,12); }
void autotest_symstreamcf_delay_15() { testbench_symstreamcf_delay( 8,12); }
void autotest_symstreamcf_delay_16() { testbench_symstreamcf_delay( 9,12); }
void autotest_symstreamcf_delay_17() { testbench_symstreamcf_delay(10,12); }
void autotest_symstreamcf_delay_18() { testbench_symstreamcf_delay(11,12); }
void autotest_symstreamcf_delay_19() { testbench_symstreamcf_delay(12,12); }


