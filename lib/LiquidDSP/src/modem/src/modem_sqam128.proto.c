/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
// modem_sqam128.c
//

// create a 'square' 128-QAM modem object
MODEM() MODEM(_create_sqam128)()
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    q->scheme = LIQUID_MODEM_SQAM128;

    MODEM(_init)(q, 7);

    // allocate memory for 32-point symbol map
    q->data.sqam128.map = (TC*) malloc( 32*sizeof(TC) );
#if T == float
    memmove(q->data.sqam128.map, modem_arb_sqam128, 32*sizeof(TC));
#endif

    // set modulation, demodulation functions
    q->modulate_func   = &MODEM(_modulate_sqam128);
    q->demodulate_func = &MODEM(_demodulate_sqam128);

    // reset and return
    MODEM(_reset)(q);
    return q;
}

// modulate symbol with 'square' 128-QAM
int MODEM(_modulate_sqam128)(MODEM()      _q,
                             unsigned int _sym_in,
                             TC *         _y)
{
    // strip off most-significant two bits (quadrant)
    unsigned int quad = (_sym_in >> 5) & 0x03;
    
    // strip off least-significant 5 bits
    unsigned int s = _sym_in & 0x1f;
    TC p = _q->data.sqam128.map[s];
    
    switch (quad) {
    case 0: *_y =  p;        break;
    case 1: *_y =  conjf(p); break;
    case 2: *_y = -conjf(p); break;
    case 3: *_y = -p;        break;
    default:
        // should never get to this point
        return liquid_error(LIQUID_EINT,"modem%s_modulate_sqam128(), logic error",EXTENSION);
    }
    return LIQUID_OK;
}


// demodulate 'square' 128-QAM
int MODEM(_demodulate_sqam128)(MODEM()        _q,
                               TC             _x,
                               unsigned int * _sym_out)
{
    // determine quadrant and de-rotate to first quadrant
    // 10 | 00
    // ---+---
    // 11 | 01
    unsigned int quad = 2*(crealf(_x) < 0.0f) + (cimagf(_x) < 0.0f);
    
    TC x_prime = _x;
    switch (quad) {
    case 0: x_prime = _x;           break;
    case 1: x_prime =  conjf(_x);   break;
    case 2: x_prime = -conjf(_x);   break;
    case 3: x_prime = -_x;          break;
    default:
        // should never get to this point
        return liquid_error(LIQUID_EINT,"modem%s_demodulate_sqam128(), logic error",EXTENSION);
    }
    //printf(" x = %12.8f +j*%12.8f, quad = %1u, r = %12.8f + j*%12.8f\n",
    //        crealf(_x), cimagf(_x), quad, crealf(r), cimagf(r));
    assert(crealf(x_prime) >= 0.0f);
    assert(cimagf(x_prime) >= 0.0f);

    // find symbol in map closest to x_prime
    T dmin = 0.0f;
    T d = 0.0f;
    unsigned int i;
    for (i=0; i<32; i++) {
        d = cabsf(x_prime - _q->data.sqam128.map[i]);
        if (i==0 || d < dmin) {
            dmin = d;
            *_sym_out = i;
        }
    }

    // add quadrant bits
    *_sym_out |= (quad << 5);

    // re-modulate symbol and store state
    MODEM(_modulate_sqam128)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
    return LIQUID_OK;
}

