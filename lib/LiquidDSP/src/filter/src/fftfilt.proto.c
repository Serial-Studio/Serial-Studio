/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
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

// finite impulse response (FIR) filter using fast Fourier transforms (FFTs)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// fftfilt object structure
struct FFTFILT(_s) {
    TC *         h;     // filter coefficients array [size; h_len x 1]
    unsigned int h_len; // filter length
    unsigned int n;     // input/output block size

    // internal memory arrays
    // TODO: make TI/TO type, but ensuring complex
    // TODO: use special format for fftfilt_rrrf type
    float complex * time_buf;   // time buffer [size: 2*n x 1]
    float complex * freq_buf;   // freq buffer [size: 2*n x 1]
    float complex * H;          // FFT of filter coefficients [size: 2*n x 1]
    float complex * w;          // overlap array [size: n x 1]

    // FFT objects
    FFT_PLAN fft;       // FFT object (forward)
    FFT_PLAN ifft;      // FFT object (inverse)
    TC scale;           // output scaling factor
};

// create FFT-based FIR filter using external coefficients
//  _h      : filter coefficients [size: _h_len x 1]
//  _h_len  : filter length, _h_len > 0
//  _n      : block size = nfft/2, at least _h_len-1
FFTFILT() FFTFILT(_create)(TC *         _h,
                           unsigned int _h_len,
                           unsigned int _n)
{
    // validate input
    if (_h_len == 0)
        return liquid_error_config("fftfilt_%s_create(), filter length must be greater than zero",EXTENSION_FULL);
    if (_n < _h_len-1)
        return liquid_error_config("fftfilt_%s_create(), block length must be greater than _h_len-1 (%u)",EXTENSION_FULL,_h_len-1);

    // create filter object and initialize
    FFTFILT() q = (FFTFILT()) malloc(sizeof(struct FFTFILT(_s)));
    q->h_len    = _h_len;
    q->n        = _n;

    // copy filter coefficients
    q->h = (TC *) malloc((q->h_len)*sizeof(TC));
    memmove(q->h, _h, _h_len*sizeof(TC));

    // allocate internal memory arrays
    q->time_buf = (float complex *) FFT_MALLOC((2*q->n)* sizeof(float complex)); // time buffer
    q->freq_buf = (float complex *) FFT_MALLOC((2*q->n)* sizeof(float complex)); // frequency buffer
    q->H        = (float complex *) malloc((2*q->n)* sizeof(float complex));     // FFT{ h }
    q->w        = (float complex *) malloc((  q->n)* sizeof(float complex));     // delay buffer

    // create internal FFT objects
    q->fft  = FFT_CREATE_PLAN(2*q->n, q->time_buf, q->freq_buf, FFT_DIR_FORWARD,  FFT_METHOD);
    q->ifft = FFT_CREATE_PLAN(2*q->n, q->freq_buf, q->time_buf, FFT_DIR_BACKWARD, FFT_METHOD);

    // compute FFT of filter coefficients and copy to internal H array
    unsigned int i;
    for (i=0; i<2*q->n; i++)
        q->time_buf[i] = (i < q->h_len) ? q->h[i] : 0;
    // time_buf > {FFT} > freq_buf
    FFT_EXECUTE(q->fft);
    memmove(q->H, q->freq_buf, 2*q->n*sizeof(float complex));

    // set default scaling
    FFTFILT(_set_scale)(q, 1);

    // reset filter state (clear buffer)
    FFTFILT(_reset)(q);

    // return object
    return q;
}

