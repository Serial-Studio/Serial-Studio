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
// modem_dpsk.c
//

// create a dpsk (differential phase-shift keying) modem object
MODEM() MODEM(_create_dpsk)(unsigned int _bits_per_symbol)
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    
    switch (_bits_per_symbol) {
    case 1: q->scheme = LIQUID_MODEM_DPSK2;   break;
    case 2: q->scheme = LIQUID_MODEM_DPSK4;   break;
    case 3: q->scheme = LIQUID_MODEM_DPSK8;   break;
    case 4: q->scheme = LIQUID_MODEM_DPSK16;  break;
    case 5: q->scheme = LIQUID_MODEM_DPSK32;  break;
    case 6: q->scheme = LIQUID_MODEM_DPSK64;  break;
    case 7: q->scheme = LIQUID_MODEM_DPSK128; break;
    case 8: q->scheme = LIQUID_MODEM_DPSK256; break;
    default:
        return liquid_error_config("modem%s_create_dpsk(), cannot support DPSK with m > 8",EXTENSION);
    }

    MODEM(_init)(q, _bits_per_symbol);

    q->data.dpsk.alpha = M_PI/(T)(q->M);
    
    q->data.dpsk.phi = 0.0f;

    unsigned int k;
    for (k=0; k<(q->m); k++)
        q->ref[k] = (1<<k) * q->data.dpsk.alpha;

    q->data.dpsk.d_phi = M_PI*(1.0f - 1.0f/(T)(q->M));

    q->modulate_func = &MODEM(_modulate_dpsk);
    q->demodulate_func = &MODEM(_demodulate_dpsk);

    // reset and return
    MODEM(_reset)(q);
    return q;
}

// modulate DPSK
int MODEM(_modulate_dpsk)(MODEM()      _q,
                          unsigned int _sym_in,
                          TC *         _y)
{
    // 'encode' input symbol (actually gray decoding)
    _sym_in = gray_decode(_sym_in);

    // compute phase difference between this symbol and the previous
    _q->data.dpsk.phi += _sym_in * 2 * _q->data.dpsk.alpha;

    // limit phase
    _q->data.dpsk.phi -= (_q->data.dpsk.phi > 2*M_PI) ? 2*M_PI : 0.0f;
    
    // compute output sample
    *_y = liquid_cexpjf(_q->data.dpsk.phi);

    // save symbol state
    _q->r = *_y;
    return LIQUID_OK;
}


int MODEM(_demodulate_dpsk)(MODEM()        _q,
                            TC             _x,
                            unsigned int * _sym_out)
{
    // compute angle difference
    T theta = cargf(_x);
    T d_theta = cargf(_x) - _q->data.dpsk.phi;
    _q->data.dpsk.phi = theta;

    // subtract phase offset, ensuring phase is in [-pi,pi)
    d_theta -= _q->data.dpsk.d_phi;
    if (d_theta > M_PI)
        d_theta -= 2*M_PI;
    else if (d_theta < -M_PI)
        d_theta += 2*M_PI;

    // demodulate on linearly-spaced array
    unsigned int s;             // demodulated symbol
    T demod_phase_error;    // demodulation phase error
    MODEM(_demodulate_linear_array_ref)(d_theta, _q->m, _q->ref, &s, &demod_phase_error);

    // 'decode' output symbol (actually gray encoding)
    *_sym_out = gray_encode(s);

    // re-modulate symbol (accounting for differential rotation)
    // and store state
    _q->x_hat = liquid_cexpjf(theta - demod_phase_error);
    _q->r = _x;
    return LIQUID_OK;
}

