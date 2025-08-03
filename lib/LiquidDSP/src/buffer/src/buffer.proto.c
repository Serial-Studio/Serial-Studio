/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// Buffers, defined by macro
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "liquid.internal.h"

struct BUFFER(_s) {
    T * v;
    unsigned int len;           // length of buffer
    unsigned int N;             // number of elements allocated
                                // in memory
    unsigned int num_elements;  // number of elements currently
                                // in the buffer

    unsigned int read_index;
    unsigned int write_index;

    buffer_type type;

    // mutex/semaphore
};

BUFFER() BUFFER(_create)(buffer_type _type, unsigned int _n)
{
    BUFFER() b = (BUFFER()) malloc(sizeof(struct BUFFER(_s)));
    b->type = _type;
    b->len = _n;

    if (b->type == CIRCULAR)
        b->N = 2*(b->len) - 1;
    else
        b->N = b->len;

    b->v = (T*) malloc((b->N)*sizeof(T));
    b->num_elements = 0;
    b->read_index = 0;
    b->write_index = 0;

    return b;
}

void BUFFER(_destroy)(BUFFER() _b)
{
    free(_b->v);
    free(_b);
}

void BUFFER(_print)(BUFFER() _b)
{
    if (_b->type == CIRCULAR)
        printf("circular ");
    else
        printf("static ");
    printf("buffer [%u elements] :\n", _b->num_elements);
    unsigned int i;
    for (i=0; i<_b->num_elements; i++) {
        printf("%u", i);
        BUFFER_PRINT_LINE(_b,(_b->read_index+i)%(_b->len))
        printf("\n");
    }
}

void BUFFER(_debug_print)(BUFFER() _b)
{
    if (_b->type == CIRCULAR)
        printf("circular ");
    else
        printf("static ");
    printf("buffer [%u elements] :\n", _b->num_elements);
    unsigned int i;
    for (i=0; i<_b->len; i++) {
        // print read index pointer
        if (i==_b->read_index)
            printf("<r>");

        // print write index pointer
        if (i==_b->write_index)
            printf("<w>");

        // print buffer value
        BUFFER_PRINT_LINE(_b,i)
        printf("\n");
    }
    printf("----------------------------------\n");

    // print excess buffer memory
    for (i=_b->len; i<_b->N; i++) {
        BUFFER_PRINT_LINE(_b,i)
        printf("\n");
    }
}

void BUFFER(_clear)(BUFFER() _b)
{
    _b->read_index = 0;
    _b->write_index = 0;
    _b->num_elements = 0;
}

void BUFFER(_zero)(BUFFER() _b)
{
    _b->read_index = 0;
    _b->write_index = 0;
    _b->num_elements = _b->len;
    memset(_b->v, 0, (_b->num_elements)*sizeof(T));
}

void BUFFER(_read)(BUFFER() _b, T ** _v, unsigned int *_n)
{
    if (_b->type == CIRCULAR)
        BUFFER(_c_read)(_b, _v, _n);
    else
        BUFFER(_s_read)(_b, _v, _n);
}

void BUFFER(_c_read)(BUFFER() _b, T ** _v, unsigned int *_n)
{
    //printf("buffer_read() trying to read %u elements (%u available)\n", *_n, _b->num_elements);
    if (*_n > _b->num_elements) {
        liquid_error(LIQUID_EIRANGE,"buffer%s_read(), cannot read more elements than are available", EXTENSION);
        *_v = NULL;
        *_n = 0;
        return;
    } else
    if (*_n > (_b->len - _b->read_index)) {
        //
        BUFFER(_linearize)(_b);
    }
    *_v = _b->v + _b->read_index;
    *_n = _b->num_elements;
}

void BUFFER(_s_read)(BUFFER() _b, T ** _v, unsigned int *_n)
{
    //printf("buffer_s_read() reading %u elements\n", _b->num_elements);
    *_v = _b->v;
    *_n = _b->num_elements;
}

void BUFFER(_release)(BUFFER() _b, unsigned int _n)
{
    if (_b->type == CIRCULAR)
        BUFFER(_c_release)(_b, _n);
    else
        BUFFER(_s_release)(_b, _n);
}


void BUFFER(_c_release)(BUFFER() _b, unsigned int _n)
{
    // advance read_index by _n making sure not to step on write_index
    if (_n > _b->num_elements) {
        liquid_error(LIQUID_EIRANGE,"buffer%s_c_release(), cannot release more elements in buffer than exist", EXTENSION);
        return;
    }

    _b->read_index = (_b->read_index + _n) % _b->len;
    _b->num_elements -= _n;
}


void BUFFER(_s_release)(BUFFER() _b, unsigned int _n)
{
    BUFFER(_clear)(_b);
}

void BUFFER(_write)(BUFFER() _b, T * _v, unsigned int _n)
{
    if (_b->type == CIRCULAR)
        BUFFER(_c_write)(_b, _v, _n);
    else
        BUFFER(_s_write)(_b, _v, _n);
}

void BUFFER(_c_write)(BUFFER() _b, T * _v, unsigned int _n)
{
    //
    if (_n > (_b->len - _b->num_elements)) {
        liquid_error(LIQUID_EIRANGE,"buffer%s_write(), cannot write more elements than are available", EXTENSION);
        return;
    }

    _b->num_elements += _n;
    // space available at end of buffer
    unsigned int k = _b->len - _b->write_index;
    //printf("n : %u, k : %u\n", _n, k);

    // check for condition where we need to wrap around
    if (_n > k) {
        memcpy(_b->v + _b->write_index, _v, k*sizeof(T));
        memcpy(_b->v, &_v[k], (_n-k)*sizeof(T));
        _b->write_index = _n - k;
    } else {
        memcpy(_b->v + _b->write_index, _v, _n*sizeof(T));
        _b->write_index += _n;
    }
}

void BUFFER(_s_write)(BUFFER() _b, T * _v, unsigned int _n)
{
    if (_n > (_b->len - _b->num_elements)) {
        liquid_error(LIQUID_EIRANGE,"buffer%s_s_write(), cannot write more elements than are available", EXTENSION);
        return;
    }

    memcpy(_b->v + _b->num_elements, _v, _n*sizeof(T));
    _b->num_elements += _n;
}

//void BUFFER(_force_write)(BUFFER() _b, T * _v, unsigned int _n)

void BUFFER(_push)(BUFFER() _b, T _v)
{
    // push value (force write)
    if (_b->type == CIRCULAR)
        BUFFER(_c_push)(_b, _v);
    else
        BUFFER(_s_push)(_b, _v);
}

void BUFFER(_c_push)(BUFFER() _b, T _v)
{
    _b->v[_b->write_index] = _v;
    if (_b->num_elements < _b->len) {
        _b->num_elements++;
    } else {
        _b->read_index = (_b->read_index+1) % _b->len;
    }
    _b->write_index = (_b->write_index+1) % _b->len;
}

void BUFFER(_s_push)(BUFFER() _b, T _v)
{

}

void BUFFER(_linearize)(BUFFER() _b)
{
    // check to see if anything needs to be done
    if ( (_b->len - _b->read_index) > _b->num_elements)
        return;

    // perform memory copy
    memcpy(_b->v + _b->len, _b->v, (_b->write_index)*sizeof(T));
}

