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

// autotest helper function
//  _M      :   decimation rate
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void firdecim_rrrf_test(unsigned int    _M,
                        float *         _h,
                        unsigned int    _h_len,
                        float *         _x,
                        unsigned int    _x_len,
                        float *         _y,
                        unsigned int    _y_len)
{
    float tol = 0.001f;

    // load filter coefficients externally
    firdecim_rrrf q = firdecim_rrrf_create(_M, _h, _h_len);

    // allocate memory for output
    float y_test[_y_len];

    unsigned int i;
    // compute output
    for (i=0; i<_y_len; i++) {
        firdecim_rrrf_execute(q, &_x[_M*i], &y_test[i]);
        
        CONTEND_DELTA( y_test[i], _y[i], tol );
    }
    
    // destroy decimator object object
    firdecim_rrrf_destroy(q);
}

// autotest helper function
//  _M      :   decimation rate
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void firdecim_crcf_test(unsigned int    _M,
                        float *         _h,
                        unsigned int    _h_len,
                        float complex * _x,
                        unsigned int    _x_len,
                        float complex * _y,
                        unsigned int    _y_len)
{
    float tol = 0.001f;

    // load filter coefficients externally
    firdecim_crcf q = firdecim_crcf_create(_M, _h, _h_len);

    // allocate memory for output
    float complex y_test[_y_len];

    unsigned int i;
    // compute output
    for (i=0; i<_y_len; i++) {
        firdecim_crcf_execute(q, &_x[_M*i], &y_test[i]);
        
        CONTEND_DELTA( crealf(y_test[i]), crealf(_y[i]), tol );
        CONTEND_DELTA( cimagf(y_test[i]), cimagf(_y[i]), tol );
    }
    
    // destroy decimator object object
    firdecim_crcf_destroy(q);
}

// autotest helper function
//  _M      :   decimation rate
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void firdecim_cccf_test(unsigned int    _M,
                        float complex * _h,
                        unsigned int    _h_len,
                        float complex * _x,
                        unsigned int    _x_len,
                        float complex * _y,
                        unsigned int    _y_len)
{
    float tol = 0.001f;

    // load filter coefficients externally
    firdecim_cccf q = firdecim_cccf_create(_M, _h, _h_len);

    // allocate memory for output
    float complex y_test[_y_len];

    unsigned int i;
    // compute output
    for (i=0; i<_y_len; i++) {
        firdecim_cccf_execute(q, &_x[_M*i], &y_test[i]);
        
        CONTEND_DELTA( crealf(y_test[i]), crealf(_y[i]), tol );
        CONTEND_DELTA( cimagf(y_test[i]), cimagf(_y[i]), tol );
    }
    
    // destroy decimator object object
    firdecim_cccf_destroy(q);
}

