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

// Symbol streaming generator with arbitrary sample rate

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// internal structure
struct SYMSTREAMR(_s) {
    SYMSTREAM()     symstream;      // half-band interpolator
    MSRESAMP()      resamp;         // arbitrary rate resampler
    TO *            buf;            // resampled output buffer
    unsigned int    buf_len;        // resampled output buffer: num allocated
    unsigned int    buf_size;       // resampled output buffer: num elements
    unsigned int    buf_index;      // resampled output buffer: sample index
};

// create symstream object using default parameters
SYMSTREAMR() SYMSTREAMR(_create)()
{
    return SYMSTREAMR(_create_linear)(LIQUID_FIRFILT_ARKAISER,
                                      0.5f,  // samples/symbol
                                      7,     // filter delay
                                      0.3f,  // filter excess bandwidth
                                      LIQUID_MODEM_QPSK);
}

// create symstream object with linear modulation
//  _ftype  : filter type (e.g. LIQUID_FIRFILT_RRC)
//  _bw     : relative signal bandwidth, 0.001 <= _bw <= 1.0
//  _m      : filter delay (symbols)
//  _beta   : filter excess bandwidth
//  _ms     : modulation scheme (e.g. LIQUID_MODEM_QPSK)
SYMSTREAMR() SYMSTREAMR(_create_linear)(int          _ftype,
                                        float        _bw,
                                        unsigned int _m,
                                        float        _beta,
                                        int          _ms)
{
    float bw_min = 0.001f;
    float bw_max = 1.000f;
    // validate input
    if (_bw < bw_min || _bw > bw_max)
        return liquid_error_config("symstreamr%s_create(), symbol bandwidth (%g) must be in [%g,%g]", EXTENSION, _bw, bw_min, bw_max);

    // create base streaming object
    symstreamcf s = SYMSTREAM(_create_linear)(_ftype, 2, _m, _beta, _ms);
    if (s == NULL)
        return liquid_error_config("symstreamr%s_create(), could not create streaming object", EXTENSION);

    // allocate memory for main object
    SYMSTREAMR() q = (SYMSTREAMR()) malloc( sizeof(struct SYMSTREAMR(_s)) );
    q->symstream = s;

    // create arbitrary rate resampler
    float rate = 0.5f / _bw;
    q->resamp = MSRESAMP(_create)(rate, 60.0f);

    // sample buffer
    // NOTE: buffer size must be at least 2^nextpow2( 0.5/bw )
    q->buf_len = 1 << liquid_nextpow2((unsigned int)ceilf(0.5f/_bw));
    q->buf = (TO*) malloc(q->buf_len*sizeof(TO));

    // reset and return main object
    SYMSTREAMR(_reset)(q);
    return q;
}

// copy object
SYMSTREAMR() SYMSTREAMR(_copy)(SYMSTREAMR() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("symstreamr%s_copy(), object cannot be NULL", EXTENSION);

    // create filter object and copy base parameters
    SYMSTREAMR() q_copy = (SYMSTREAMR()) malloc(sizeof(struct SYMSTREAMR(_s)));
    memmove(q_copy, q_orig, sizeof(struct SYMSTREAMR(_s)));

    // copy internal objects
    q_copy->symstream = SYMSTREAM(_copy)(q_orig->symstream);
    q_copy->resamp    = MSRESAMP (_copy)(q_orig->resamp   );

    // copy internal buffer
    q_copy->buf = (TO *) liquid_malloc_copy(q_orig->buf, q_orig->buf_len, sizeof(TO));

    return q_copy;
}

// destroy symstream object, freeing all internal memory
int SYMSTREAMR(_destroy)(SYMSTREAMR() _q)
{
    // destroy objects
    SYMSTREAM(_destroy)(_q->symstream);
    MSRESAMP(_destroy)(_q->resamp);

    // free buffer
    free(_q->buf);

    // free main object
    free(_q);
    return LIQUID_OK;
}

// print symstream object's parameters
int SYMSTREAMR(_print)(SYMSTREAMR() _q)
{
    printf("<liquid.symstreamr%s, ftype\"%s\", bw=%.6f, m=%u, beta=%.3f, ms=\"%s\", gain=%g>\n",
        EXTENSION,
        liquid_firfilt_type_str[SYMSTREAMR(_get_ftype)(_q)][0],
        SYMSTREAMR(_get_bw)  (_q),
        SYMSTREAMR(_get_m)   (_q),
        SYMSTREAMR(_get_beta)(_q),
        modulation_types[SYMSTREAMR(_get_scheme)(_q)].name,
        SYMSTREAMR(_get_gain)(_q));
    return LIQUID_OK;
}

