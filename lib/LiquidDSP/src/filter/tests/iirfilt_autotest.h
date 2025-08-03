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
// autotest iirfilt data definitions
//

#ifndef __LIQUID_IIRFILT_H__
#define __LIQUID_IIRFILT_H__

// autotest helper functions:
//  _b      :   filter coefficients (numerator)
//  _a      :   filter coefficients (denominator)
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void iirfilt_rrrf_test(float *      _b,
                       float *      _a,
                       unsigned int _h_len,
                       float *      _x,
                       unsigned int _x_len,
                       float *      _y,
                       unsigned int _y_len);

void iirfilt_crcf_test(float *         _b,
                       float *         _a,
                       unsigned int    _h_len,
                       float complex * _x,
                       unsigned int    _x_len,
                       float complex * _y,
                       unsigned int    _y_len);

void iirfilt_cccf_test(float complex * _b,
                       float complex * _a,
                       unsigned int    _h_len,
                       float complex * _x,
                       unsigned int    _x_len,
                       float complex * _y,
                       unsigned int    _y_len);

// 
// autotest datasets
//

// rrrf
extern float         iirfilt_rrrf_data_h3x64_b[];
extern float         iirfilt_rrrf_data_h3x64_a[];
extern float         iirfilt_rrrf_data_h3x64_x[];
extern float         iirfilt_rrrf_data_h3x64_y[];

extern float         iirfilt_rrrf_data_h5x64_b[];
extern float         iirfilt_rrrf_data_h5x64_a[];
extern float         iirfilt_rrrf_data_h5x64_x[];
extern float         iirfilt_rrrf_data_h5x64_y[];

extern float         iirfilt_rrrf_data_h7x64_b[];
extern float         iirfilt_rrrf_data_h7x64_a[];
extern float         iirfilt_rrrf_data_h7x64_x[];
extern float         iirfilt_rrrf_data_h7x64_y[];

// crcf
extern float         iirfilt_crcf_data_h3x64_b[];
extern float         iirfilt_crcf_data_h3x64_a[];
extern float complex iirfilt_crcf_data_h3x64_x[];
extern float complex iirfilt_crcf_data_h3x64_y[];

extern float         iirfilt_crcf_data_h5x64_b[];
extern float         iirfilt_crcf_data_h5x64_a[];
extern float complex iirfilt_crcf_data_h5x64_x[];
extern float complex iirfilt_crcf_data_h5x64_y[];

extern float         iirfilt_crcf_data_h7x64_b[];
extern float         iirfilt_crcf_data_h7x64_a[];
extern float complex iirfilt_crcf_data_h7x64_x[];
extern float complex iirfilt_crcf_data_h7x64_y[];

// cccf
extern float complex iirfilt_cccf_data_h3x64_b[];
extern float complex iirfilt_cccf_data_h3x64_a[];
extern float complex iirfilt_cccf_data_h3x64_x[];
extern float complex iirfilt_cccf_data_h3x64_y[];

extern float complex iirfilt_cccf_data_h5x64_b[];
extern float complex iirfilt_cccf_data_h5x64_a[];
extern float complex iirfilt_cccf_data_h5x64_x[];
extern float complex iirfilt_cccf_data_h5x64_y[];

extern float complex iirfilt_cccf_data_h7x64_b[];
extern float complex iirfilt_cccf_data_h7x64_a[];
extern float complex iirfilt_cccf_data_h7x64_x[];
extern float complex iirfilt_cccf_data_h7x64_y[];

#endif // __LIQUID_IIRFILT_H__

