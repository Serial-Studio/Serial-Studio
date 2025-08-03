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

// Symbol streaming generator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// internal structure
struct SYMSTREAM(_s) {
    int             filter_type;    // filter type (e.g. LIQUID_FIRFILT_RRC)
    unsigned int    k;              // samples/symbol
    unsigned int    m;              // filter semi-length
    float           beta;           // filter excess bandwidth
    int             mod_scheme;     // demodulator
    MODEM()         mod;            // modulator
    float           gain;           // gain before interpolation
    FIRINTERP()     interp;         // interpolator
    TO *            buf;            // output buffer
    unsigned int    buf_index;      // output buffer sample index
};

// create symstream object using default parameters
SYMSTREAM() SYMSTREAM(_create)()
{
    return SYMSTREAM(_create_linear)(LIQUID_FIRFILT_ARKAISER,
                                     2,     // samples/symbol
                                     7,     // filter delay
                                     0.3f,  // filter excess bandwidth
                                     LIQUID_MODEM_QPSK);
}

// create symstream object with linear modulation
//  _ftype  : filter type (e.g. LIQUID_FIRFILT_RRC)
//  _k      : samples per symbol
//  _m      : filter delay (symbols)
//  _beta   : filter excess bandwidth
//  _ms     : modulation scheme (e.g. LIQUID_MODEM_QPSK)
SYMSTREAM() SYMSTREAM(_create_linear)(int          _ftype,
                                      unsigned int _k,
                                      unsigned int _m,
                                      float        _beta,
                                      int          _ms)
{
    // validate input
    if (_k < 2)
        return liquid_error_config("symstream%s_create(), samples/symbol must be at least 2", EXTENSION);
    if (_m == 0)
        return liquid_error_config("symstream%s_create(), filter delay must be greater than zero", EXTENSION);
    if (_beta <= 0.0f || _beta > 1.0f)
        return liquid_error_config("symstream%s_create(), filter excess bandwidth must be in (0,1]", EXTENSION);
    if (_ms == LIQUID_MODEM_UNKNOWN || _ms >= LIQUID_MODEM_NUM_SCHEMES)
        return liquid_error_config("symstream%s_create(), invalid modulation scheme", EXTENSION);

    // allocate memory for main object
    SYMSTREAM() q = (SYMSTREAM()) malloc( sizeof(struct SYMSTREAM(_s)) );

    // set input parameters
    q->filter_type = _ftype;
    q->k           = _k;
    q->m           = _m;
    q->beta        = _beta;
    q->mod_scheme  = _ms;
    q->gain        = 1.0f;

    // modulator
    q->mod = MODEM(_create)(q->mod_scheme);

    // interpolator
    q->interp = FIRINTERP(_create_prototype)(q->filter_type, q->k, q->m, q->beta, 0);

    // sample buffer
    q->buf = (TO*) malloc(q->k*sizeof(TO));

    // reset and return main object
    SYMSTREAM(_reset)(q);
    return q;
}

// copy object
SYMSTREAM() SYMSTREAM(_copy)(SYMSTREAM() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("symstream%s_copy(), object cannot be NULL", EXTENSION);

    // create object and copy base parameters
    SYMSTREAM() q_copy = (SYMSTREAM()) malloc(sizeof(struct SYMSTREAM(_s)));
    memmove(q_copy, q_orig, sizeof(struct SYMSTREAM(_s)));

    // copy internal objects
    q_copy->mod    = MODEM    (_copy)(q_orig->mod   );
    q_copy->interp = FIRINTERP(_copy)(q_orig->interp);

    // copy output buffer state
    q_copy->buf = (TO *) liquid_malloc_copy(q_orig->buf, q_orig->k, sizeof(TO));

    return q_copy;
}

