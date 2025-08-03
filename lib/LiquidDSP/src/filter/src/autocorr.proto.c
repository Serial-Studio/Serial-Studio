/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
// auto-correlator (delay cross-correlation)
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// defined:
//  AUTOCORR()      name-mangling macro
//  TI              type (input)
//  TC              type (coefficients)
//  TO              type (output)
//  WINDOW()        window macro
//  DOTPROD()       dotprod macro
//  PRINTVAL()      print macro

struct AUTOCORR(_s) {
    unsigned int window_size;
    unsigned int delay;

    WINDOW() w;         // input buffer
    WINDOW() wdelay;    // input buffer with delay

    float * we2;        // energy buffer
    float e2_sum;       // running sum of energy
    unsigned int ie2;   // read index
};

// create auto-correlator object                            
//  _window_size    : size of the correlator window         
//  _delay          : correlator delay [samples]            
AUTOCORR() AUTOCORR(_create)(unsigned int _window_size,
                             unsigned int _delay)
{
    // create main object
    AUTOCORR() q = (AUTOCORR()) malloc(sizeof(struct AUTOCORR(_s)));

    // set user-based parameters
    q->window_size = _window_size;
    q->delay       = _delay;

    // create window objects
    q->w      = WINDOW(_create)(q->window_size);
    q->wdelay = WINDOW(_create)(q->window_size + q->delay);

    // allocate array for squared energy buffer
    q->we2 = (float*) malloc( (q->window_size)*sizeof(float) );

    // clear object
    AUTOCORR(_reset)(q);

    // return main object
    return q;
}

// destroy auto-correlator object, freeing internal memory
int AUTOCORR(_destroy)(AUTOCORR() _q)
{
    // destroy internal window objects
    WINDOW(_destroy)(_q->w);
    WINDOW(_destroy)(_q->wdelay);

    // free array for squared energy buffer
    free(_q->we2);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// reset auto-correlator object's internals
int AUTOCORR(_reset)(AUTOCORR() _q)
{
    // clear/reset internal window buffers
    WINDOW(_reset)(_q->w);
    WINDOW(_reset)(_q->wdelay);
    
    // reset internal squared energy buffer
    _q->e2_sum = 0.0;
    unsigned int i;
    for (i=0; i<_q->window_size; i++)
        _q->we2[i] = 0.0;
    _q->ie2 = 0;    // reset read index to zero
    return LIQUID_OK;
}

// print auto-correlator parameters to stdout
int AUTOCORR(_print)(AUTOCORR() _q)
{
    printf("<liquid.autocorr, window=%u, delay=%u>\n", _q->window_size, _q->delay);
    return LIQUID_OK;
}

// push sample into auto-correlator object
int AUTOCORR(_push)(AUTOCORR() _q, TI _x)
{
    // push input sample into buffers
    WINDOW(_push)(_q->w,      _x);          // non-delayed buffer
    WINDOW(_push)(_q->wdelay, conj(_x));    // delayed buffer

    // push |_x|^2 into buffer at appropriate location
    float e2 = creal( _x*conj(_x) );
    _q->e2_sum -= _q->we2[ _q->ie2 ];
    _q->e2_sum += e2;
    _q->we2[ _q->ie2 ] = e2;
    _q->ie2 = (_q->ie2+1) % _q->window_size;
    return LIQUID_OK;
}

// Write block of samples into auto-correlator object
//  _q      :   auto-correlation object
//  _x      :   input array [size: _n x 1]
//  _n      :   number of input samples
int AUTOCORR(_write)(AUTOCORR()   _q,
                     TI *         _x,
                     unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        AUTOCORR(_push)(_q, _x[i]);
    return LIQUID_OK;
}

// compute auto-correlation output
int AUTOCORR(_execute)(AUTOCORR() _q, TO *_rxx)
{
    // provide pointers for reading buffer
    TI * rw;        // input buffer read pointer
    TC * rwdelay;   // input buffer read pointer (with delay)

    // read buffers; set internal pointers appropriately
    WINDOW(_read)(_q->w,      &rw     );
    WINDOW(_read)(_q->wdelay, &rwdelay);

    // execute vector dot product on arrays, saving result to
    // user-supplied output pointer
    return DOTPROD(_run4)(rw, rwdelay, _q->window_size, _rxx);
}

// compute auto-correlation on block of samples; the input
// and output arrays may have the same pointer
//  _q      :   auto-correlation object
//  _x      :   input array [size: _n x 1]
//  _n      :   number of input, output samples
//  _rxx    :   input array [size: _n x 1]
int AUTOCORR(_execute_block)(AUTOCORR()   _q,
                             TI *         _x,
                             unsigned int _n,
                             TO *         _rxx)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        // push input sample into auto-correlator
        AUTOCORR(_push)(_q, _x[i]);

        // compute output
        AUTOCORR(_execute)(_q, &_rxx[i]);
    }
    return LIQUID_OK;
}

// return sum of squares of buffered samples
float AUTOCORR(_get_energy)(AUTOCORR() _q)
{
    // value is already computed; simply return value
    return _q->e2_sum;
}

