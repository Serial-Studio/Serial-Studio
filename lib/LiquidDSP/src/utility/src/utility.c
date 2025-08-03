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
// utility.c
//
// Useful generic utilities
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "liquid.h"

// get scale for constant, particularly for plotting purposes
//  _val    : input value (e.g. 100e6)
//  _unit   : output unit character (e.g. 'M')
//  _scale  : output scale (e.g. 1e-6)
int liquid_get_scale(float   _val,
                     char *  _unit,
                     float * _scale)
{
    float v = fabsf(_val);
    if      (v < 1e-9) { *_scale = 1e12;  *_unit = 'p'; }
    else if (v < 1e-6) { *_scale = 1e9 ;  *_unit = 'n'; }
    else if (v < 1e-3) { *_scale = 1e6 ;  *_unit = 'u'; }
    else if (v < 1e+0) { *_scale = 1e3 ;  *_unit = 'm'; }
    else if (v < 1e3)  { *_scale = 1e0 ;  *_unit = ' '; }
    else if (v < 1e6)  { *_scale = 1e-3;  *_unit = 'k'; }
    else if (v < 1e9)  { *_scale = 1e-6;  *_unit = 'M'; }
    else if (v < 1e12) { *_scale = 1e-9;  *_unit = 'G'; }
    else if (v < 1e15) { *_scale = 1e-12; *_unit = 'T'; }
    else               { *_scale = 1e-15; *_unit = 'P'; }
    return LIQUID_OK;
}

