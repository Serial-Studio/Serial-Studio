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
// Windowing functions
//
// References:
//  [Kaiser:1980] James F. Kaiser and Ronald W. Schafer, "On
//      the Use of I0-Sinh Window for Spectrum Analysis,"
//      IEEE Transactions on Acoustics, Speech, and Signal
//      Processing, vol. ASSP-28, no. 1, pp. 105--107,
//      February, 1980.
//  [harris:1978] frederic j. harris, "On the Use of Windows for Harmonic
//      Analysis with the Discrete Fourier Transform," Proceedings of the
//      IEEE, vol. 66, no. 1, January, 1978.
//  [Nuttall:1981] Albert H. Nuttall, "Some Windows with Very Good Sidelobe
//      Behavior,"  IEEE Transactions on Acoustics, Speech, and Signal
//      Processing, vol. ASSP-29, no. 1, pp. 84-91, February, 1981.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

const char * liquid_window_str[LIQUID_WINDOW_NUM_FUNCTIONS][2] = {
    // short name,  long name
    {"unknown",         "unknown"                   },
    {"hamming",         "Hamming"                   },
    {"hann",            "Hann"                      },
    {"blackmanharris",  "Blackman-harris (4-term)"  },
    {"blackmanharris7", "Blackman-harris (7-term)"  },
    {"kaiser",          "Kaiser-Bessel"             },
    {"flattop",         "flat top"                  },
    {"triangular",      "triangular"                },
    {"rcostaper",       "raised-cosine taper"       },
    {"kbd",             "Kaiser-Bessel derived"     },
};

// Print compact list of existing and available windowing functions
int liquid_print_windows()
{
    unsigned int i;
    unsigned int len = 10;

    // print all available window functions
    printf("          ");
    for (i=0; i<LIQUID_WINDOW_NUM_FUNCTIONS; i++) {
        printf("%s", liquid_window_str[i][0]);

        if (i != LIQUID_WINDOW_NUM_FUNCTIONS-1)
            printf(", ");

        len += strlen(liquid_window_str[i][0]);
        if (len > 48 && i != LIQUID_WINDOW_NUM_FUNCTIONS-1) {
            len = 10;
            printf("\n          ");
        }
    }
    printf("\n");
    return LIQUID_OK;
}

// returns modulation_scheme based on input string
liquid_window_type liquid_getopt_str2window(const char * _str)
{
    // compare each string to short name
    unsigned int i;
    for (i=0; i<LIQUID_WINDOW_NUM_FUNCTIONS; i++) {
        if (strcmp(_str,liquid_window_str[i][0])==0) {
            return i;
        }
    }

    liquid_error(LIQUID_EICONFIG,"liquid_getopt_str2window(), unknown/unsupported window scheme: %s", _str);
    return LIQUID_WINDOW_UNKNOWN;
}

// generic window function given type
//  _type   :   window type, e.g. LIQUID_WINDOW_KAISER
//  _i      :   window index, _i in [0,_wlen-1]
//  _wlen   :   length of window
//  _arg    :   window-specific argument, if required
float liquid_windowf(liquid_window_type _type,
                     unsigned int       _i,
                     unsigned int       _wlen,
                     float              _arg)
{
    switch (_type) {
    case LIQUID_WINDOW_HAMMING:         return liquid_hamming         (_i, _wlen);
    case LIQUID_WINDOW_HANN:            return liquid_hann            (_i, _wlen);
    case LIQUID_WINDOW_BLACKMANHARRIS:  return liquid_blackmanharris  (_i, _wlen);
    case LIQUID_WINDOW_BLACKMANHARRIS7: return liquid_blackmanharris7 (_i, _wlen);
    case LIQUID_WINDOW_KAISER:          return liquid_kaiser          (_i, _wlen, _arg);
    case LIQUID_WINDOW_FLATTOP:         return liquid_flattop         (_i, _wlen);
    case LIQUID_WINDOW_TRIANGULAR:      return liquid_triangular      (_i, _wlen, (unsigned int)_arg);
    case LIQUID_WINDOW_RCOSTAPER:       return liquid_rcostaper_window(_i, _wlen, (unsigned int)_arg);
    case LIQUID_WINDOW_KBD:             return liquid_kbd             (_i, _wlen, _arg);
    case LIQUID_WINDOW_UNKNOWN:
    default:
        liquid_error(LIQUID_EICONFIG,"liquid_windowf(), invalid type: %d", _type);
    }
    return 0.0f;
}

