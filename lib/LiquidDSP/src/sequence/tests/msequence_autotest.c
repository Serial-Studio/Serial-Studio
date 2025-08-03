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

// test initialization of binary sequence on m-sequence
void autotest_bsequence_init_msequence() {
    // create and initialize m-sequence
    msequence ms = msequence_create_default(4);

    // create and initialize binary sequence on m-sequence
    bsequence bs;
    bs = bsequence_create( msequence_get_length(ms) );
    bsequence_init_msequence(bs, ms);
    if (liquid_autotest_verbose)
        bsequence_print(bs);

    CONTEND_EQUALITY( bsequence_get_length(bs), msequence_get_length(ms) )

    // clean up memory
    bsequence_destroy(bs);
    msequence_destroy(ms);
}

// helper function to test auto-correlation properties of m-sequence
void msequence_test_autocorrelation(unsigned int _m)
{
    // create and initialize m-sequence
    msequence ms = msequence_create_default(_m);
    unsigned int n = msequence_get_length(ms);

    // create and initialize first binary sequence on m-sequence
    bsequence bs1 = bsequence_create(n);
    bsequence_init_msequence(bs1, ms);

    // create and initialize second binary sequence on same m-sequence
    bsequence bs2 = bsequence_create(n);
    bsequence_init_msequence(bs2, ms);

    // ensure sequences are the same length
    CONTEND_EQUALITY( bsequence_get_length(bs1), n )
    CONTEND_EQUALITY( bsequence_get_length(bs2), n )

    // when sequences are aligned, autocorrelation is equal to length
    signed int rxy;
    rxy = bsequence_correlate(bs1, bs2);
    CONTEND_EQUALITY( rxy, n )

    // when sequences are misaligned, autocorrelation is equal to -1
    unsigned int i;
    for (i=0; i<n-1; i++) {
        bsequence_push(bs2, msequence_advance(ms));
        rxy = 2*bsequence_correlate(bs1, bs2) - n;
        CONTEND_EQUALITY( rxy, -1 );
    }

    // clean up memory
    bsequence_destroy(bs1);
    bsequence_destroy(bs2);
    msequence_destroy(ms);
}

void autotest_msequence_xcorr_m2()  { msequence_test_autocorrelation(2);  }   // n = 3
void autotest_msequence_xcorr_m3()  { msequence_test_autocorrelation(3);  }   // n = 7
void autotest_msequence_xcorr_m4()  { msequence_test_autocorrelation(4);  }   // n = 15
void autotest_msequence_xcorr_m5()  { msequence_test_autocorrelation(5);  }   // n = 31
void autotest_msequence_xcorr_m6()  { msequence_test_autocorrelation(6);  }   // n = 63
void autotest_msequence_xcorr_m7()  { msequence_test_autocorrelation(7);  }   // n = 127
void autotest_msequence_xcorr_m8()  { msequence_test_autocorrelation(8);  }   // n = 255
void autotest_msequence_xcorr_m9()  { msequence_test_autocorrelation(9);  }   // n = 511
void autotest_msequence_xcorr_m10() { msequence_test_autocorrelation(10); }   // n = 1023
void autotest_msequence_xcorr_m11() { msequence_test_autocorrelation(11); }   // n = 2047
void autotest_msequence_xcorr_m12() { msequence_test_autocorrelation(12); }   // n = 4095

// helper function to test cyclic period of sequences
void msequence_test_period(unsigned int _m)
{
    // create and initialize m-sequence
    msequence q = msequence_create_default(_m);

    // measure period and compare to expected
    unsigned int n = (1U << _m) - 1;
    unsigned int p = msequence_measure_period(q);
    CONTEND_EQUALITY(p,n)

    // clean up objects
    msequence_destroy(q);
}
void autotest_msequence_period_m2()  { msequence_test_period(2);  }
void autotest_msequence_period_m3()  { msequence_test_period(3);  }
void autotest_msequence_period_m4()  { msequence_test_period(4);  }
void autotest_msequence_period_m5()  { msequence_test_period(5);  }
void autotest_msequence_period_m6()  { msequence_test_period(6);  }
void autotest_msequence_period_m7()  { msequence_test_period(7);  }
void autotest_msequence_period_m8()  { msequence_test_period(8);  }
void autotest_msequence_period_m9()  { msequence_test_period(9);  }
void autotest_msequence_period_m10() { msequence_test_period(10); }
void autotest_msequence_period_m11() { msequence_test_period(11); }
void autotest_msequence_period_m12() { msequence_test_period(12); }
void autotest_msequence_period_m13() { msequence_test_period(13); }
void autotest_msequence_period_m14() { msequence_test_period(14); }
void autotest_msequence_period_m15() { msequence_test_period(15); }
void autotest_msequence_period_m16() { msequence_test_period(16); }
void autotest_msequence_period_m17() { msequence_test_period(17); }
void autotest_msequence_period_m18() { msequence_test_period(18); }
void autotest_msequence_period_m19() { msequence_test_period(19); }
void autotest_msequence_period_m20() { msequence_test_period(20); }
void autotest_msequence_period_m21() { msequence_test_period(21); }
void autotest_msequence_period_m22() { msequence_test_period(22); }
void autotest_msequence_period_m23() { msequence_test_period(23); }
void autotest_msequence_period_m24() { msequence_test_period(24); }
void autotest_msequence_period_m25() { msequence_test_period(25); }
void autotest_msequence_period_m26() { msequence_test_period(26); }
void autotest_msequence_period_m27() { msequence_test_period(27); }
void autotest_msequence_period_m28() { msequence_test_period(28); }
void autotest_msequence_period_m29() { msequence_test_period(29); }
void autotest_msequence_period_m30() { msequence_test_period(30); }
void autotest_msequence_period_m31() { msequence_test_period(31); }

void autotest_msequence_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping firfilt config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // check invalid configurations
    CONTEND_ISNULL(msequence_create        (100, 0, 0))
    CONTEND_ISNULL(msequence_create_default( 32)) // too long
    CONTEND_ISNULL(msequence_create_genpoly(  0))

    // create proper object and test configurations
    msequence q = msequence_create_genpoly(LIQUID_MSEQUENCE_GENPOLY_M11);

    CONTEND_EQUALITY(LIQUID_OK, msequence_print(q))
    CONTEND_EQUALITY(1,         msequence_get_state(q))
    CONTEND_EQUALITY(LIQUID_OK, msequence_set_state(q, 0x8a))
    CONTEND_EQUALITY(0x8a,      msequence_get_state(q))

    msequence_destroy(q);
}

