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
// asgram (ASCII spectrogram)
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <complex.h>
#include "liquid.internal.h"

struct ASGRAM(_s) {
    unsigned int nfft;          // transform size (display)
    unsigned int nfftp;         // transform size (processing)
    unsigned int p;             // over-sampling rate
    SPGRAM()     periodogram;   // spectral periodogram object
    TC *         X;             // spectral periodogram output
    float *      psd;           // power spectral density

    float        levels[10];    // threshold for signal levels
    char         levelchar[10]; // characters representing levels
    unsigned int num_levels;    // number of levels
    float        div;           // dB per division
    float        ref;           // dB reference value
};

// create asgram object with size _nfft
ASGRAM() ASGRAM(_create)(unsigned int _nfft)
{
    // validate input
    if (_nfft < 2)
        return liquid_error_config("asgram%s_create(), fft size must be at least 2", EXTENSION);

    // create main object
    ASGRAM() q = (ASGRAM()) malloc(sizeof(struct ASGRAM(_s)));

    q->nfft  = _nfft;

    // derived values
    q->p     = 4;   // over-sampling rate
    q->nfftp = q->nfft * q->p;

    // allocate memory for PSD estimate
    q->X   = (TC *   ) malloc((q->nfftp)*sizeof(TC)   );
    q->psd = (float *) malloc((q->nfftp)*sizeof(float));

    // create spectral periodogram object
    q->periodogram = SPGRAM(_create)(q->nfftp,LIQUID_WINDOW_HANN,q->nfft,q->nfft/2);

    // power spectral density levels
    q->num_levels = 10;
    ASGRAM(_set_display)(q," .,-+*&NM#");
    ASGRAM(_set_scale)(q, 0.0f, 10.0f);

    return q;
}

// copy object
ASGRAM() ASGRAM(_copy)(ASGRAM() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("spgram%s_copy(), object cannot be NULL", EXTENSION);

    // allocate memory for main object
    ASGRAM() q_copy = (ASGRAM()) malloc(sizeof(struct ASGRAM(_s)));

    // copy all internal memory
    memmove(q_copy, q_orig, sizeof(struct ASGRAM(_s)));

    // create periodogram object
    q_copy->periodogram = SPGRAM(_copy)(q_orig->periodogram);

    // allocate and copy memory arrays
    q_copy->X   = (TC *   ) malloc((q_copy->nfftp)*sizeof(TC)   );
    q_copy->psd = (float *) malloc((q_copy->nfftp)*sizeof(float));
    memmove(q_copy->X,   q_orig->X,   q_copy->nfftp*sizeof(TC));
    memmove(q_copy->psd, q_orig->psd, q_copy->nfftp*sizeof(float));

    // return copied object
    return q_copy;
}