// Kaiser-Bessel derived window
// TODO add reference
float liquid_kbd(unsigned int _i,
                 unsigned int _wlen,
                 float        _beta)
{
    // validate input
    if (_i >= _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_kbd(), index exceeds maximum");
        return 0.0f;
    } else if (_wlen == 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_kbd(), window length must be greater than zero");
        return 0.0f;
    } else if ( _wlen % 2 ) {
        liquid_error(LIQUID_EICONFIG,"liquid_kbd(), window length must be odd");
        return 0.0f;
    }

    unsigned int M = _wlen / 2;
    if (_i >= M)
        return liquid_kbd(_wlen-_i-1,_wlen,_beta);

    float w0 = 0.0f;
    float w1 = 0.0f;
    float w;
    unsigned int i;
    for (i=0; i<=M; i++) {
        // compute Kaiser window
        w = liquid_kaiser(i,M+1,_beta);

        // accumulate window sums
        w1 += w;
        if (i <= _i) w0 += w;
    }
    //printf("%12.8f / %12.8f = %12.8f\n", w0, w1, w0/w1);

    return sqrtf(w0 / w1);
}


// Kaiser-Bessel derived window (full window function)
// TODO add reference
int liquid_kbd_window(unsigned int _i,
                      float        _beta,
                      float *      _w)
{
    // validate input
    if (_i == 0)
        return liquid_error(LIQUID_EICONFIG,"liquid_kbd_window(), window length must be greater than zero");
    if ( _i % 2 )
        return liquid_error(LIQUID_EICONFIG,"liquid_kbd_window(), window length must be odd");
    if ( _beta < 0.0f )
        return liquid_error(LIQUID_EICONFIG,"liquid_kbd_window(), _beta must be positive");

    // compute half length
    unsigned int M = _i / 2;

    // generate regular Kaiser window, length M+1
    unsigned int i;
    float w_kaiser[M+1];
    for (i=0; i<=M; i++)
        w_kaiser[i] = liquid_kaiser(i,M+1,_beta);

    // compute sum(wk[])
    float w_sum = 0.0f;
    for (i=0; i<=M; i++)
        w_sum += w_kaiser[i];

    // accumulate window
    float w_acc = 0.0f;
    for (i=0; i<M; i++) {
        w_acc += w_kaiser[i];
        _w[i] = sqrtf(w_acc / w_sum);
    }

    // window has even symmetry; flip around index M
    for (i=0; i<M; i++)
        _w[_i-i-1] = _w[i];
    return LIQUID_OK;
}


// Kaiser window [Kaiser:1980]
//  _i      :   sample index
//  _wlen   :   window length (samples)
//  _beta   :   window taper parameter
float liquid_kaiser(unsigned int _i,
                    unsigned int _wlen,
                    float        _beta)
{
    // validate input
    if (_i >= _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_kaiser(), sample index must not exceed window length");
        return 0.0f;
    } else if (_beta < 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_kaiser(), beta must be greater than or equal to zero");
        return 0.0f;
    }

    float t = (float)_i - (float)(_wlen-1)/2;
    float r = 2.0f*t/(float)(_wlen-1);
    float a = liquid_besseli0f(_beta*sqrtf(1-r*r));
    float b = liquid_besseli0f(_beta);
    //printf("kaiser(%3u,%u3,%6.3f) t:%8.3f, r:%8.3f, a:%8.3f, b:%8.3f\n", _i,_wlen,_beta,t,r,a,b);
    return a / b;
}

// Hamming window [Nuttall:1981]
float liquid_hamming(unsigned int _i,
                     unsigned int _wlen)
{
    // validate input
    if (_i > _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_hamming(), sample index must not exceed window length");
        return 0.0f;
    }

    return 0.53836 - 0.46164*cosf( (2*M_PI*(float)_i) / ((float)(_wlen-1)) );
}

// Hann window
float liquid_hann(unsigned int _i,
                  unsigned int _wlen)
{
    // validate input
    if (_i > _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_hann(), sample index must not exceed window length");
        return 0.0f;
    }

    // TODO test this function
    // TODO add reference
    return 0.5f - 0.5f*cosf( (2*M_PI*(float)_i) / ((float)(_wlen-1)) );
}

// Blackman-harris window [harris:1978]
float liquid_blackmanharris(unsigned int _i,
                            unsigned int _wlen)
{
    // validate input
    if (_i > _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_blackmanharris(), sample index must not exceed window length");
        return 0.0f;
    }

    // TODO test this function
    // TODO add reference
    float a0 = 0.35875f;
    float a1 = 0.48829f;
    float a2 = 0.14128f;
    float a3 = 0.01168f;
    float t = 2*M_PI*(float)_i / ((float)(_wlen-1));

    return a0 - a1*cosf(t) + a2*cosf(2*t) - a3*cosf(3*t);
}

