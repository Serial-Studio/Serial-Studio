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

// M-ary frequency-shift keying demodulator

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "liquid.internal.h"

#define DEBUG_FSKDEM 0

// fskdem
struct fskdem_s {
    // common
    unsigned int    m;          // bits per symbol
    unsigned int    k;          // samples per symbol
    float           bandwidth;  // filter bandwidth parameter

    // derived values
    unsigned int    M;          // constellation size
    float           M2;         // (M-1)/2
    unsigned int    K;          // FFT size
    float complex * buf_time;   // FFT input buffer
    float complex * buf_freq;   // FFT output buffer
    FFT_PLAN        fft;        // FFT object
    unsigned int *  demod_map;  // demodulation map

    // state variables
    unsigned int    s_demod;    // demodulated symbol (used for frequency error)
};

// create fskdem object (frequency demodulator)
//  _m          :   bits per symbol, _m > 0
//  _k          :   samples/symbol, _k >= 2^_m
//  _bandwidth  :   total signal bandwidth, (0,0.5)
fskdem fskdem_create(unsigned int _m,
                     unsigned int _k,
                     float        _bandwidth)
{
    // validate input
    if (_m == 0)
        return liquid_error_config("fskdem_create(), bits/symbol must be greater than 0");
    if (_k < 2 || _k > 2048)
        return liquid_error_config("fskdem_create(), samples/symbol must be in [2^_m, 2048]");
    if (_bandwidth <= 0.0f || _bandwidth >= 0.5f)
        return liquid_error_config("fskdem_create(), bandwidth must be in (0,0.5)");

    // create main object memory
    fskdem q = (fskdem) malloc(sizeof(struct fskdem_s));

    // set basic internal properties
    q->m         = _m;              // bits per symbol
    q->k         = _k;              // samples per symbol
    q->bandwidth = _bandwidth;      // signal bandwidth

    // derived values
    q->M  = 1 << q->m;              // constellation size
    q->M2 = 0.5f*(float)(q->M-1);   // (M-1)/2

    // compute demodulation FFT size such that FFT output bin frequencies are
    // as close to modulated frequencies as possible
    float        df = q->bandwidth / q->M2;         // frequency spacing
    float        err_min = 1e9f;                    // minimum error value
    unsigned int K_min = q->k;                      // minimum FFT size
    unsigned int K_max = q->k*4 < 16 ? 16 : q->k*4; // maximum FFT size
    unsigned int K_hat;
    for (K_hat=K_min; K_hat<=K_max; K_hat++) {
        // compute candidate FFT size
        float v     = 0.5f*df*(float)K_hat;         // bin spacing
        float err = fabsf( roundf(v) - v );         // fractional bin spacing

#if DEBUG_FSKDEM
        // print results
        printf("  K_hat = %4u : v = %12.8f, err=%12.8f %s\n", K_hat, v, err, err < err_min ? "*" : "");
#endif

        // save best result
        if (K_hat==K_min || err < err_min) {
            q->K    = K_hat;
            err_min = err;
        }

        // perfect match; no need to continue searching
        if (err < 1e-6f)
            break;
    }
    
    // determine demodulation mapping between tones and frequency bins
    // TODO: use gray coding
    q->demod_map = (unsigned int *) malloc(q->M * sizeof(unsigned int));
    unsigned int i;
    for (i=0; i<q->M; i++) {
        // print frequency bins
        float freq = ((float)i - q->M2) * q->bandwidth / q->M2;
        float idx  = freq * (float)(q->K);
        unsigned int index = (unsigned int) (idx < 0 ? roundf(idx + q->K) : roundf(idx));
        q->demod_map[i] = index;
#if DEBUG_FSKDEM
        printf("  s=%3u, f = %12.8f, index=%3u\n", i, freq, index);
#endif
    }

    // check for uniqueness
    for (i=1; i<q->M; i++) {
        if (q->demod_map[i] == q->demod_map[i-1]) {
            liquid_error(LIQUID_EICONFIG,"fskdem_create(), demod map is not unique; consider increasing bandwidth");
            break;
        }
    }

    // allocate memory for transform
    q->buf_time = (float complex*) FFT_MALLOC(q->K * sizeof(float complex));
    q->buf_freq = (float complex*) FFT_MALLOC(q->K * sizeof(float complex));
    q->fft = FFT_CREATE_PLAN(q->K, q->buf_time, q->buf_freq, FFT_DIR_FORWARD, 0);

    // reset modem object
    fskdem_reset(q);

    // return main object
    return q;
}

