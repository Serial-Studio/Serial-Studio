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

#include "autotest/autotest.h"
#include "liquid.h"

void testbench_gcd(unsigned int _gcd,
                   unsigned int _p,
                   unsigned int _q)
{
	unsigned int P = _p * _gcd;
	unsigned int Q = _q * _gcd;
	unsigned int gcd_test = liquid_gcd(P,Q);
    if (liquid_autotest_verbose)
        printf("  gcd(%12u,%12u) = %12u (expected %u)\n", P, Q, gcd_test, _gcd);
    CONTEND_EQUALITY(gcd_test, _gcd);
}

// no common roots; gcd = 1
void autotest_gcd_one()
{
    testbench_gcd( 1,  2,  3);
    testbench_gcd( 1,  2,  5);
    testbench_gcd( 1,  2,  7);
    testbench_gcd( 1,  7,  2);
    testbench_gcd( 1, 17, 19);
    testbench_gcd( 1, 23, 31);
    testbench_gcd( 1, 2*2*2*2*2, 3*5*7*19);
}

// edge cases
void autotest_gcd_edge_cases()
{
    testbench_gcd( 1,         1, 1);
    testbench_gcd( 1,         1, 2);
    testbench_gcd( 1,         1, 2400);
    testbench_gcd( 12345,     1, 1);    // P==Q
    testbench_gcd( (1<<17)-1, 1, 1);    // P==Q
}

// set of base tests
void autotest_gcd_base()
{
    testbench_gcd( 2*2*3*5*7, 2*3, 17);
    testbench_gcd( 2*2*3*5*7, 2*3, 17*17);
    testbench_gcd( 2*2*3*5*7, 2*3, 17*17*17);
    testbench_gcd( 11,        3,   1);
    testbench_gcd( 3,         127, 131);
    testbench_gcd( 131,       127, 3);
    testbench_gcd( 127,       131, 3);
}

