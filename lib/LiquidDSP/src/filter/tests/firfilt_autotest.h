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
// autotest firfilt data definitions
//

#ifndef __LIQUID_FIRFILT_H__
#define __LIQUID_FIRFILT_H__

// autotest helper functions:
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void firfilt_rrrf_test(float *      _h,
                       unsigned int _h_len,
                       float *      _x,
                       unsigned int _x_len,
                       float *      _y,
                       unsigned int _y_len);

void firfilt_crcf_test(float *         _h,
                       unsigned int    _h_len,
                       float complex * _x,
                       unsigned int    _x_len,
                       float complex * _y,
                       unsigned int    _y_len);

void firfilt_cccf_test(float complex * _h,
                       unsigned int    _h_len,
                       float complex * _x,
                       unsigned int    _x_len,
                       float complex * _y,
                       unsigned int    _y_len);

// 
// autotest datasets
//

// rrrf
extern float         firfilt_rrrf_data_h4x8_h[];
extern float         firfilt_rrrf_data_h4x8_x[];
extern float         firfilt_rrrf_data_h4x8_y[];

extern float         firfilt_rrrf_data_h7x16_h[];
extern float         firfilt_rrrf_data_h7x16_x[];
extern float         firfilt_rrrf_data_h7x16_y[];

extern float         firfilt_rrrf_data_h13x32_h[];
extern float         firfilt_rrrf_data_h13x32_x[];
extern float         firfilt_rrrf_data_h13x32_y[];

extern float         firfilt_rrrf_data_h23x64_h[];
extern float         firfilt_rrrf_data_h23x64_x[];
extern float         firfilt_rrrf_data_h23x64_y[];

// crcf
extern float         firfilt_crcf_data_h4x8_h[];
extern float complex firfilt_crcf_data_h4x8_x[];
extern float complex firfilt_crcf_data_h4x8_y[];

extern float         firfilt_crcf_data_h7x16_h[];
extern float complex firfilt_crcf_data_h7x16_x[];
extern float complex firfilt_crcf_data_h7x16_y[];

extern float         firfilt_crcf_data_h13x32_h[];
extern float complex firfilt_crcf_data_h13x32_x[];
extern float complex firfilt_crcf_data_h13x32_y[];

extern float         firfilt_crcf_data_h23x64_h[];
extern float complex firfilt_crcf_data_h23x64_x[];
extern float complex firfilt_crcf_data_h23x64_y[];

// cccf
extern float complex firfilt_cccf_data_h4x8_h[];
extern float complex firfilt_cccf_data_h4x8_x[];
extern float complex firfilt_cccf_data_h4x8_y[];

extern float complex firfilt_cccf_data_h7x16_h[];
extern float complex firfilt_cccf_data_h7x16_x[];
extern float complex firfilt_cccf_data_h7x16_y[];

extern float complex firfilt_cccf_data_h13x32_h[];
extern float complex firfilt_cccf_data_h13x32_x[];
extern float complex firfilt_cccf_data_h13x32_y[];

extern float complex firfilt_cccf_data_h23x64_h[];
extern float complex firfilt_cccf_data_h23x64_x[];
extern float complex firfilt_cccf_data_h23x64_y[];

#endif // __LIQUID_FIRFILT_H__

