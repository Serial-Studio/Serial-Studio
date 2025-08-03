/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// Flipped Nyquist/root-Nyquist filter designs
//
// References:
//   [Beaulieu:2001]
//   [Assalini:2004]
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// Design flipped Nyquist/root-Nyquist filter
//  _type   : filter type (e.g. LIQUID_FIRFILT_FEXP)
//  _root   : square-root Nyquist filter?
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_fnyquist(liquid_firfilt_type _type,
                           int                 _root,
                           unsigned int        _k,
                           unsigned int        _m,
                           float               _beta,
                           float               _dt,
                           float *             _h)
{
    // validate input
    if ( _k < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_fnyquist(): k must be greater than 0");
    if ( _m < 1 )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_fnyquist(): m must be greater than 0");
    if ( (_beta < 0.0f) || (_beta > 1.0f) )
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_fnyquist(): beta must be in [0,1]");

    unsigned int i;

    // derived values
    unsigned int h_len = 2*_k*_m+1;   // filter length

    float H_prime[h_len];   // frequency response of Nyquist filter (real)
    float complex H[h_len]; // frequency response of Nyquist filter
    float complex h[h_len]; // impulse response of Nyquist filter

    // compute Nyquist filter frequency response
    switch (_type) {
    case LIQUID_FIRFILT_FEXP:
        liquid_firdes_fexp_freqresponse(_k, _m, _beta, H_prime);
        break;
    case LIQUID_FIRFILT_FSECH:
        liquid_firdes_fsech_freqresponse(_k, _m, _beta, H_prime);
        break;
    case LIQUID_FIRFILT_FARCSECH:
        liquid_firdes_farcsech_freqresponse(_k, _m, _beta, H_prime);
        break;
    default:
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_fnyquist(), unknown/unsupported filter type");
    }

    // copy result to fft input buffer, computing square root
    // if required
    for (i=0; i<h_len; i++)
        H[i] = _root ? sqrtf(H_prime[i]) : H_prime[i];

    // compute ifft
    fft_run(h_len, H, h, LIQUID_FFT_BACKWARD, 0);
    
    // copy shifted, scaled response
    for (i=0; i<h_len; i++)
        _h[i] = crealf( h[(i+_k*_m+1)%h_len] ) * (float)_k / (float)(h_len);
    return LIQUID_OK;
}

// Design fexp Nyquist filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_fexp(unsigned int _k,
                       unsigned int _m,
                       float _beta,
                       float _dt,
                       float * _h)
{
    // compute response using generic function
    return liquid_firdes_fnyquist(LIQUID_FIRFILT_FEXP, 0, _k, _m, _beta, _dt, _h);
}

// Design fexp square-root Nyquist filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_rfexp(unsigned int _k,
                        unsigned int _m,
                        float _beta,
                        float _dt,
                        float * _h)
{
    // compute response using generic function
    return liquid_firdes_fnyquist(LIQUID_FIRFILT_FEXP, 1, _k, _m, _beta, _dt, _h);
}

// flipped exponential frequency response
int liquid_firdes_fexp_freqresponse(unsigned int _k,
                                    unsigned int _m,
                                    float        _beta,
                                    float *      _H)
{
    // TODO : validate input

    unsigned int i;

    unsigned int h_len = 2*_k*_m + 1;

    float f0 = 0.5f*(1.0f - _beta) / (float)_k;
    float f1 = 0.5f*(1.0f        ) / (float)_k;
    float f2 = 0.5f*(1.0f + _beta) / (float)_k;

    float B     = 0.5f/(float)_k;
    float gamma = logf(2.0f)/(_beta*B);

    // compute frequency response of Nyquist filter
    for (i=0; i<h_len; i++) {
        float f = (float)i / (float)h_len;
        if (f > 0.5f) f = f - 1.0f;

        // enforce even symmetry
        f = fabsf(f);

        if ( f < f0 ) {
            // pass band
            _H[i] = 1.0f;
        } else if (f > f0 && f < f2) {
            // transition band
            if ( f < f1) {
                _H[i] = expf(gamma*(B*(1-_beta) - f));
            } else {
                _H[i] = 1.0f - expf(gamma*(f - (1+_beta)*B));
            }
        } else {
            // stop band
            _H[i] = 0.0f;
        }
    }
    return LIQUID_OK;
}

// Design fsech Nyquist filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_fsech(unsigned int _k,
                        unsigned int _m,
                        float _beta,
                        float _dt,
                        float * _h)
{
    // compute response using generic function
    return liquid_firdes_fnyquist(LIQUID_FIRFILT_FSECH, 0, _k, _m, _beta, _dt, _h);
}

