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
// optim.common.c
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"


// optimization threshold switch
//  _u0         :   first utility
//  _u1         :   second utility
//  _minimize   :   minimize flag
//
// returns:
//  (_u0 > _u1) if (_minimize == 1)
//  (_u0 < _u1) otherwise
int optim_threshold_switch(float _u0,
                           float _u1,
                           int _minimize)
{
    return _minimize ? _u0 > _u1 : _u0 < _u1;
}

// sort values by index
//  _v          :   input values [size: _len x 1]
//  _rank       :   output rank array (indices) [size: _len x 1]
//  _len        :   length of input array
//  _descending :   descending/ascending
void optim_sort(float *_v,
                unsigned int * _rank,
                unsigned int _len,
                int _descending)
{
    unsigned int i, j, tmp_index;

    for (i=0; i<_len; i++)
        _rank[i] = i;

    for (i=0; i<_len; i++) {
        for (j=_len-1; j>i; j--) {
            //if (_v[_rank[j]]>_v[_rank[j-1]]) {
            if ( optim_threshold_switch(_v[_rank[j]], _v[_rank[j-1]], _descending) ) {
                // swap elements
                tmp_index = _rank[j];
                _rank[j] = _rank[j-1];
                _rank[j-1] = tmp_index;
            }
        }
    }
}