// 7th-order Blackman-harris window
float liquid_blackmanharris7(unsigned int _i,
                             unsigned int _wlen)
{
    // validate input
    if (_i > _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_blackmanharris7(), sample index must not exceed window length");
        return 0.0f;
    }

	float a0 = 0.27105f;
	float a1 = 0.43329f;
	float a2 = 0.21812f;
	float a3 = 0.06592f;
	float a4 = 0.01081f;
	float a5 = 0.00077f;
	float a6 = 0.00001f;
	float t = 2*M_PI*(float)_i / ((float)(_wlen-1));

	return a0 - a1*cosf(  t) + a2*cosf(2*t) - a3*cosf(3*t)
			  + a4*cosf(4*t) - a5*cosf(5*t) + a6*cosf(6*t);
}

// Flat-top window
float liquid_flattop(unsigned int _i,
                     unsigned int _wlen)
{
    // validate input
    if (_i > _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_flattop(), sample index must not exceed window length");
        return 0.0f;
    }

	float a0 = 1.000f;
	float a1 = 1.930f;
	float a2 = 1.290f;
	float a3 = 0.388f;
	float a4 = 0.028f;
	float t = 2*M_PI*(float)_i / ((float)(_wlen-1));

	return a0 - a1*cosf(t) + a2*cosf(2*t) - a3*cosf(3*t) + a4*cosf(4*t);
}

// Triangular window
float liquid_triangular(unsigned int _i,
                        unsigned int _wlen,
                        unsigned int _n)
{
    // validate input
    if (_i > _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_triangular(), sample index must not exceed window length");
        return 0.0f;
    } else if (_n != _wlen-1 && _n != _wlen && _n != _wlen+1) {
        liquid_error(LIQUID_EICONFIG,"liquid_triangular(), sub-length must be in _wlen+{-1,0,1}");
        return 0.0f;
    } else if (_n == 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_triangular(), sub-length must be greater than zero");
        return 0.0f;
    }

	float v0 = (float)_i - (float)((_wlen-1)/2.0f);
	float v1 = ((float)_n)/2.0f;
	return 1.0 - fabsf(v0 / v1);
}

// raised-cosine tapering window
//  _i      :   window index
//  _wlen   :   full window length
//  _t      :   taper length
float liquid_rcostaper_window(unsigned int _i,
                              unsigned int _wlen,
                              unsigned int _t)
{
    // validate input
    if (_i > _wlen) {
        liquid_error(LIQUID_EICONFIG,"liquid_rcostaper_window(), sample index must not exceed window length");
        return 0.0f;
    } else if (_t > _wlen/2) {
        liquid_error(LIQUID_EICONFIG,"liquid_rcostaper_window(), taper length cannot exceed half window length");
        return 0.0f;
    }

    // reverse time for ramp-down section
    if (_i > _wlen - _t - 1)
        _i = _wlen - _i - 1;

    // return ramp or flat component
    return (_i < _t) ? 0.5f - 0.5f*cosf(M_PI*((float)_i + 0.5f) / (float)_t) : 1.0f;
}

// shim to support legacy APIs (backwards compatible with 1.3.2)

float kaiser(unsigned int _i,unsigned int _wlen, float _beta, float _dt)
    { return liquid_kaiser(_i,_wlen,_beta); }

float hamming(unsigned int _i,unsigned int _wlen)
    { return liquid_hamming(_i,_wlen); }

float hann(unsigned int _i,unsigned int _wlen)
    { return liquid_hann(_i,_wlen); }

float blackmanharris(unsigned int _i,unsigned int _wlen)
    { return liquid_blackmanharris(_i,_wlen); }

float blackmanharris7(unsigned int _i,unsigned int _wlen)
    { return liquid_blackmanharris7(_i,_wlen); }

float flattop(unsigned int _i,unsigned int _wlen)
    { return liquid_flattop(_i,_wlen); }

float triangular(unsigned int _i,unsigned int _wlen,unsigned int _n)
    { return liquid_triangular(_i,_wlen,_n); }

float liquid_rcostaper_windowf(unsigned int _i,unsigned int _wlen,unsigned int _t)
    { return liquid_rcostaper_window(_i,_wlen,_t); }

float kbd(unsigned int _i,unsigned int _wlen,float _beta)
    { return liquid_kbd(_i,_wlen,_beta); }

int kbd_window(unsigned int _wlen,float _beta,float * _w)
    { return liquid_kbd_window(_wlen,_beta,_w); }