// copy object
fskdem fskdem_copy(fskdem q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("fskdem_copy(), object cannot be NULL");

    // create object and copy base parameters
    fskdem q_copy = (fskdem) malloc(sizeof(struct fskdem_s));
    memmove(q_copy, q_orig, sizeof(struct fskdem_s));

    // allocate memory for transform
    q_copy->buf_time = (float complex*) FFT_MALLOC(q_copy->K * sizeof(float complex));
    q_copy->buf_freq = (float complex*) FFT_MALLOC(q_copy->K * sizeof(float complex));
    q_copy->fft = FFT_CREATE_PLAN(q_copy->K, q_copy->buf_time, q_copy->buf_freq, FFT_DIR_FORWARD, 0);

    // copy internal time and frequency buffers
    memmove(q_copy->buf_time, q_orig->buf_time, q_copy->K * sizeof(float complex));
    memmove(q_copy->buf_freq, q_orig->buf_freq, q_copy->K * sizeof(float complex));

    // copy demodulation map
    q_copy->demod_map = (unsigned int*)liquid_malloc_copy(q_orig->demod_map, q_copy->M, sizeof(unsigned int));

    // return new object
    return q_copy;
}

// destroy fskdem object
int fskdem_destroy(fskdem _q)
{
    // free allocated arrays
    free(_q->demod_map);
    FFT_FREE(_q->buf_time);
    FFT_FREE(_q->buf_freq);
    FFT_DESTROY_PLAN(_q->fft);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print fskdem object internals
int fskdem_print(fskdem _q)
{
    printf("<liquid.fskdem");
    printf(", bits/symbol=%u", _q->m);
    printf(", samples/symbol=%u", _q->k);
    printf(", bandwidth=%g", _q->bandwidth);
    printf(">\n");
    return LIQUID_OK;
}

// reset state
int fskdem_reset(fskdem _q)
{
    // reset time and frequency buffers
    unsigned int i;
    for (i=0; i<_q->K; i++) {
        _q->buf_time[i] = 0.0f;
        _q->buf_freq[i] = 0.0f;
    }

    // clear state variables
    _q->s_demod = 0;
    return LIQUID_OK;
}

// demodulate symbol, assuming perfect symbol timing
//  _q      :   fskdem object
//  _y      :   input sample array [size: _k x 1]
unsigned int fskdem_demodulate(fskdem          _q,
                               float complex * _y)
{
    // copy input to internal time buffer
    memmove(_q->buf_time, _y, _q->k*sizeof(float complex));

    // compute transform, storing result in 'buf_freq'
    FFT_EXECUTE(_q->fft);

    // find maximum by looking at particular bins
    float        vmax  = 0;
    unsigned int s     = 0;

    // run search
    for (s=0; s<_q->M; s++) {
        float v = cabsf( _q->buf_freq[_q->demod_map[s]] );
        if (s==0 || v > vmax) {
            // save optimal output symbol
            _q->s_demod = s;

            // save peak FFT bin value
            vmax = v;
        }
    }

    // save best result
    return _q->s_demod;
}

// get demodulator frequency error
float fskdem_get_frequency_error(fskdem _q)
{
    // get index of peak bin
    //unsigned int index = _q->buf_freq[ _q->s_demod ];

    // extract peak value of previous, post FFT index
    float vm = cabsf(_q->buf_freq[(_q->s_demod+_q->K-1)%_q->K]);  // previous
    float v0 = cabsf(_q->buf_freq[ _q->s_demod               ]);  // peak
    float vp = cabsf(_q->buf_freq[(_q->s_demod+      1)%_q->K]);  // post

    // compute derivative
    // TODO: compensate for bin spacing
    // TODO: just find peak using polynomial interpolation
    return (vp - vm) / v0;
}

// get energy for a particular symbol within a certain range
float fskdem_get_symbol_energy(fskdem       _q,
                               unsigned int _s,
                               unsigned int _range)
{
    // validate input
    if (_s >= _q->M) {
        liquid_error(LIQUID_EICONFIG,"fskdem_get_symbol_energy(), input symbol (%u) exceeds maximum (%u)",
                _s, _q->M);
        _s = 0;
    }

    if (_range > _q->K)
        _range = _q->K;

    // map input symbol to FFT bin
    unsigned int index = _q->demod_map[_s];

    // compute energy around FFT bin
    float complex v = _q->buf_freq[index];
    float energy = crealf(v)*crealf(v) + cimagf(v)*cimagf(v);
    int i;
    for (i=0; i<_range; i++) {
        // positive negative indices
        unsigned int i0 = (index         + i) % _q->K;
        unsigned int i1 = (index + _q->K - i) % _q->K;

        float complex v0 = _q->buf_freq[i0];
        float complex v1 = _q->buf_freq[i1];

        energy += crealf(v0)*crealf(v0) + cimagf(v0)*cimagf(v0);
        energy += crealf(v1)*crealf(v1) + cimagf(v1)*cimagf(v1);
    }

    return energy;
}

