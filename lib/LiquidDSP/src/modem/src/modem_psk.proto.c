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
// modem_psk.c
//

// create a psk (phase-shift keying) modem object
MODEM() MODEM(_create_psk)(unsigned int _bits_per_symbol)
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );

    switch (_bits_per_symbol) {
    case 1: q->scheme = LIQUID_MODEM_PSK2;   break;
    case 2: q->scheme = LIQUID_MODEM_PSK4;   break;
    case 3: q->scheme = LIQUID_MODEM_PSK8;   break;
    case 4: q->scheme = LIQUID_MODEM_PSK16;  break;
    case 5: q->scheme = LIQUID_MODEM_PSK32;  break;
    case 6: q->scheme = LIQUID_MODEM_PSK64;  break;
    case 7: q->scheme = LIQUID_MODEM_PSK128; break;
    case 8: q->scheme = LIQUID_MODEM_PSK256; break;
    default:
        return liquid_error_config("modem%s_create_psk(), cannot support PSK with m > 8",EXTENSION);
    }

    // initialize basic modem structure
    MODEM(_init)(q, _bits_per_symbol);

    // compute alpha
    q->data.psk.alpha = M_PI/(T)(q->M);

    // initialize demodulation array reference
    unsigned int k;
    for (k=0; k<(q->m); k++)
        q->ref[k] = (1<<k) * q->data.psk.alpha;

    // compute phase offset (half of phase difference between symbols)
    q->data.psk.d_phi = M_PI*(1.0f - 1.0f/(T)(q->M));

    // set modulation/demodulation functions
    q->modulate_func = &MODEM(_modulate_psk);
    q->demodulate_func = &MODEM(_demodulate_psk);

    // initialize symbol map
    q->symbol_map = (TC*)malloc(q->M*sizeof(TC));
    MODEM(_init_map)(q);
    q->modulate_using_map = 1;

    // initialize soft-demodulation look-up table
    if (q->m >= 3)
        MODEM(_demodsoft_gentab)(q, 2);

    // reset and return
    MODEM(_reset)(q);
    return q;
}

// modulate PSK
int MODEM(_modulate_psk)(MODEM()      _q,
                         unsigned int _sym_in,
                         TC *         _y)
{
    // 'encode' input symbol (actually gray decoding)
    _sym_in = gray_decode(_sym_in);

    // compute output sample
    *_y = liquid_cexpjf(_sym_in * 2 * _q->data.psk.alpha );
    return LIQUID_OK;
}

// demodulate PSK
int MODEM(_demodulate_psk)(MODEM()        _q,
                           TC             _x,
                           unsigned int * _sym_out)
{
    // compute angle and subtract phase offset, ensuring phase is in [-pi,pi)
    T theta = cargf(_x);
    theta -= _q->data.psk.d_phi;
    if (theta < -M_PI)
        theta += 2*M_PI;

    // demodulate on linearly-spaced array
    unsigned int s;             // demodulated symbol
    T demod_phase_error;        // demodulation phase error
    MODEM(_demodulate_linear_array_ref)(theta, _q->m, _q->ref, &s, &demod_phase_error);

    // 'decode' output symbol (actually gray encoding)
    *_sym_out = gray_encode(s);

    // re-modulate symbol and store state
    MODEM(_modulate_psk)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
    return LIQUID_OK;
}