// destroy symstream object, freeing all internal memory
int SYMSTREAM(_destroy)(SYMSTREAM() _q)
{
    // destroy objects
    MODEM    (_destroy)(_q->mod);
    FIRINTERP(_destroy)(_q->interp);

    free(_q->buf);

    // free main object
    free(_q);
    return LIQUID_OK;
}

// print symstream object's parameters
int SYMSTREAM(_print)(SYMSTREAM() _q)
{
    printf("<liquid.symstream_%s", EXTENSION);
    printf(", k=%u", _q->k);
    printf(", m=%u", _q->m);
    printf(", beta=%.3f", _q->beta);
    printf(", ms=\"%s\"", modulation_types[_q->mod_scheme].name);
    printf(", gain=%g", _q->gain);
    printf(">\n");
    return LIQUID_OK;
}

// reset symstream internal state
int SYMSTREAM(_reset)(SYMSTREAM() _q)
{
    // reset objects and counter
    MODEM(_reset)(_q->mod);
    FIRINTERP(_reset)(_q->interp);
    _q->buf_index = 0;
    return LIQUID_OK;
}

// Get internal filter type
int SYMSTREAM(_get_ftype)(SYMSTREAM() _q)
{
    return _q->filter_type;
}

// Get internal signal bandwidth (symbol rate)
float SYMSTREAM(_get_k)(SYMSTREAM() _q)
{
    return _q->k;
}

// Get internal filter semi-length
unsigned int SYMSTREAM(_get_m)(SYMSTREAM() _q)
{
    return _q->m;
}

// Get internal filter excess bandwidth factor
float SYMSTREAM(_get_beta)(SYMSTREAM() _q)
{
    return _q->beta;
}

// Set internal linear modulation scheme, leaving the filter parameters
// (interpolator) unmodified
int SYMSTREAM(_set_scheme)(SYMSTREAM() _q,
                            int         _ms)
{
    _q->mod = MODEM(_recreate)(_q->mod, _ms);
    return LIQUID_OK;
}

// Get internal linear modulation scheme
int SYMSTREAM(_get_scheme)(SYMSTREAM() _q)
{
    return MODEM(_get_scheme)(_q->mod);
}

// Set internal linear gain (before interpolation)
int SYMSTREAM(_set_gain)(SYMSTREAM() _q,
                          float       _gain)
{
    _q->gain = _gain;
    return LIQUID_OK;
}

// Get internal linear gain (before interpolation)
float SYMSTREAM(_get_gain)(SYMSTREAM() _q)
{
    return _q->gain;
}

// Get delay in samples
unsigned int SYMSTREAM(_get_delay)(SYMSTREAM() _q)
{
    return _q->k*_q->m;
}

// fill buffer with samples
int SYMSTREAM(_fill_buffer)(SYMSTREAM() _q)
{
    // generate random symbol
    unsigned int sym = MODEM(_gen_rand_sym)(_q->mod);

    // modulate
    TO v;
    MODEM(_modulate)(_q->mod, sym, &v);

    // apply gain
    v *= _q->gain;

    // interpolate
    FIRINTERP(_execute)(_q->interp, v, _q->buf);
    return LIQUID_OK;
}

// write block of samples to output buffer
//  _q      : synchronizer object
//  _buf    : output buffer [size: _buf_len x 1]
//  _buf_len: output buffer size
int SYMSTREAM(_write_samples)(SYMSTREAM()  _q,
                              TO *         _buf,
                              unsigned int _buf_len)
{
    unsigned int i;
    for (i=0; i<_buf_len; i++) {
        // check to see if buffer needs samples
        if (_q->buf_index==0) {
            if (SYMSTREAM(_fill_buffer)(_q))
                return liquid_error(LIQUID_EINT,"symstream%s_write_samples(), could not fill internal buffer\n", EXTENSION);
        }

        // write output sample from internal buffer
        _buf[i] = _q->buf[_q->buf_index];

        // increment internal index
        _q->buf_index = (_q->buf_index + 1) % _q->k;
    }
    return LIQUID_OK;
}

