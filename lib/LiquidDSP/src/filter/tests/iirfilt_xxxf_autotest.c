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
// iirfilt_xxxf_autotest.c : test floating-point filters
//

#include "autotest/autotest.h"
#include "liquid.h"

// autotest data definitions
#include "src/filter/tests/iirfilt_autotest.h"

// 
// AUTOTEST: iirfilt_rrrf tests
//
void autotest_iirfilt_rrrf_h3x64()
{
    iirfilt_rrrf_test(iirfilt_rrrf_data_h3x64_b,
                      iirfilt_rrrf_data_h3x64_a, 3,
                      iirfilt_rrrf_data_h3x64_x, 64,
                      iirfilt_rrrf_data_h3x64_y, 64);
}
void autotest_iirfilt_rrrf_h5x64()
{
    iirfilt_rrrf_test(iirfilt_rrrf_data_h5x64_b,
                      iirfilt_rrrf_data_h5x64_a, 5,
                      iirfilt_rrrf_data_h5x64_x, 64,
                      iirfilt_rrrf_data_h5x64_y, 64);
}
void autotest_iirfilt_rrrf_h7x64()
{
    iirfilt_rrrf_test(iirfilt_rrrf_data_h7x64_b,
                      iirfilt_rrrf_data_h7x64_a, 7,
                      iirfilt_rrrf_data_h7x64_x, 64,
                      iirfilt_rrrf_data_h7x64_y, 64);
}


// 
// AUTOTEST: iirfilt_crcf tests
//
void autotest_iirfilt_crcf_h3x64()
{
    iirfilt_crcf_test(iirfilt_crcf_data_h3x64_b,
                      iirfilt_crcf_data_h3x64_a, 3,
                      iirfilt_crcf_data_h3x64_x, 64,
                      iirfilt_crcf_data_h3x64_y, 64);
}
void autotest_iirfilt_crcf_h5x64()
{
    iirfilt_crcf_test(iirfilt_crcf_data_h5x64_b,
                      iirfilt_crcf_data_h5x64_a, 5,
                      iirfilt_crcf_data_h5x64_x, 64,
                      iirfilt_crcf_data_h5x64_y, 64);
}
void autotest_iirfilt_crcf_h7x64()
{
    iirfilt_crcf_test(iirfilt_crcf_data_h7x64_b,
                      iirfilt_crcf_data_h7x64_a, 7,
                      iirfilt_crcf_data_h7x64_x, 64,
                      iirfilt_crcf_data_h7x64_y, 64);
}


// 
// AUTOTEST: iirfilt_cccf tests
//
void autotest_iirfilt_cccf_h3x64()
{
    iirfilt_cccf_test(iirfilt_cccf_data_h3x64_b,
                      iirfilt_cccf_data_h3x64_a, 3,
                      iirfilt_cccf_data_h3x64_x, 64,
                      iirfilt_cccf_data_h3x64_y, 64);
}
void autotest_iirfilt_cccf_h5x64()
{
    iirfilt_cccf_test(iirfilt_cccf_data_h5x64_b,
                      iirfilt_cccf_data_h5x64_a, 5,
                      iirfilt_cccf_data_h5x64_x, 64,
                      iirfilt_cccf_data_h5x64_y, 64);
}
void autotest_iirfilt_cccf_h7x64()
{
    iirfilt_cccf_test(iirfilt_cccf_data_h7x64_b,
                      iirfilt_cccf_data_h7x64_a, 7,
                      iirfilt_cccf_data_h7x64_x, 64,
                      iirfilt_cccf_data_h7x64_y, 64);
}