// reset symstream internal state
int SYMSTREAMR(_reset)(SYMSTREAMR() _q)
{
    // reset objects and counter
    SYMSTREAM(_reset)(_q->symstream);
    MSRESAMP (_reset)(_q->resamp);
    _q->buf_size  = 0;
    _q->buf_index = 0;
    return LIQUID_OK;
}

// Get internal filter type
int SYMSTREAMR(_get_ftype)(SYMSTREAMR() _q)
{
    return SYMSTREAM(_get_ftype)(_q->symstream);
}

// Get internal signal bandwidth (symbol rate)
float SYMSTREAMR(_get_bw)(SYMSTREAMR() _q)
{
    return 1.0f / (MSRESAMP(_get_rate)(_q->resamp) * (float)SYMSTREAM(_get_k)(_q->symstream));
}

// Get internal filter semi-length
unsigned int SYMSTREAMR(_get_m)(SYMSTREAMR() _q)
{
    return SYMSTREAM(_get_m)(_q->symstream);
}

// Get internal filter excess bandwidth factor
float SYMSTREAMR(_get_beta)(SYMSTREAMR() _q)
{
    return SYMSTREAM(_get_beta)(_q->symstream);
}

// Set internal linear modulation scheme, leaving the filter parameters
// (interpolator) unmodified
int SYMSTREAMR(_set_scheme)(SYMSTREAMR() _q,
                            int         _ms)
{
    return SYMSTREAM(_set_scheme)(_q->symstream, _ms);
}

// Get internal linear modulation scheme
int SYMSTREAMR(_get_scheme)(SYMSTREAMR() _q)
{
    return SYMSTREAM(_get_scheme)(_q->symstream);
}

// Set internal linear gain (before interpolation)
int SYMSTREAMR(_set_gain)(SYMSTREAMR() _q,
                          float       _gain)
{
    return SYMSTREAM(_set_gain)(_q->symstream, _gain);
}

// Get internal linear gain (before interpolation)
float SYMSTREAMR(_get_gain)(SYMSTREAMR() _q)
{
    return SYMSTREAM(_get_gain)(_q->symstream);
}

// Get delay in samples
float SYMSTREAMR(_get_delay)(SYMSTREAMR() _q)
{
    float p = SYMSTREAM(_get_delay)(_q->symstream);
    float d = MSRESAMP (_get_delay)(_q->resamp   );
    float r = MSRESAMP (_get_rate) (_q->resamp   );
    return (p + d)*r;
}

// fill buffer with samples
int SYMSTREAMR(_fill_buffer)(SYMSTREAMR() _q)
{
    if (_q->buf_index != _q->buf_size)
        return liquid_error(LIQUID_EINT,"symstreamr%s_write_samples(), buffer not empty\n", EXTENSION);

    // reset counters
    _q->buf_size  = 0;
    _q->buf_index = 0;

    // continue running until at least one sample is generated
    while (!_q->buf_size) {
        // generate base sample
        TO sample;
        SYMSTREAM(_write_samples)(_q->symstream, &sample, 1);

        // resample
        MSRESAMP(_execute)(_q->resamp, &sample, 1, _q->buf, &_q->buf_size);
    }
    return LIQUID_OK;
}

// write block of samples to output buffer
//  _q      : synchronizer object
//  _buf    : output buffer [size: _buf_len x 1]
//  _buf_len: output buffer size
int SYMSTREAMR(_write_samples)(SYMSTREAMR()  _q,
                              TO *         _buf,
                              unsigned int _buf_len)
{
    unsigned int i;
    for (i=0; i<_buf_len; i++) {
        // check to see if buffer needs samples
        if (_q->buf_index==_q->buf_size) {
            if (SYMSTREAMR(_fill_buffer)(_q))
                return liquid_error(LIQUID_EINT,"symstreamr%s_write_samples(), could not fill internal buffer\n", EXTENSION);
        }

        // write output sample from internal buffer
        _buf[i] = _q->buf[_q->buf_index];

        // increment internal index
        _q->buf_index++;
    }
    return LIQUID_OK;
}

