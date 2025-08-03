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

//
// fft_composite_autotest.c : test FFTs of 'composite' length (not
//   prime, not of form 2^m)
//

#include "autotest/autotest.h"
#include "liquid.h"

// autotest data definitions
#include "src/fft/tests/fft_runtest.h"

// 
// AUTOTESTS: n-point ffts
//
void autotest_fft_10()  { fft_test( fft_test_x10,   fft_test_y10,   10); }
void autotest_fft_21()  { fft_test( fft_test_x21,   fft_test_y21,   21); }
void autotest_fft_22()  { fft_test( fft_test_x22,   fft_test_y22,   22); }
void autotest_fft_24()  { fft_test( fft_test_x24,   fft_test_y24,   24); }
void autotest_fft_26()  { fft_test( fft_test_x26,   fft_test_y26,   26); }
void autotest_fft_30()  { fft_test( fft_test_x30,   fft_test_y30,   30); }
void autotest_fft_35()  { fft_test( fft_test_x35,   fft_test_y35,   35); }
void autotest_fft_36()  { fft_test( fft_test_x36,   fft_test_y36,   36); }
void autotest_fft_48()  { fft_test( fft_test_x48,   fft_test_y48,   48); }
void autotest_fft_63()  { fft_test( fft_test_x63,   fft_test_y63,   63); }
void autotest_fft_92()  { fft_test( fft_test_x92,   fft_test_y92,   92); }
void autotest_fft_96()  { fft_test( fft_test_x96,   fft_test_y96,   96); }

void autotest_fft_120() { fft_test( fft_test_x120,  fft_test_y120, 120); }
void autotest_fft_130() { fft_test( fft_test_x130,  fft_test_y130, 130); }
void autotest_fft_192() { fft_test( fft_test_x192,  fft_test_y192, 192); }

