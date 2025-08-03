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
#include <math.h>
#include <complex.h>

// compute complex phase rotation: x[i] = exp{ j theta[i] }
//  _theta  :   input primitive array [size: _n x 1]
//  _n      :   array length
//  _x      :   output array pointer [size: _n x 1]
void VECTOR(_cexpj)(TP *         _theta,
                    unsigned int _n,
                    T *          _x)
{
    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // compute in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
#if T_COMPLEX
        _x[i  ] = cexpf(_Complex_I*_theta[i  ]);
        _x[i+1] = cexpf(_Complex_I*_theta[i+1]);
        _x[i+2] = cexpf(_Complex_I*_theta[i+2]);
        _x[i+3] = cexpf(_Complex_I*_theta[i+3]);
#else
        _x[i  ] = _theta[i  ] > 0 ? 1.0 : -1.0;
        _x[i+1] = _theta[i+1] > 0 ? 1.0 : -1.0;
        _x[i+2] = _theta[i+2] > 0 ? 1.0 : -1.0;
        _x[i+3] = _theta[i+3] > 0 ? 1.0 : -1.0;
#endif
    }

    // clean up remaining
    for ( ; i<_n; i++) {
#if T_COMPLEX
        _x[i] = cexpf(_Complex_I*_theta[i]);
#else
        _x[i] = _theta[i] > 0 ? 1.0 : -1.0;
#endif
    }
}

// compute complex phase rotation: x[i] = exp{ j theta[i] }
//  _x      :   input array [size: _n x 1]
//  _n      :   array length
//  _theta  :   output primitive array [size: _n x 1]
void VECTOR(_carg)(T *          _x,
                   unsigned int _n,
                   TP *         _theta)
{
    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // compute in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
#if T_COMPLEX
        _theta[i  ] = cargf(_x[i  ]);
        _theta[i+1] = cargf(_x[i+1]);
        _theta[i+2] = cargf(_x[i+2]);
        _theta[i+3] = cargf(_x[i+3]);
#else
        _theta[i  ] = _x[i  ] > 0 ? 0 : M_PI;
        _theta[i+1] = _x[i+1] > 0 ? 0 : M_PI;
        _theta[i+2] = _x[i+2] > 0 ? 0 : M_PI;
        _theta[i+3] = _x[i+3] > 0 ? 0 : M_PI;
#endif
    }

    // clean up remaining
    for ( ; i<_n; i++) {
#if T_COMPLEX
        _theta[i] = cargf(_x[i]);
#else
        _theta[i] = _x[i] > 0 ? 0 : M_PI;
#endif
    }
}

// compute absolute value of each element: y[i] = |x[i]|
//  _x      :   input array [size: _n x 1]
//  _n      :   array length
//  _y      :   output primitive array pointer [size: _n x 1]
void VECTOR(_abs)(T *          _x,
                  unsigned int _n,
                  TP *         _y)
{
    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // compute in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
#if T_COMPLEX
        _y[i  ] = cabsf(_x[i  ]);
        _y[i+1] = cabsf(_x[i+1]);
        _y[i+2] = cabsf(_x[i+2]);
        _y[i+3] = cabsf(_x[i+3]);
#else
        _x[i  ] = fabsf(_x[i  ]);
        _x[i+1] = fabsf(_x[i+1]);
        _x[i+2] = fabsf(_x[i+2]);
        _x[i+3] = fabsf(_x[i+3]);
#endif
    }

    // clean up remaining
    for ( ; i<_n; i++) {
#if T_COMPLEX
        _y[i] = cabsf(_x[i]);
#else
        _x[i] = fabsf(_x[i]);
#endif
    }
}

