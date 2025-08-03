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
// fft_radix2_autotest.c : test power-of-two transforms
//

#include "autotest/autotest.h"
#include "liquid.h"

// autotest data definitions
#include "src/fft/tests/fft_runtest.h"

// 
// AUTOTESTS: power-of-two transforms
//
void autotest_fft_2()       { fft_test( fft_test_x2,   fft_test_y2,      2);     }
void autotest_fft_4()       { fft_test( fft_test_x4,   fft_test_y4,      4);     }
void autotest_fft_8()       { fft_test( fft_test_x8,   fft_test_y8,      8);     }
void autotest_fft_16()      { fft_test( fft_test_x16,  fft_test_y16,     16);    }
void autotest_fft_32()      { fft_test( fft_test_x32,  fft_test_y32,     32);    }
void autotest_fft_64()      { fft_test( fft_test_x64,  fft_test_y64,     64);    }