// destroy asgram object
int ASGRAM(_destroy)(ASGRAM() _q)
{
    // destroy spectral periodogram object
    SPGRAM(_destroy)(_q->periodogram);

    // free PSD estimate array
    free(_q->X);
    free(_q->psd);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// resets the internal state of the asgram object
int ASGRAM(_reset)(ASGRAM() _q)
{
    return SPGRAM(_reset)(_q->periodogram);
}

// set scale and offset for spectrogram
//  _q      :   asgram object
//  _ref    :   signal reference level [dB]
//  _div    :   signal division [dB]
int ASGRAM(_set_scale)(ASGRAM() _q,
                       float    _ref,
                       float    _div)
{
    if (_div <= 0.0f)
        return liquid_error(LIQUID_EICONFIG,"asgram%s_set_scale(), div must be greater than zero", EXTENSION);

    _q->ref = _ref;
    _q->div = _div;

    unsigned int i;
    for (i=0; i<_q->num_levels; i++)
        _q->levels[i] = _q->ref + i*_q->div;
    return LIQUID_OK;
}

// set display characters for output string
//  _q      :   asgram object
//  _ascii  :   10-character display, default: " .,-+*&NM#"
int ASGRAM(_set_display)(ASGRAM()     _q,
                         const char * _ascii)
{
    unsigned int i;
    for (i=0; i<10; i++) {
        if (_ascii[i] == '\0') {
            liquid_error(LIQUID_EICONFIG,"asgram%s_display(), invalid use of null character", EXTENSION);
            _q->levelchar[i] = '?';
        } else {
            _q->levelchar[i] = _ascii[i];
        }
    }
    return LIQUID_OK;
}

// push a single sample into the asgram object
//  _q      :   asgram object
//  _x      :   input buffer [size: _n x 1]
//  _n      :   input buffer length
int ASGRAM(_push)(ASGRAM() _q,
                  TI       _x)
{
    // push sample into internal spectral periodogram
    return SPGRAM(_push)(_q->periodogram, _x);
}

// write a block of samples to the asgram object
//  _q      :   asgram object
//  _x      :   input buffer [size: _n x 1]
//  _n      :   input buffer length
int ASGRAM(_write)(ASGRAM()     _q,
                   TI *         _x,
                   unsigned int _n)
{
    // write samples to internal spectral periodogram
    return SPGRAM(_write)(_q->periodogram, _x, _n);
}

// compute spectral periodogram output from current buffer contents
//  _q          :   ascii spectrogram object
//  _ascii      :   output ASCII string [size: _nfft x 1]
//  _peakval    :   value at peak (returned value)
//  _peakfreq   :   frequency at peak (returned value)
int ASGRAM(_execute)(ASGRAM() _q,
                     char *   _ascii,
                     float *  _peakval,
                     float *  _peakfreq)
{
    // check number of transforms
    if (SPGRAM(_get_num_transforms)(_q->periodogram)==0) {
        memset(_ascii,' ',_q->nfft);
        *_peakval = 0.0f;
        *_peakfreq = 0.0f;
        return LIQUID_OK;
    }

    // execute spectral periodogram
    SPGRAM(_get_psd)(_q->periodogram, _q->psd);
    SPGRAM(_reset)(_q->periodogram);

    unsigned int i;
    unsigned int j;
    // find peak
    for (i=0; i<_q->nfftp; i++) {
        if (i==0 || _q->psd[i] > *_peakval) {
            *_peakval = _q->psd[i];
            *_peakfreq = (float)(i) / (float)(_q->nfftp) - 0.5f;
        }
    }

    // down-sample from nfft*p frequency bins to just nfft by retaining
    // one value (e.g. maximum or average) over range.
    for (i=0; i<_q->nfft; i++) {
#if 0
        // find maximum within 'p' samples
        float psd_val = 0.0f;
        for (j=0; j<_q->p; j++) {
            unsigned int index = (_q->p*i) + j;
            psd_val = (j==0 || _q->psd[index] > psd_val) ? _q->psd[index] : psd_val;
        }
#else
        // find average over 'p' samples
        float psd_val = 0.0f;
        for (j=0; j<_q->p; j++)
            psd_val += _q->psd[(_q->p*i) + j];
        psd_val /= (float)(_q->p);
#endif

        // determine ascii level (which character to use)
        _ascii[i] = _q->levelchar[0];
        for (j=0; j<_q->num_levels; j++) {
            if ( psd_val > _q->levels[j] )
                _ascii[i] = _q->levelchar[j];
        }
    }

    // append null character to end of string
    //_ascii[i] = '\0';
    return LIQUID_OK;
}

// compute spectral periodogram output from current buffer
// contents and print standard format to stdout
int ASGRAM(_print)(ASGRAM() _q)
{
    float maxval;
    float maxfreq;
    char ascii[_q->nfft+1];
    memset(ascii, '\0', _q->nfft+1); // fill buffer with null characters
        
    // execute the spectrogram
    ASGRAM(_execute)(_q, ascii, &maxval, &maxfreq);

    // print the spectrogram to stdout
    printf(" > %s < pk%5.1f dB [%5.2f]\n", ascii, maxval, maxfreq);
    return LIQUID_OK;
}