// copy object
FFTFILT() FFTFILT(_copy)(FFTFILT() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("firfilt_%s_copy(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy base parameters
    FFTFILT() q_copy = (FFTFILT()) malloc(sizeof(struct FFTFILT(_s)));
    memmove(q_copy, q_orig, sizeof(struct FFTFILT(_s)));

    // copy filter coefficients
    q_copy->h = (TC *) liquid_malloc_copy(q_orig->h, q_orig->h_len, sizeof(TC));

    // allocate FFT buffers
    q_copy->time_buf = (float complex*) FFT_MALLOC((2*q_orig->n) * sizeof(float complex));
    q_copy->freq_buf = (float complex*) FFT_MALLOC((2*q_orig->n) * sizeof(float complex));

    // copy buffers
    memmove(q_copy->time_buf, q_orig->time_buf, (2*q_orig->n) * sizeof(float complex));
    memmove(q_copy->freq_buf, q_orig->freq_buf, (2*q_orig->n) * sizeof(float complex));
    q_copy->H = (float complex*) liquid_malloc_copy(q_orig->H, 2*q_orig->n, sizeof(float complex));
    q_copy->w = (float complex*) liquid_malloc_copy(q_orig->w,   q_orig->n, sizeof(float complex));

    // create internal FFT objects and return
    q_copy->fft  = FFT_CREATE_PLAN(2*q_copy->n, q_copy->time_buf, q_copy->freq_buf, FFT_DIR_FORWARD,  FFT_METHOD);
    q_copy->ifft = FFT_CREATE_PLAN(2*q_copy->n, q_copy->freq_buf, q_copy->time_buf, FFT_DIR_BACKWARD, FFT_METHOD);
    return q_copy;
}

// destroy object, freeing all internally-allocated memory
int FFTFILT(_destroy)(FFTFILT() _q)
{
    // free internal arrays
    free(_q->h);                // filter coefficients
    FFT_FREE(_q->time_buf);     // buffer (time domain)
    FFT_FREE(_q->freq_buf);     // buffer (frequency domain)
    free(_q->H);                // frequency response of filter coefficients
    free(_q->w);                // output window buffer

    // destroy FFT objects
    FFT_DESTROY_PLAN(_q->fft);  // forward transform
    FFT_DESTROY_PLAN(_q->ifft); // reverse transform

    // free main object
    free(_q);
    return LIQUID_OK;
}

// reset internal state of filter object
int FFTFILT(_reset)(FFTFILT() _q)
{
    // reset overlap window
    unsigned int i;
    for (i=0; i<_q->n; i++) {
        _q->w[i] = 0;
    }
    return LIQUID_OK;
}

// print filter object internals (taps, buffer)
int FFTFILT(_print)(FFTFILT() _q)
{
    printf("<liquid.fftfilt_%s, len=%u, nfft=%u",
        EXTENSION_FULL, _q->h_len, _q->n);

    printf(", scale=");
    PRINTVAL_TC(_q->scale,%g);
    printf(">\n");
    return LIQUID_OK;
}

// set output scaling for filter
int FFTFILT(_set_scale)(FFTFILT() _q,
                         TC        _scale)
{
    // set scale, normalized by fft size
    _q->scale = _scale / (float)(2*_q->n);
    return LIQUID_OK;
}

// get output scaling for filter
int FFTFILT(_get_scale)(FFTFILT() _q,
                         TC *      _scale)
{
    // get scale, normalized by fft size
    *_scale = _q->scale * (float)(2*_q->n);
    return LIQUID_OK;
}

// execute the filter on internal buffer and coefficients
//  _q      : filter object
//  _x      : pointer to input data array  [size: _n x 1]
//  _y      : pointer to output data array [size: _n x 1]
int FFTFILT(_execute)(FFTFILT() _q,
                      TI *      _x,
                      TO *      _y)
{
    unsigned int i;

    // copy input
#if 0 //TI_COMPLEX
    memmove(_q->time_buf, _x, _q->n*sizeof(TI));
#else
    // manual copy for type conversion
    // TODO: use DCT or equivalent
    for (i=0; i<_q->n; i++)
        _q->time_buf[i] = _x[i];
#endif

    // pad end of time-domain buffer with zeros
    // TODO: not necessary to do this every time
#if 0
    memset(&_q->time_buf[_q->n], 0, _q->n*sizeof(TI));
#else
    for (i=0; i<_q->n; i++)
        _q->time_buf[_q->n + i] = 0;
#endif

    // run forward transform
    FFT_EXECUTE(_q->fft);

    // compute inner product between FFT{ _x } and FFT{ H }
#if 1
    for (i=0; i<2*_q->n; i++)
        _q->freq_buf[i] *= _q->H[i];
#else
    // use SIMD vector extensions
# if TI_COMPLEX
    // complex floating-point inner product
    liquid_vectorcf_mul(_q->freq_buf, _q->H, 2*_q->n, _q->freq_buf);
# else
    // real floating-point inner product
    liquid_vectorf_mul(_q->freq_buf, _q->H, 2*_q->n, _q->freq_buf);
# endif
#endif

    // compute inverse transform
    FFT_EXECUTE(_q->ifft);

    // copy output summed with buffer and scaled
#if TI_COMPLEX
    for (i=0; i<_q->n; i++)
        _y[i] = (_q->time_buf[i] + _q->w[i]) * _q->scale;
#else
    // manual copy for type conversion
    // TODO: use DCT or equivalent
    for (i=0; i<_q->n; i++)
        _y[i] = (T) crealf(_q->time_buf[i] + _q->w[i]) * _q->scale;
#endif

    // copy buffer
    memmove(_q->w, &_q->time_buf[_q->n], _q->n*sizeof(float complex));
    return LIQUID_OK;
}

// return length of filter object's internal coefficients
unsigned int FFTFILT(_get_length)(FFTFILT() _q)
{
    return _q->h_len;
}

