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

//
// windowed delay, defined by macro
//

#include "liquid.internal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct WDELAY(_s) {
    T *          v;             // allocated array pointer
    unsigned int delay;         // length of window
    unsigned int read_index;    // index for reading
};

// create delay buffer object with '_delay' samples
WDELAY() WDELAY(_create)(unsigned int _delay)
{
    // create main object
    WDELAY() q = (WDELAY()) malloc(sizeof(struct WDELAY(_s)));

    // set internal values
    q->delay = _delay;

    // allocate memory
    q->v = (T*) malloc((q->delay+1)*sizeof(T));
    q->read_index = 0;

    // clear window
    WDELAY(_reset)(q);

    return q;
}

// re-create delay buffer object with '_delay' samples
//  _q      :   old delay buffer object
//  _delay  :   delay for new object
WDELAY() WDELAY(_recreate)(WDELAY()     _q,
                           unsigned int _delay)
{
    // copy internal buffer, re-aligned
    unsigned int ktmp = _q->delay+1;
    T * vtmp = (T*) malloc((_q->delay+1) * sizeof(T));
    unsigned int i;
    for (i=0; i<_q->delay+1; i++)
        vtmp[i] = _q->v[ (i + _q->read_index) % (_q->delay+1) ];
    
    // destroy object and re-create it
    WDELAY(_destroy)(_q);
    _q = WDELAY(_create)(_delay);

    // push old values
    for (i=0; i<ktmp; i++)
        WDELAY(_push)(_q, vtmp[i]);

    // free temporary array
    free(vtmp);

    // return object
    return _q;
}

// copy object
WDELAY() WDELAY(_copy)(WDELAY() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("error: cbuffer%s_copy(), window object cannot be NULL", EXTENSION);

    // create initial object
    WDELAY() q_copy = (WDELAY()) malloc(sizeof(struct WDELAY(_s)));
    memmove(q_copy, q_orig, sizeof(struct WDELAY(_s)));

    // allocate and copy full memory array
    q_copy->v = (T*) malloc((q_copy->delay+1)*sizeof(T));
    memmove(q_copy->v, q_orig->v, (q_copy->delay+1)*sizeof(T));

    // return new object
    return q_copy;
}

// destroy delay buffer object, freeing internal memory
int WDELAY(_destroy)(WDELAY() _q)
{
    // free internal array buffer
    free(_q->v);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print delay buffer object's state to stdout
int WDELAY(_print)(WDELAY() _q)
{
    printf("<liquid.wdelay, delay=%u>\n", _q->delay);
    return LIQUID_OK;
}

// clear/reset state of object
int WDELAY(_reset)(WDELAY() _q)
{
    _q->read_index = 0;
    memset(_q->v, 0, (_q->delay+1)*sizeof(T));
    return LIQUID_OK;
}

// read delayed sample from delay buffer object
//  _q  :   delay buffer object
//  _v  :   value of delayed element
int WDELAY(_read)(WDELAY() _q,
                  T *      _v)
{
    // return value at end of buffer
    *_v = _q->v[_q->read_index];
    return LIQUID_OK;
}

// push new sample into delay buffer object
//  _q  :   delay buffer object
//  _v  :   new value to be added to buffer
int WDELAY(_push)(WDELAY() _q,
                  T        _v)
{
    // append value to end of buffer
    _q->v[_q->read_index] = _v;

    // increment index
    _q->read_index++;

    // wrap around pointer
    _q->read_index %= (_q->delay+1);
    return LIQUID_OK;
}

