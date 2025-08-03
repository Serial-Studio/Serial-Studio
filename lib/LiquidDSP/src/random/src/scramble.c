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

// data scrambler

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

void scramble_data(unsigned char * _x,
                   unsigned int _n)
{
    // t = 4*(floor(_n/4))
    unsigned int t = (_n>>2)<<2;

    // apply static masks
    unsigned int i;
    for (i=0; i<t; i+=4) {
        _x[i  ] ^= LIQUID_SCRAMBLE_MASK0;
        _x[i+1] ^= LIQUID_SCRAMBLE_MASK1;
        _x[i+2] ^= LIQUID_SCRAMBLE_MASK2;
        _x[i+3] ^= LIQUID_SCRAMBLE_MASK3;
    }

    // clean up remainder of elements
    if ( (i+0) < _n ) _x[i+0] ^= LIQUID_SCRAMBLE_MASK0;
    if ( (i+1) < _n ) _x[i+1] ^= LIQUID_SCRAMBLE_MASK1;
    if ( (i+2) < _n ) _x[i+2] ^= LIQUID_SCRAMBLE_MASK2;
    if ( (i+3) < _n ) _x[i+3] ^= LIQUID_SCRAMBLE_MASK3;
}

void unscramble_data(unsigned char * _x,
                     unsigned int _n)
{
    // for now apply simple static mask (re-run scramble)
    scramble_data(_x,_n);
}

// unscramble soft bits
//  _x      :   input message (soft bits) [size: 8*_n x 1]
//  _n      :   original message length (bytes)
void unscramble_data_soft(unsigned char * _x,
                          unsigned int _n)
{
    // bit mask
    unsigned char mask = 0x00;

    // apply static masks
    unsigned int i;
    for (i=0; i<_n; i++) {
        switch ( i % 4 ) {
        case 0: mask = LIQUID_SCRAMBLE_MASK0; break;
        case 1: mask = LIQUID_SCRAMBLE_MASK1; break;
        case 2: mask = LIQUID_SCRAMBLE_MASK2; break;
        case 3: mask = LIQUID_SCRAMBLE_MASK3; break;
        default:;
        }

#if 0
        unsigned int j;
        for (j=0; j<8; j++) {
            if ( (mask >> (8-j-1)) & 0x01 )
                _x[8*i+j] = 255 - _x[8*i+j];
        }
#else
        if ( mask & 0x80 ) _x[8*i+0] = 255 - _x[8*i+0];
        if ( mask & 0x40 ) _x[8*i+1] = 255 - _x[8*i+1];
        if ( mask & 0x20 ) _x[8*i+2] = 255 - _x[8*i+2];
        if ( mask & 0x10 ) _x[8*i+3] = 255 - _x[8*i+3];
        if ( mask & 0x08 ) _x[8*i+4] = 255 - _x[8*i+4];
        if ( mask & 0x04 ) _x[8*i+5] = 255 - _x[8*i+5];
        if ( mask & 0x02 ) _x[8*i+6] = 255 - _x[8*i+6];
        if ( mask & 0x01 ) _x[8*i+7] = 255 - _x[8*i+7];
#endif
    }
}

