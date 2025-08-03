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
// structured quantizer
//

#include <stdio.h>
#include <stdlib.h>

struct QUANTIZER(_s) {
    int ctype;          // compander type
    unsigned int n;     // number of bits
                        // table?
};

// create quantizer object
//  _ctype      :   compander type (e.g. LIQUID_COMPANDER_LINEAR)
//  _range      :   maximum absolute input
//  _num_bits   :   number of bits per sample
QUANTIZER() QUANTIZER(_create)(liquid_compander_type _ctype,
                               float                 _range,
                               unsigned int          _num_bits)
{
    // validate input
    if (_num_bits == 0)
        return liquid_error_config("quantizer_create(), must have at least one bit/sample");

    // create quantizer object
    QUANTIZER() q = (QUANTIZER()) malloc(sizeof(struct QUANTIZER(_s)));

    // initialize values
    q->ctype = _ctype;
    q->n     = _num_bits;

    // return object
    return q;
}

int QUANTIZER(_destroy)(QUANTIZER() _q)
{
    // free main object memory
    free(_q);
    return LIQUID_OK;
}

int QUANTIZER(_print)(QUANTIZER() _q)
{
    printf("quantizer:\n");
    printf("  compander :   ");
    switch(_q->ctype) {
    case LIQUID_COMPANDER_NONE:     printf("none\n");   break;
    case LIQUID_COMPANDER_LINEAR:   printf("linear\n"); break;
    case LIQUID_COMPANDER_MULAW:    printf("mu-law\n"); break;
    case LIQUID_COMPANDER_ALAW:     printf("A-law\n");  break;
    default:
        printf("unknown\n");
    }
    printf("  num bits  :   %u\n", _q->n);
    return LIQUID_OK;
}

int QUANTIZER(_execute_adc)(QUANTIZER() _q,
                             T _x,
                             unsigned int * _sample)
{
#if T_COMPLEX
#else
#endif
    *_sample = 0;
    return LIQUID_OK;
}

int QUANTIZER(_execute_dac)(QUANTIZER() _q,
                             unsigned int _sample,
                             T * _x)
{
#if T_COMPLEX
#else
#endif
    *_x = 0.0;
    return LIQUID_OK;
}


