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
// firfilt_xxxf_autotest.c : test floating-point filters
//

#include "autotest/autotest.h"
#include "liquid.h"

// autotest data definitions
#include "src/filter/tests/firfilt_autotest.h"

// 
// AUTOTEST: firfilt_rrrf tests
//
void autotest_firfilt_rrrf_data_h4x8()
{
    firfilt_rrrf_test(firfilt_rrrf_data_h4x8_h, 4,
                      firfilt_rrrf_data_h4x8_x, 8,
                      firfilt_rrrf_data_h4x8_y, 8);
}
void autotest_firfilt_rrrf_data_h7x16()
{
    firfilt_rrrf_test(firfilt_rrrf_data_h7x16_h, 7,
                      firfilt_rrrf_data_h7x16_x, 16,
                      firfilt_rrrf_data_h7x16_y, 16);
}
void autotest_firfilt_rrrf_data_h13x32()
{
    firfilt_rrrf_test(firfilt_rrrf_data_h13x32_h, 13,
                      firfilt_rrrf_data_h13x32_x, 32,
                      firfilt_rrrf_data_h13x32_y, 32);
}
void autotest_firfilt_rrrf_data_h23x64()
{
    firfilt_rrrf_test(firfilt_rrrf_data_h23x64_h, 23,
                      firfilt_rrrf_data_h23x64_x, 64,
                      firfilt_rrrf_data_h23x64_y, 64);
}


// 
// AUTOTEST: firfilt_crcf tests
//
void autotest_firfilt_crcf_data_h4x8()
{
    firfilt_crcf_test(firfilt_crcf_data_h4x8_h, 4,
                      firfilt_crcf_data_h4x8_x, 8,
                      firfilt_crcf_data_h4x8_y, 8);
}
void autotest_firfilt_crcf_data_h7x16()
{
    firfilt_crcf_test(firfilt_crcf_data_h7x16_h, 7,
                      firfilt_crcf_data_h7x16_x, 16,
                      firfilt_crcf_data_h7x16_y, 16);
}
void autotest_firfilt_crcf_data_h13x32()
{
    firfilt_crcf_test(firfilt_crcf_data_h13x32_h, 13,
                      firfilt_crcf_data_h13x32_x, 32,
                      firfilt_crcf_data_h13x32_y, 32);
}
void autotest_firfilt_crcf_data_h23x64()
{
    firfilt_crcf_test(firfilt_crcf_data_h23x64_h, 23,
                      firfilt_crcf_data_h23x64_x, 64,
                      firfilt_crcf_data_h23x64_y, 64);
}


// 
// AUTOTEST: firfilt_cccf tests
//
void autotest_firfilt_cccf_data_h4x8()
{
    firfilt_cccf_test(firfilt_cccf_data_h4x8_h, 4,
                      firfilt_cccf_data_h4x8_x, 8,
                      firfilt_cccf_data_h4x8_y, 8);
}
void autotest_firfilt_cccf_data_h7x16()
{
    firfilt_cccf_test(firfilt_cccf_data_h7x16_h, 7,
                      firfilt_cccf_data_h7x16_x, 16,
                      firfilt_cccf_data_h7x16_y, 16);
}
void autotest_firfilt_cccf_data_h13x32()
{
    firfilt_cccf_test(firfilt_cccf_data_h13x32_h, 13,
                      firfilt_cccf_data_h13x32_x, 32,
                      firfilt_cccf_data_h13x32_y, 32);
}
void autotest_firfilt_cccf_data_h23x64()
{
    firfilt_cccf_test(firfilt_cccf_data_h23x64_h, 23,
                      firfilt_cccf_data_h23x64_x, 64,
                      firfilt_cccf_data_h23x64_y, 64);
}


