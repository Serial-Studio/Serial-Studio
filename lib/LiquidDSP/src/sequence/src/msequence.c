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

// maximum-length sequence

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

#define LIQUID_MIN_MSEQUENCE_M  2
#define LIQUID_MAX_MSEQUENCE_M  31

// maximal-length sequence
struct msequence_s {
    unsigned int m;     // length generator polynomial, shift register
    unsigned int g;     // generator polynomial, form: { x^m + ... + 1 }
    unsigned int a;     // initial shift register state, default: 1

    // derived values
    unsigned int n;     // length of sequence, n = (2^m)-1
    unsigned int state; // shift register
};

// create a maximal-length sequence (m-sequence) object with
// an internal shift register length of _m bits.
//  _m      :   generator polynomial length, sequence length is (2^m)-1
//  _g      :   generator polynomial, starting with most-significant bit
//  _a      :   initial shift register state, default: 000...001
msequence msequence_create(unsigned int _m,
                           unsigned int _g,
                           unsigned int _a)
{
    // validate input
    if (_m > LIQUID_MAX_MSEQUENCE_M || _m < LIQUID_MIN_MSEQUENCE_M)
        return liquid_error_config("msequence_create(), m (%u) not in range", _m);
    //if (_a == 0)
    //    return liquid_error_config("msequence_create(), state 'a' cannot be 0");
    
    // allocate memory for msequence object
    msequence ms = (msequence) malloc(sizeof(struct msequence_s));

    // set internal values
    ms->m = _m;         // generator polynomial length
    ms->g = _g;         // generator polynomial
    ms->a = _a;         // generator polynomial

    ms->n = (1<<_m)-1;  // sequence length, (2^m)-1
    ms->state = ms->a;  // shift register state
    return ms;
}

// Copy maximal-length sequence (m-sequence) object
msequence msequence_copy(msequence q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("msequence_copy(), object cannot be NULL");

    // create filter object and copy base parameters
    msequence q_copy = (msequence) malloc(sizeof(struct msequence_s));
    memmove(q_copy, q_orig, sizeof(struct msequence_s));
    return q_copy;
}

// create a maximal-length sequence (m-sequence) object from a generator polynomial
msequence msequence_create_genpoly(unsigned int _g)
{
    unsigned int t = liquid_msb_index(_g);

    // validate input
    if (t < 2)
        return liquid_error_config("msequence_create_genpoly(), invalid generator polynomial: 0x%x", _g);

    // compute derived values
    unsigned int m = t; // m-sequence shift register length
    unsigned int a = 1; // m-sequence initial state

    // generate object and return
    return msequence_create(m,_g,a);
}

// creates a default maximal-length sequence
msequence msequence_create_default(unsigned int _m)
{
    unsigned int g = 0;
    switch (_m) {
    case  2: g = LIQUID_MSEQUENCE_GENPOLY_M2;  break;
    case  3: g = LIQUID_MSEQUENCE_GENPOLY_M3;  break;
    case  4: g = LIQUID_MSEQUENCE_GENPOLY_M4;  break;
    case  5: g = LIQUID_MSEQUENCE_GENPOLY_M5;  break;
    case  6: g = LIQUID_MSEQUENCE_GENPOLY_M6;  break;
    case  7: g = LIQUID_MSEQUENCE_GENPOLY_M7;  break;
    case  8: g = LIQUID_MSEQUENCE_GENPOLY_M8;  break;
    case  9: g = LIQUID_MSEQUENCE_GENPOLY_M9;  break;
    case 10: g = LIQUID_MSEQUENCE_GENPOLY_M10; break;
    case 11: g = LIQUID_MSEQUENCE_GENPOLY_M11; break;
    case 12: g = LIQUID_MSEQUENCE_GENPOLY_M12; break;
    case 13: g = LIQUID_MSEQUENCE_GENPOLY_M13; break;
    case 14: g = LIQUID_MSEQUENCE_GENPOLY_M14; break;
    case 15: g = LIQUID_MSEQUENCE_GENPOLY_M15; break;
    case 16: g = LIQUID_MSEQUENCE_GENPOLY_M16; break;
    case 17: g = LIQUID_MSEQUENCE_GENPOLY_M17; break;
    case 18: g = LIQUID_MSEQUENCE_GENPOLY_M18; break;
    case 19: g = LIQUID_MSEQUENCE_GENPOLY_M19; break;
    case 20: g = LIQUID_MSEQUENCE_GENPOLY_M20; break;
    case 21: g = LIQUID_MSEQUENCE_GENPOLY_M21; break;
    case 22: g = LIQUID_MSEQUENCE_GENPOLY_M22; break;
    case 23: g = LIQUID_MSEQUENCE_GENPOLY_M23; break;
    case 24: g = LIQUID_MSEQUENCE_GENPOLY_M24; break;
    case 25: g = LIQUID_MSEQUENCE_GENPOLY_M25; break;
    case 26: g = LIQUID_MSEQUENCE_GENPOLY_M26; break;
    case 27: g = LIQUID_MSEQUENCE_GENPOLY_M27; break;
    case 28: g = LIQUID_MSEQUENCE_GENPOLY_M28; break;
    case 29: g = LIQUID_MSEQUENCE_GENPOLY_M29; break;
    case 30: g = LIQUID_MSEQUENCE_GENPOLY_M30; break;
    case 31: g = LIQUID_MSEQUENCE_GENPOLY_M31; break;
    default:
        return liquid_error_config("msequence_create_default(), m (%u) not in range", _m);
    }

    // return
    return msequence_create_genpoly(g);
}

