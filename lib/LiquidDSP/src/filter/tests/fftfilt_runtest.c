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

#include <stdlib.h>
#include <string.h>
#include "autotest/autotest.h"
#include "liquid.h"

// autotest helper function
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void fftfilt_rrrf_test(float *      _h,
                       unsigned int _h_len,
                       float *      _x,
                       unsigned int _x_len,
                       float *      _y,
                       unsigned int _y_len)
{
    float tol = 0.001f;

    // determine appropriate block size
    // NOTE: this number can be anything at least _h_len-1
    unsigned int n = 1 << liquid_nextpow2(_h_len-1);

    // determine number of blocks
    div_t d = div(_x_len, n);
    unsigned int num_blocks = d.quot + (d.rem ? 1 : 0);
    if (liquid_autotest_verbose) {
        printf("fftfilt_rrrf_test(), h_len: %3u, x_len: %3u (%3u blocks @ %3u samples, %3u remaining)\n",
                _h_len, _x_len, n, d.quot, d.rem);
    }

    // load filter coefficients externally
    fftfilt_rrrf q = fftfilt_rrrf_create(_h, _h_len, n);

    // allocate memory for output
    float y_test[n*num_blocks];

    unsigned int i;

    // compute output in blocks of size 'n'
    for (i=0; i<num_blocks; i++)
        fftfilt_rrrf_execute(q, &_x[i*n], &y_test[i*n]);

    // compare results
    for (i=0; i<_y_len; i++)
        CONTEND_DELTA( y_test[i], _y[i], tol );
    
    // destroy filter object
    fftfilt_rrrf_destroy(q);
}

// autotest helper function
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void fftfilt_crcf_test(float *         _h,
                       unsigned int    _h_len,
                       float complex * _x,
                       unsigned int    _x_len,
                       float complex * _y,
                       unsigned int    _y_len)
{
    float tol = 0.001f;

    // determine appropriate block size
    // NOTE: this number can be anything at least _h_len-1
    unsigned int n = 1 << liquid_nextpow2(_h_len-1);

    // determine number of blocks
    div_t d = div(_x_len, n);
    unsigned int num_blocks = d.quot + (d.rem ? 1 : 0);
    if (liquid_autotest_verbose) {
        printf("fftfilt_crcf_test(), h_len: %3u, x_len: %3u (%3u blocks @ %3u samples, %3u remaining)\n",
                _h_len, _x_len, n, d.quot, d.rem);
    }

    // load filter coefficients externally
    fftfilt_crcf q = fftfilt_crcf_create(_h, _h_len, n);

    // allocate memory for output
    float complex y_test[n*num_blocks];

    unsigned int i;

    // compute output in blocks of size 'n'
    for (i=0; i<num_blocks; i++)
        fftfilt_crcf_execute(q, &_x[i*n], &y_test[i*n]);

    // compare results
    for (i=0; i<_y_len; i++) {
        CONTEND_DELTA( crealf(y_test[i]), crealf(_y[i]), tol );
        CONTEND_DELTA( cimagf(y_test[i]), cimagf(_y[i]), tol );
    }
    
    // destroy filter object
    fftfilt_crcf_destroy(q);
}

// autotest helper function
//  _h      :   filter coefficients
//  _h_len  :   filter coefficients length
//  _x      :   input array
//  _x_len  :   input array length
//  _y      :   output array
//  _y_len  :   output array length
void fftfilt_cccf_test(float complex * _h,
                       unsigned int    _h_len,
                       float complex * _x,
                       unsigned int    _x_len,
                       float complex * _y,
                       unsigned int    _y_len)
{
    float tol = 0.001f;

    // determine appropriate block size
    // NOTE: this number can be anything at least _h_len-1
    unsigned int n = 1 << liquid_nextpow2(_h_len-1);

    // determine number of blocks
    div_t d = div(_x_len, n);
    unsigned int num_blocks = d.quot + (d.rem ? 1 : 0);
    if (liquid_autotest_verbose) {
        printf("fftfilt_cccf_test(), h_len: %3u, x_len: %3u (%3u blocks @ %3u samples, %3u remaining)\n",
                _h_len, _x_len, n, d.quot, d.rem);
    }

    // load filter coefficients externally
    fftfilt_cccf q = fftfilt_cccf_create(_h, _h_len, n);

    // allocate memory for output
    float complex y_test[n*num_blocks];

    unsigned int i;

    // compute output in blocks of size 'n'
    for (i=0; i<num_blocks; i++)
        fftfilt_cccf_execute(q, &_x[i*n], &y_test[i*n]);

    // compare results
    for (i=0; i<_y_len; i++) {
        CONTEND_DELTA( crealf(y_test[i]), crealf(_y[i]), tol );
        CONTEND_DELTA( cimagf(y_test[i]), cimagf(_y[i]), tol );
    }
    
    // destroy filter object
    fftfilt_cccf_destroy(q);
}

