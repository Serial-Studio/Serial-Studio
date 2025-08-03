/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
// firdecim_xxxf_autotest.c : test floating-point filters
//

#include "autotest/autotest.h"
#include "liquid.h"

// autotest data definitions
#include "src/filter/tests/firdecim_autotest.h"

void autotest_firdecim_rrrf_common()
{
    firdecim_rrrf decim = firdecim_rrrf_create_kaiser(17, 4, 60.0f);
    CONTEND_EQUALITY(firdecim_rrrf_get_decim_rate(decim), 17);
    firdecim_rrrf_destroy(decim);
}

void autotest_firdecim_crcf_common()
{
    firdecim_crcf decim = firdecim_crcf_create_kaiser(7, 4, 60.0f);
    CONTEND_EQUALITY(firdecim_crcf_get_decim_rate(decim), 7);
    firdecim_crcf_destroy(decim);
}

// 
// AUTOTEST: firdecim_rrrf tests
//
void autotest_firdecim_rrrf_data_M2h4x20()
{
    firdecim_rrrf_test(2,
                       firdecim_rrrf_data_M2h4x20_h, 4,
                       firdecim_rrrf_data_M2h4x20_x, 20,
                       firdecim_rrrf_data_M2h4x20_y, 10);
}
void autotest_firdecim_rrrf_data_M3h7x30()
{
    firdecim_rrrf_test(3,
                       firdecim_rrrf_data_M3h7x30_h, 7,
                       firdecim_rrrf_data_M3h7x30_x, 30,
                       firdecim_rrrf_data_M3h7x30_y, 10);
}
void autotest_firdecim_rrrf_data_M4h13x40()
{
    firdecim_rrrf_test(4,
                       firdecim_rrrf_data_M4h13x40_h, 13,
                       firdecim_rrrf_data_M4h13x40_x, 40,
                       firdecim_rrrf_data_M4h13x40_y, 10);
}
void autotest_firdecim_rrrf_data_M5h23x50()
{
    firdecim_rrrf_test(5,
                       firdecim_rrrf_data_M5h23x50_h, 23,
                       firdecim_rrrf_data_M5h23x50_x, 50,
                       firdecim_rrrf_data_M5h23x50_y, 10);
}


// 
// AUTOTEST: firdecim_crcf tests
//
void autotest_firdecim_crcf_data_M2h4x20()
{
    firdecim_crcf_test(2,
                       firdecim_crcf_data_M2h4x20_h, 4,
                       firdecim_crcf_data_M2h4x20_x, 20,
                       firdecim_crcf_data_M2h4x20_y, 8);
}
void autotest_firdecim_crcf_data_M3h7x30()
{
    firdecim_crcf_test(3,
                       firdecim_crcf_data_M3h7x30_h, 7,
                       firdecim_crcf_data_M3h7x30_x, 30,
                       firdecim_crcf_data_M3h7x30_y, 10);
}
void autotest_firdecim_crcf_data_M4h13x40()
{
    firdecim_crcf_test(4,
                       firdecim_crcf_data_M4h13x40_h, 13,
                       firdecim_crcf_data_M4h13x40_x, 40,
                       firdecim_crcf_data_M4h13x40_y, 10);
}
void autotest_firdecim_crcf_data_M5h23x50()
{
    firdecim_crcf_test(5,
                       firdecim_crcf_data_M5h23x50_h, 23,
                       firdecim_crcf_data_M5h23x50_x, 50,
                       firdecim_crcf_data_M5h23x50_y, 10);
}


// 
// AUTOTEST: firdecim_cccf tests
//
void autotest_firdecim_cccf_data_M2h4x20()
{
    firdecim_cccf_test(2,
                       firdecim_cccf_data_M2h4x20_h, 4,
                       firdecim_cccf_data_M2h4x20_x, 20,
                       firdecim_cccf_data_M2h4x20_y, 8);
}
void autotest_firdecim_cccf_data_M3h7x30()
{
    firdecim_cccf_test(3,
                       firdecim_cccf_data_M3h7x30_h, 7,
                       firdecim_cccf_data_M3h7x30_x, 30,
                       firdecim_cccf_data_M3h7x30_y, 10);
}
void autotest_firdecim_cccf_data_M4h13x40()
{
    firdecim_cccf_test(4,
                       firdecim_cccf_data_M4h13x40_h, 13,
                       firdecim_cccf_data_M4h13x40_x, 40,
                       firdecim_cccf_data_M4h13x40_y, 10);
}
void autotest_firdecim_cccf_data_M5h23x50()
{
    firdecim_cccf_test(5,
                       firdecim_cccf_data_M5h23x50_h, 23,
                       firdecim_cccf_data_M5h23x50_x, 50,
                       firdecim_cccf_data_M5h23x50_y, 10);
}


