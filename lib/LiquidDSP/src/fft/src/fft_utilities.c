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
// fft_utilities.c : common utilities not specific to precision
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.internal.h"

// clean up fftw's static plan memory
int liquid_fftwf_cleanup_wrapper(void)
{
#if HAVE_FFTW3_H && !defined LIQUID_FFTOVERRIDE
    fftwf_cleanup();
#endif
    return LIQUID_OK;
}

// determine best FFT method based on size
liquid_fft_method liquid_fft_estimate_method(unsigned int _nfft)
{
    if (_nfft == 0) {
        // invalid length
        liquid_error(LIQUID_EIRANGE,"liquid_fft_estimate_method(), fft size must be > 0");
        return LIQUID_FFT_METHOD_UNKNOWN;

    } else if (_nfft <= 8 || _nfft==11 || _nfft==13 || _nfft==16 || _nfft==17) {
        // use simple DFT
        return LIQUID_FFT_METHOD_DFT;

    } else if (fft_is_radix2(_nfft)) {
        // transform is of the form 2^m
#if 0
        // use radix-2 algorithm
        return LIQUID_FFT_METHOD_RADIX2;
#else
        // actually, prefer Cooley-Tukey algorithm
        return LIQUID_FFT_METHOD_MIXED_RADIX;
#endif

    } else if (liquid_is_prime(_nfft)) {
        // prefer Rader's alternate method (using radix-2 transform)
        // unless _nfft-1 is also radix2
        // TODO : also prefer Rader-I if _nfft-1 is mostly factors of 2
        if ( fft_is_radix2(_nfft-1) )
            return LIQUID_FFT_METHOD_RADER;
        else
            return LIQUID_FFT_METHOD_RADER2;
    }

    // last resort
    //return LIQUID_FFT_METHOD_DFT;         // use slow DFT method
    return LIQUID_FFT_METHOD_MIXED_RADIX;   // use mixed radix method
}

// is input radix-2?
int fft_is_radix2(unsigned int _n)
{
    // check to see if _n is radix 2
    unsigned int i;
    unsigned int d=0;
    unsigned int m=0;
    unsigned int t=_n;
    for (i=0; i<8*sizeof(unsigned int); i++) {
        d += (t & 1);           // count bits, radix-2 if d==1
        if (!m && (t&1)) m = i; // count lagging zeros, effectively log2(n)
        t >>= 1;
    }

    return (d == 1) ? 1 : 0;
}


// reverse _n-bit index _i
unsigned int fft_reverse_index(unsigned int _i, unsigned int _n)
{
    unsigned int j=0, k;
    for (k=0; k<_n; k++) {
        j <<= 1;
        j |= ( _i & 1 );
        _i >>= 1;
    }

    return j;
}