// destroy an msequence object, freeing all internal memory
int msequence_destroy(msequence _ms)
{
    free(_ms);
    return LIQUID_OK;
}

// prints the sequence's internal state to the screen
int msequence_print(msequence _ms)
{
    printf("<liquid.msequence, m=%u, n=%u, g=0x%x, state=0x%x>\n",
        _ms->m, _ms->n, _ms->g, _ms->state);
    return LIQUID_OK;
}

// advance msequence on shift register, returning output bit
unsigned int msequence_advance(msequence _ms)
{
    // compute return bit as binary dot product between the
    // internal shift register and the generator polynomial
    unsigned int b = liquid_bdotprod( _ms->state, _ms->g);

    _ms->state <<= 1;       // shift internal register
    _ms->state |= b;        // push bit onto register
    _ms->state &= _ms->n;   // apply mask to register
    return b;               // return result
}


// generate pseudo-random symbol from shift register
//  _ms     :   m-sequence object
//  _bps    :   bits per symbol of output
unsigned int msequence_generate_symbol(msequence    _ms,
                                       unsigned int _bps)
{
    unsigned int i;
    unsigned int s = 0;
    for (i=0; i<_bps; i++) {
        s <<= 1;
        s |= msequence_advance(_ms);
    }
    return s;
}

// reset msequence shift register to original state, typically '1'
int msequence_reset(msequence _ms)
{
    _ms->state = _ms->a;
    return LIQUID_OK;
}

// initialize a bsequence object on an msequence object
//  _bs     :   bsequence object
//  _ms     :   msequence object
int bsequence_init_msequence(bsequence _bs,
                             msequence _ms)
{
    // clear binary sequence
    bsequence_reset(_bs);

    unsigned int i;
    for (i=0; i<(_ms->n); i++)
        bsequence_push(_bs, msequence_advance(_ms));
    return LIQUID_OK;
}

// get the length of the generator polynomial, g (m)
unsigned int msequence_get_genpoly_length(msequence _ms)
{
    return _ms->m;
}

// get the length of the sequence
unsigned int msequence_get_length(msequence _ms)
{
    return _ms->n;
}

// get the generator polynomial, g
unsigned int msequence_get_genpoly(msequence _ms)
{
    return _ms->g;
}

// get the internal state of the sequence
unsigned int msequence_get_state(msequence _ms)
{
    return _ms->state;
}

// set the internal state of the sequence
int msequence_set_state(msequence    _ms,
                        unsigned int _a)
{
    // set internal state
    // NOTE: if state is set to zero, this will lock the sequence generator,
    //       but let the user set this value if they wish
    _ms->state = _a;
    return LIQUID_OK;
}

// measure the period the shift register (should be 2^m-1 with a proper generator polynomial)
unsigned int msequence_measure_period(msequence _ms)
{
    // get current state
    unsigned int s = msequence_get_state(_ms);

    // cycle through sequence and look for initial state
    unsigned int i;
    unsigned int period = 0;
    for (i=0; i<_ms->n+1; i++) {
        msequence_advance(_ms);
        period++;
        if (msequence_get_state(_ms)==s)
            break;
    }

    // assert that state has been returned
    return period;
}

// measure the period of a generator polynomial
unsigned int msequence_genpoly_period(unsigned int _g)
{
    msequence q = msequence_create_genpoly(_g);
    if (q == NULL) {
        liquid_error(LIQUID_EICONFIG,"msequence_genpoly_period(), invalid generator polynomial 0x%x\n", _g);
        return 0;
    }
    unsigned int period = msequence_measure_period(q);
    msequence_destroy(q);
    return period;
}

