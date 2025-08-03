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
#include "liquid.h"

// 
// AUTOTEST: validate autocorrelation properties of
//           complementary codes
//
void complementary_codes_test(unsigned int _n)
{
    // create and initialize codes
    bsequence a = bsequence_create(_n);
    bsequence b = bsequence_create(_n);
    bsequence_create_ccodes(a, b);

    // print
    if (liquid_autotest_verbose) {
        bsequence_print(a);
        bsequence_print(b);
    }

    // generate test sequences
    bsequence ax = bsequence_create(_n);
    bsequence bx = bsequence_create(_n);
    bsequence_create_ccodes(ax, bx);

    unsigned int i;
    signed int raa, rbb;
    for (i=0; i<_n; i++) {
        // correlate like sequences
        raa = 2*bsequence_correlate(a,ax) - _n;
        rbb = 2*bsequence_correlate(b,bx) - _n;

        if (i==0) { CONTEND_EQUALITY(raa+rbb,2*_n); }
        else      { CONTEND_EQUALITY(raa+rbb,0);    }

        bsequence_circshift(ax);
        bsequence_circshift(bx);
    }
    
    // clean up memory
    bsequence_destroy(a);
    bsequence_destroy(b);
    bsequence_destroy(ax);
    bsequence_destroy(bx);
}

void autotest_complementary_code_n8()       {   complementary_codes_test(8);    }
void autotest_complementary_code_n16()      {   complementary_codes_test(16);   }
void autotest_complementary_code_n32()      {   complementary_codes_test(32);   }
void autotest_complementary_code_n64()      {   complementary_codes_test(64);   }
void autotest_complementary_code_n128()     {   complementary_codes_test(128);  }
void autotest_complementary_code_n256()     {   complementary_codes_test(256);  }
void autotest_complementary_code_n512()     {   complementary_codes_test(512);  }

