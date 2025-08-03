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
// Generic vector addition
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// basic vector addition, unrolling loop
//  _x      :   first array  [size: _n x 1]
//  _y      :   second array [size: _n x 1]
//  _n      :   array lengths
//  _z      :   output array pointer [size: _n x 1]
void VECTOR(_add)(T *          _x,
                  T *          _y,
                  unsigned int _n,
                  T *          _z)
{
    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // compute in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
        _z[i  ] = _x[i  ] + _y[i  ];
        _z[i+1] = _x[i+1] + _y[i+1];
        _z[i+2] = _x[i+2] + _y[i+2];
        _z[i+3] = _x[i+3] + _y[i+3];
    }

    // clean up remaining
    for ( ; i<_n; i++)
        _z[i] = _x[i] + _y[i];
}

// basic vector scalar addition, unrolling loop
//  _x      :   input array  [size: _n x 1]
//  _n      :   array length
//  _v      :   scalar
//  _y      :   output array pointer [size: _n x 1]
void VECTOR(_addscalar)(T *          _x,
                        unsigned int _n,
                        T            _v,
                        T *          _y)
{
    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // compute in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
        _y[i  ] = _x[i  ] + _v;
        _y[i+1] = _x[i+1] + _v;
        _y[i+2] = _x[i+2] + _v;
        _y[i+3] = _x[i+3] + _v;
    }

    // clean up remaining
    for ( ; i<_n; i++)
        _y[i] = _x[i] + _v;
}

