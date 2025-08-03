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
// autotest firdecim data definitions
//

#ifndef __LIQUID_FIRDECIM_H__
#define __LIQUID_FIRDECIM_H__

// autotest helper functions:
//  _M      :   decimation factor
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void firdecim_rrrf_test(unsigned int _M,
                        float *      _h,
                        unsigned int _h_len,
                        float *      _x,
                        unsigned int _x_len,
                        float *      _y,
                        unsigned int _y_len);

void firdecim_crcf_test(unsigned int    _M,
                        float *         _h,
                        unsigned int    _h_len,
                        float complex * _x,
                        unsigned int    _x_len,
                        float complex * _y,
                        unsigned int    _y_len);

void firdecim_cccf_test(unsigned int    _M,
                        float complex * _h,
                        unsigned int    _h_len,
                        float complex * _x,
                        unsigned int    _x_len,
                        float complex * _y,
                        unsigned int    _y_len);

// 
// autotest datasets
//

// rrrf
extern float         firdecim_rrrf_data_M2h4x20_h[];
extern float         firdecim_rrrf_data_M2h4x20_x[];
extern float         firdecim_rrrf_data_M2h4x20_y[];

extern float         firdecim_rrrf_data_M3h7x30_h[];
extern float         firdecim_rrrf_data_M3h7x30_x[];
extern float         firdecim_rrrf_data_M3h7x30_y[];

extern float         firdecim_rrrf_data_M4h13x40_h[];
extern float         firdecim_rrrf_data_M4h13x40_x[];
extern float         firdecim_rrrf_data_M4h13x40_y[];

extern float         firdecim_rrrf_data_M5h23x50_h[];
extern float         firdecim_rrrf_data_M5h23x50_x[];
extern float         firdecim_rrrf_data_M5h23x50_y[];

// crcf
extern float         firdecim_crcf_data_M2h4x20_h[];
extern float complex firdecim_crcf_data_M2h4x20_x[];
extern float complex firdecim_crcf_data_M2h4x20_y[];

extern float         firdecim_crcf_data_M3h7x30_h[];
extern float complex firdecim_crcf_data_M3h7x30_x[];
extern float complex firdecim_crcf_data_M3h7x30_y[];

extern float         firdecim_crcf_data_M4h13x40_h[];
extern float complex firdecim_crcf_data_M4h13x40_x[];
extern float complex firdecim_crcf_data_M4h13x40_y[];

extern float         firdecim_crcf_data_M5h23x50_h[];
extern float complex firdecim_crcf_data_M5h23x50_x[];
extern float complex firdecim_crcf_data_M5h23x50_y[];

// cccf
extern float complex firdecim_cccf_data_M2h4x20_h[];
extern float complex firdecim_cccf_data_M2h4x20_x[];
extern float complex firdecim_cccf_data_M2h4x20_y[];

extern float complex firdecim_cccf_data_M3h7x30_h[];
extern float complex firdecim_cccf_data_M3h7x30_x[];
extern float complex firdecim_cccf_data_M3h7x30_y[];

extern float complex firdecim_cccf_data_M4h13x40_h[];
extern float complex firdecim_cccf_data_M4h13x40_x[];
extern float complex firdecim_cccf_data_M4h13x40_y[];

extern float complex firdecim_cccf_data_M5h23x50_h[];
extern float complex firdecim_cccf_data_M5h23x50_x[];
extern float complex firdecim_cccf_data_M5h23x50_y[];

#endif // __LIQUID_FIRDECIM_H__