// Design fsech square-root Nyquist filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_rfsech(unsigned int _k,
                         unsigned int _m,
                         float _beta,
                         float _dt,
                         float * _h)
{
    // compute response using generic function
    return liquid_firdes_fnyquist(LIQUID_FIRFILT_FSECH, 1, _k, _m, _beta, _dt, _h);
}

// flipped exponential frequency response
int liquid_firdes_fsech_freqresponse(unsigned int _k,
                                    unsigned int _m,
                                    float        _beta,
                                    float *      _H)
{
    // TODO : validate input

    unsigned int i;

    unsigned int h_len = 2*_k*_m + 1;

    float f0 = 0.5f*(1.0f - _beta) / (float)_k;
    float f1 = 0.5f*(1.0f        ) / (float)_k;
    float f2 = 0.5f*(1.0f + _beta) / (float)_k;

    float B     = 0.5f/(float)_k;
    float gamma = logf(sqrtf(3.0f) + 2.0f) / (_beta*B);

    // compute frequency response of Nyquist filter
    for (i=0; i<h_len; i++) {
        float f = (float)i / (float)h_len;
        if (f > 0.5f) f = f - 1.0f;

        // enforce even symmetry
        f = fabsf(f);

        if ( f < f0 ) {
            // pass band
            _H[i] = 1.0f;
        } else if (f > f0 && f < f2) {
            // transition band
            if ( f < f1) {
                _H[i] = 1.0f / coshf(gamma*(f - B*(1-_beta)));
            } else {
                _H[i] = 1.0f - 1.0f / coshf(gamma*(B*(1+_beta) - f));
            }
        } else {
            // stop band
            _H[i] = 0.0f;
        }
    }
    return LIQUID_OK;
}

// Design farcsech Nyquist filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_farcsech(unsigned int _k,
                           unsigned int _m,
                           float _beta,
                           float _dt,
                           float * _h)
{
    // compute response using generic function
    return liquid_firdes_fnyquist(LIQUID_FIRFILT_FARCSECH, 0, _k, _m, _beta, _dt, _h);
}

// Design farcsech square-root Nyquist filter
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_rfarcsech(unsigned int _k,
                            unsigned int _m,
                            float _beta,
                            float _dt,
                            float * _h)
{
    // compute response using generic function
    return liquid_firdes_fnyquist(LIQUID_FIRFILT_FARCSECH, 1, _k, _m, _beta, _dt, _h);
}

// hyperbolic arc-secant
float liquid_asechf(float _z)
{
    if (_z <= 0.0f || _z > 1.0f) {
        liquid_error(LIQUID_EICONFIG,"liquid_asechf(), input (_z=%g)out of range (0,1)", _z);
        return 0.0f;
    }

    float z_inv = 1.0f / _z;

    return logf( sqrtf(z_inv - 1.0f)*sqrtf(z_inv + 1.0f) + z_inv );
}

// flipped exponential frequency response
int liquid_firdes_farcsech_freqresponse(unsigned int _k,
                                        unsigned int _m,
                                        float        _beta,
                                        float *      _H)
{
    // TODO : validate input

    unsigned int i;

    unsigned int h_len = 2*_k*_m + 1;

    float f0 = 0.5f*(1.0f - _beta) / (float)_k;
    float f1 = 0.5f*(1.0f        ) / (float)_k;
    float f2 = 0.5f*(1.0f + _beta) / (float)_k;

    float B     = 0.5f/(float)_k;
    float gamma = logf(sqrtf(3.0f) + 2.0f) / (_beta*B);
    float zeta  = 1.0f / (2.0f * _beta * B);

    // compute frequency response of Nyquist filter
    for (i=0; i<h_len; i++) {
        float f = (float)i / (float)h_len;
        if (f > 0.5f) f = f - 1.0f;

        // enforce even symmetry
        f = fabsf(f);

        if ( f < f0 ) {
            // pass band
            _H[i] = 1.0f;
        } else if (f > f0 && f < f2) {
            // transition band
            if ( f < f1) {
                _H[i] = 1.0f - (zeta/gamma)*liquid_asechf(zeta*(B*(1+_beta) - f));
            } else {
                _H[i] = (zeta/gamma)*liquid_asechf(zeta*(f - B*(1-_beta)));
            }
        } else {
            // stop band
            _H[i] = 0.0f;
        }
    }
    return LIQUID_OK;
}

