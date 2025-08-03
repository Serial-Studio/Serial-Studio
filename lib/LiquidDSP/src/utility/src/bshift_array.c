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
// bshift_array.c
//
// bit-wise array shifting
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

 
// shift array to the left _b bits, filling in zeros
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_lbshift(unsigned char * _src,
                   unsigned int    _n,
                   unsigned int    _b)
{
    // validate input
    if (_b >= 8)
        return liquid_error(LIQUID_EIRANGE,"liquid_lbshift(), shift amount must be in [0,7]");

    // invoke circular shift left and mask last byte
    liquid_lbcircshift(_src, _n, _b);
    _src[_n-1] &= 0xff << _b;
    return LIQUID_OK;
}

 
// shift array to the right _b bits filling in zeros
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_rbshift(unsigned char * _src,
                   unsigned int    _n,
                   unsigned int    _b)
{
    // validate input
    if (_b >= 8)
        return liquid_error(LIQUID_EIRANGE,"liquid_rbshift(), shift amount must be in [0,7]");

    // invoke circular shift right and mask first byte
    liquid_rbcircshift(_src, _n, _b);
    _src[0] &= 0xff >> _b;
    return LIQUID_OK;
}

 
// circular shift array to the left _b bits
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_lbcircshift(unsigned char * _src,
                       unsigned int    _n,
                       unsigned int    _b)
{
    // validate input
    if (_b >= 8)
        return liquid_error(LIQUID_EIRANGE,"liquid_lbcircshift(), shift amount must be in [0,7]");

    // 
    unsigned int shift_0 = _b;              // shift amount: first byte
    unsigned int shift_1 = 8 - _b;          // shift amount: second byte
    unsigned char mask_0 = 0xff << shift_0; // bit mask: first byte
    unsigned char mask_1 = 0xff >> shift_1; // bit mask: second byte
#if 0
    printf("num_bits = %1u, shift_0 = %1u, shift_1 = %1u mask_0=%.2x, mask_1=%.2x\n",
            _b, shift_0, shift_1, mask_0, mask_1);
#endif
    // shift then mask
    unsigned int i;
    unsigned char byte;
    unsigned char byte_0;
    unsigned char byte_1;
    unsigned char src_0 = _src[0];  // retain first byte
    for (i=0; i<_n; i++) {
        // strip bytes
        byte_0 = _src[i];
        byte_1 = (i==_n-1) ? src_0 : _src[i+1];

        // shift then mask
        byte = ((byte_0 << shift_0) & mask_0) |
               ((byte_1 >> shift_1) & mask_1);

        // store result
        _src[i] = byte;
    }
    return LIQUID_OK;
}

 
// circular shift array to the right _b bits
//  _src        :   source address [size: _n x 1]
//  _n          :   input data array size
//  _b          :   number of bits to shift
int liquid_rbcircshift(unsigned char * _src,
                       unsigned int    _n,
                       unsigned int    _b)
{
    // validate input
    if (_b >= 8)
        return liquid_error(LIQUID_EIRANGE,"liquid_rbshift(), shift amount must be in [0,7]");

    // 
    unsigned int shift_0 = 8 - _b;              // shift amount: first byte
    unsigned int shift_1 = _b;          // shift amount: second byte
    unsigned char mask_0 = 0xff << shift_0; // bit mask: first byte
    unsigned char mask_1 = 0xff >> shift_1; // bit mask: second byte
#if 0
    printf("num_bits = %1u, shift_0 = %1u, shift_1 = %1u mask_0=%.2x, mask_1=%.2x\n",
            _b, shift_0, shift_1, mask_0, mask_1);
#endif
    // shift then mask
    int i;
    unsigned char byte;
    unsigned char byte_0;
    unsigned char byte_1;
    unsigned char src_n = _src[_n-1];   // retain last byte
    for (i=_n-1; i>=0; i--) {
        // strip bytes
        byte_0 = (i==0) ? src_n : _src[i-1];
        byte_1 = _src[i];

        // shift then mask
        byte = ((byte_0 << shift_0) & mask_0) |
               ((byte_1 >> shift_1) & mask_1);

        // store result
        _src[i] = byte;
    }
    return LIQUID_OK;
}

