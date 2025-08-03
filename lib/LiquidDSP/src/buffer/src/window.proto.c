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
// circular window buffer
//

#include "liquid.internal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct WINDOW(_s) {
    T * v;                      // allocated array pointer
    unsigned int len;           // length of window
    unsigned int m;             // floor(log2(len)) + 1
    unsigned int n;             // 2^m
    unsigned int mask;          // n-1
    unsigned int num_allocated; // number of elements allocated
                                // in memory
    unsigned int read_index;
};

// create window buffer object of length _n
WINDOW() WINDOW(_create)(unsigned int _n)
{
    // validate input
    if (_n == 0)
        return liquid_error_config("error: window%s_create(), window size must be greater than zero", EXTENSION);

    // create initial object
    WINDOW() q = (WINDOW()) malloc(sizeof(struct WINDOW(_s)));

    // set internal parameters
    q->len  = _n;                   // nominal window size
    q->m    = liquid_msb_index(_n); // effectively floor(log2(len))+1
    q->n    = 1<<(q->m);            // 2^m
    q->mask = q->n - 1;             // bit mask

    // number of elements to allocate to memory
    q->num_allocated = q->n + q->len - 1;

    // allocate memory
    q->v = (T*) malloc((q->num_allocated)*sizeof(T));
    q->read_index = 0;

    // reset window
    WINDOW(_reset)(q);

    // return object
    return q;
}

// recreate window buffer object with new length
//  _q      : old window object
//  _n      : new window length
WINDOW() WINDOW(_recreate)(WINDOW() _q, unsigned int _n)
{
    // TODO: only create new window if old is too small

    if (_n == _q->len)
        return _q;

    // create new window
    WINDOW() w = WINDOW(_create)(_n);

    // copy old values
    T* r;
    WINDOW(_read)(_q, &r);
    //memmove(q->v, ...);
    unsigned int i;
    if (_n > _q->len) {
        // new buffer is larger; push zeros, then old values
        for (i=0; i<(_n-_q->len); i++)
            WINDOW(_push)(w, 0);
        for (i=0; i<_q->len; i++)
            WINDOW(_push)(w, r[i]);
    } else {
        // new buffer is shorter; push latest old values
        for (i=(_q->len-_n); i<_q->len; i++)
            WINDOW(_push)(w, r[i]);
    }

    // destroy old window
    WINDOW(_destroy)(_q);

    return w;
}

// copy object
WINDOW() WINDOW(_copy)(WINDOW() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("error: window%s_copy(), window object cannot be NULL", EXTENSION);

    // create object, copy memory, create new elements as needed
    WINDOW() q_copy = (WINDOW()) malloc(sizeof(struct WINDOW(_s)));
    memmove(q_copy, q_orig, sizeof(struct WINDOW(_s)));

    // allocate and copy full memory array
    q_copy->v = (T*) liquid_malloc_copy(q_copy->v, q_copy->num_allocated, sizeof(T));

    // return new object
    return q_copy;
}

// destroy window object, freeing all internally memory
int WINDOW(_destroy)(WINDOW() _q)
{
    // free internal memory array
    free(_q->v);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print window object to stdout
int WINDOW(_print)(WINDOW() _q)
{
    printf("window [%u elements] :\n", _q->len);
    unsigned int i;
    T * r;
    WINDOW(_read)(_q, &r);
    for (i=0; i<_q->len; i++) {
        printf("%4u", i);
        BUFFER_PRINT_VALUE(r[i]);
        printf("\n");
    }
    return LIQUID_OK;
}

// print window object to stdout (with extra information)
int WINDOW(_debug_print)(WINDOW() _q)
{
    printf("window [%u elements] :\n", _q->len);
    unsigned int i;
    for (i=0; i<_q->len; i++) {
        // print read index pointer
        if (i==_q->read_index)
            printf("<r>");

        // print window value
        BUFFER_PRINT_LINE(_q,i)
        printf("\n");
    }
    printf("----------------------------------\n");

    // print excess window memory
    for (i=_q->len; i<_q->num_allocated; i++) {
        BUFFER_PRINT_LINE(_q,i)
        printf("\n");
    }
    return LIQUID_OK;
}

// reset window object (initialize to zeros)
int WINDOW(_reset)(WINDOW() _q)
{
    // reset read index
    _q->read_index = 0;

    // clear all allocated memory
    memset(_q->v, 0, (_q->num_allocated)*sizeof(T));
    return LIQUID_OK;
}

// read window buffer contents
//  _q      : window object
//  _v      : output pointer (set to internal array)
int WINDOW(_read)(WINDOW() _q, T ** _v)
{
    // return pointer to buffer
    *_v = _q->v + _q->read_index;
    return LIQUID_OK;
}

// index single element in buffer at a particular index
//  _q      : window object
//  _i      : index of element to read
//  _v      : output value pointer
int WINDOW(_index)(WINDOW()     _q,
                   unsigned int _i,
                   T *          _v)
{
    // validate input
    if (_i >= _q->len)
        return liquid_error(LIQUID_EIRANGE,"error: window_index(), index value out of range");

    // return value at index
    *_v = _q->v[_q->read_index + _i];
    return LIQUID_OK;
}

// push single element onto window buffer
//  _q      : window object
//  _v      : single input element
int WINDOW(_push)(WINDOW() _q, T _v)
{
    // increment index
    _q->read_index++;

    // wrap around pointer
    _q->read_index &= _q->mask;

    // if pointer wraps around, copy excess memory
    if (_q->read_index == 0)
        memmove(_q->v, _q->v + _q->n, (_q->len-1)*sizeof(T));

    // append value to end of buffer
    _q->v[_q->read_index + _q->len - 1] = _v;
    return LIQUID_OK;
}

// write array of elements onto window buffer
//  _q      : window object
//  _v      : input array of values to write
//  _n      : number of input values to write
int WINDOW(_write)(WINDOW()     _q,
                   T *          _v,
                   unsigned int _n)
{
    // TODO make this more efficient
    unsigned int i;
    for (i=0; i<_n; i++)
        WINDOW(_push)(_q, _v[i]);
    return LIQUID_OK;
}

