/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
// shift_array.c
//
// byte-wise array shifting
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"


// shift array to the left _b bytes, filling in zeros
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_lshift(unsigned char * _src,
                  unsigned int    _n,
                  unsigned int    _b)
{
    // shift amount exceeds buffer size; fill with zeros
    if (_b >= _n) {
        memset(_src, 0x00, _n*sizeof(unsigned char));
        return LIQUID_OK;
    }

    // move memory
    memmove(_src, &_src[_b], (_n-_b)*sizeof(unsigned char));

    // fill remaining buffer with zeros
    memset(&_src[_n-_b], 0x00, _b*sizeof(unsigned char));
    return LIQUID_OK;
}
 
// shift array to the right _b bytes, filling in zeros
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_rshift(unsigned char * _src,
                  unsigned int    _n,
                  unsigned int    _b)
{
    // shift amount exceeds buffer size; fill with zeros
    if (_b >= _n) {
        memset(_src, 0x00, _n*sizeof(unsigned char));
        return LIQUID_OK;
    }

    // move memory
    memmove(&_src[_b], _src, (_n-_b)*sizeof(unsigned char));

    // fill remaining buffer with zeros
    memset(_src, 0x00, _b*sizeof(unsigned char));
    return LIQUID_OK;
}
 
 
// circular shift array to the left _n bytes
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_lcircshift(unsigned char * _src,
                      unsigned int    _n,
                      unsigned int    _b)
{
    // validate input
    if (_n == 0)
        return LIQUID_OK;

    // ensure 0 <= _b < _n
    _b = _b % _n;

    // check if less memory is used with rcircshift
    if (_b > (_n>>1))
        return liquid_rcircshift(_src, _n, _n-_b);

    // allocate memory for temporary array
    unsigned char * tmp = (unsigned char*) malloc(_b*sizeof(unsigned char));

    // copy to temporary array
    memmove(tmp, _src, _b*sizeof(unsigned char));

    // shift left
    memmove(_src, &_src[_b], (_n-_b)*sizeof(unsigned char));

    // copy from temporary array
    memmove(&_src[_n-_b], tmp, _b*sizeof(unsigned char));

    // free temporary array
    free(tmp);
    return LIQUID_OK;
}


// circular shift array to the right _n bytes
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bytes to shift
int liquid_rcircshift(unsigned char * _src,
                      unsigned int    _n,
                      unsigned int    _b)
{
    // validate input
    if (_n == 0)
        return LIQUID_OK;

    // ensure 0 <= _b < _n
    _b = _b % _n;

    // check if less memory is used with lcircshift
    if (_b > (_n>>1))
        return liquid_lcircshift(_src, _n, _n-_b);

    // allocate memory for temporary array
    unsigned char * tmp = (unsigned char*) malloc(_b*sizeof(unsigned char));

    // copy to temporary array
    memmove(tmp, &_src[_n-_b], _b*sizeof(unsigned char));

    // shift right
    memmove(&_src[_b], _src, (_n-_b)*sizeof(unsigned char));

    // copy from temporary array
    memmove(_src, tmp, _b*sizeof(unsigned char));

    // free temporary array
    free(tmp);
    return LIQUID_OK;
}
 
