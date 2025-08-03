/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
// ordfilt : order-statistics filter
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// defined:
//  ORDFILT()       name-mangling macro
//  T               coefficients type
//  WINDOW()        window macro
//  DOTPROD()       dotprod macro
//  PRINTVAL()      print macro

#define LIQUID_ORDFILT_USE_WINDOW 1

#if LIQUID_ORDFILT_USE_WINDOW
int ordfilt_sort_compf(const void * _v1, const void * _v2)
{
    return *(float*)_v1 > *(float*)_v2 ? 1 : -1;
}
#endif

// ordfilt object structure
struct ORDFILT(_s) {
    unsigned int    n;          // buffer length
    unsigned int    k;          // sample index of order statistic
#if LIQUID_ORDFILT_USE_WINDOW
    // simple to implement but much slower
    WINDOW()        buf;        // input buffer
    TI *            buf_sorted; // input buffer (sorted)
#else
    // trickier to implement but faster
    TI *            buf;        // input buffer
    uint16_t *      buf_index;  // buffer index of sorted values
#endif
};

// Create a order-statistic filter (ordfilt) object by specifying
// the buffer size and appropriate sample index of order statistic.
//  _n      : buffer size
//  _k      : sample index for order statistic, 0 <= _k < _n
ORDFILT() ORDFILT(_create)(unsigned int _n,
                           unsigned int _k)
{
    // validate input
    if (_n == 0)
        return liquid_error_config("ordfilt_%s_create(), filter length must be greater than zero", EXTENSION_FULL);
    if (_k >= _n)
        return liquid_error_config("ordfilt_%s_create(), filter index must be in [0,n-1]", EXTENSION_FULL);

    // create filter object and initialize
    ORDFILT() q = (ORDFILT()) malloc(sizeof(struct ORDFILT(_s)));
    q->n = _n;
    q->k = _k;

#if LIQUID_ORDFILT_USE_WINDOW
    // create window (internal buffer)
    q->buf        = WINDOW(_create)(q->n);
    q->buf_sorted = (TI*) malloc(q->n * sizeof(TI));
#else
#endif

    // reset filter state (clear buffer)
    ORDFILT(_reset)(q);
    return q;
}

// Create a median filter by specifying buffer semi-length.
//  _m      : buffer semi-length
ORDFILT() ORDFILT(_create_medfilt)(unsigned int _m)
{
    return ORDFILT(_create)(2*_m+1, _m);
}

// copy object
ORDFILT() ORDFILT(_copy)(ORDFILT() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("ordfilt_%s_copy(), object cannot be NULL", EXTENSION_FULL);

    // create filter object and copy base parameters
    ORDFILT() q_copy = (ORDFILT()) malloc(sizeof(struct ORDFILT(_s)));
    memmove(q_copy, q_orig, sizeof(struct ORDFILT(_s)));

#if LIQUID_ORDFILT_USE_WINDOW
    // copy window
    q_copy->buf = WINDOW(_copy)(q_orig->buf);
    q_copy->buf_sorted = (TI*) liquid_malloc_copy(q_orig->buf_sorted, q_orig->n, sizeof(TI));
#else
    // copy buffers
    q_copy->buf_      = (TI*)       liquid_malloc_copy(q_orig->buf,       q_orig->n, sizeof(TI));
    q_copy->buf_index = (uint16_t*) liquid_malloc_copy(q_orig->buf_index, q_orig->n, sizeof(uint16_t));
#endif

    return q_copy;
}

// destroy ordfilt object
int ORDFILT(_destroy)(ORDFILT() _q)
{
#if LIQUID_ORDFILT_USE_WINDOW
    WINDOW(_destroy)(_q->buf);
    free(_q->buf_sorted);
#else
#endif
    free(_q);
    return LIQUID_OK;
}

// reset internal state of filter object
int ORDFILT(_reset)(ORDFILT() _q)
{
#if LIQUID_ORDFILT_USE_WINDOW
    return WINDOW(_reset)(_q->buf);
#else
#endif
    return LIQUID_OK;
}

// print filter object internals (taps, buffer)
int ORDFILT(_print)(ORDFILT() _q)
{
    printf("<liquid.ordfilt_%s, len=%u, order=%u>\n", EXTENSION_FULL, _q->n, _q->k);
    return LIQUID_OK;
}

// push sample into filter object's internal buffer
//  _q      :   filter object
//  _x      :   input sample
int ORDFILT(_push)(ORDFILT() _q,
                   TI        _x)
{
#if LIQUID_ORDFILT_USE_WINDOW
    return WINDOW(_push)(_q->buf, _x);
#else
#endif
    return LIQUID_OK;
}

// Write block of samples into object's internal buffer
//  _q      : filter object
//  _x      : array of input samples, [size: _n x 1]
//  _n      : number of input elements
int ORDFILT(_write)(ORDFILT()    _q,
                    TI *         _x,
                    unsigned int _n)
{
#if LIQUID_ORDFILT_USE_WINDOW
    return WINDOW(_write)(_q->buf, _x, _n);
#else
#endif
    return LIQUID_OK;
}

// Execute on the filter's internal buffer
//  _q      :   filter object
//  _y      :   output sample pointer
int ORDFILT(_execute)(ORDFILT() _q,
                      TO *      _y)
{
#if LIQUID_ORDFILT_USE_WINDOW
    // read buffer (retrieve pointer to aligned memory array)
    TI *r;
    WINDOW(_read)(_q->buf, &r);

    // copy to buffer and sort
    memmove(_q->buf_sorted, r, _q->n*sizeof(TI));

    // sort results
    qsort((void*)_q->buf_sorted, _q->n, sizeof(TI), &ordfilt_sort_compf);

    // save output
    *_y = _q->buf_sorted[_q->k];
#else
#endif
    return LIQUID_OK;
}

// Execute filter on one sample, equivalent to push() and execute()
//  _q      : filter object
//  _x      : single input sample
//  _y      : pointer to single output sample
int ORDFILT(_execute_one)(ORDFILT() _q,
                          TI        _x,
                          TO *      _y)
{
    ORDFILT(_push)(_q, _x);
    return ORDFILT(_execute)(_q, _y);
}

// execute the filter on a block of input samples; the
// input and output buffers may be the same
//  _q      : filter object
//  _x      : pointer to input array [size: _n x 1]
//  _n      : number of input, output samples
//  _y      : pointer to output array [size: _n x 1]
int ORDFILT(_execute_block)(ORDFILT()    _q,
                            TI *         _x,
                            unsigned int _n,
                            TO *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        // push sample into filter
        ORDFILT(_push)(_q, _x[i]);

        // compute output sample
        ORDFILT(_execute)(_q, &_y[i]);
    }
    return LIQUID_OK;
}

