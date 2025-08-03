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

// DS/SS frame generator with fixed fields: 8-byte header and 64-byte payload

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <complex.h>

#include "liquid.internal.h"

// write samples to buffer
int dsssframe64gen_write(dsssframe64gen _q, float complex * _buf);

struct dsssframe64gen_s {
    unsigned int    m;                  // filter delay (symbols)
    float           beta;               // filter excess bandwidth factor
    unsigned int    sf;                 // spreading factor (fixed)
    // objects
    qpacketmodem    enc;                // packet encoder/modulator
    qpilotgen       pilotgen;           // pilot symbol generator
    msequence       ms;                 // spreading sequence generator
    firinterp_crcf  interp;             // pulse-shaping filter/interpolator
    // buffers
    float complex   preamble_pn[1024];  // 1024-symbol p/n sequence
    unsigned char   payload_dec[ 150];  // 600 = 150 bytes * 8 bits/bytes / 2 bits/symbol
    float complex   payload_sym[ 600];  // modulated payload symbols
    float complex   payload_tx [ 650];  // modulated payload symbols with pilots
};

// create dsssframe64gen object
dsssframe64gen dsssframe64gen_create()
{
    dsssframe64gen q = (dsssframe64gen) malloc(sizeof(struct dsssframe64gen_s));
    q->m    = 15;
    q->beta = 0.20f;
    q->sf   =  80;  // spreading factor

    unsigned int i;

    // generate p/n sequence
    q->ms = msequence_create(11, 0x0500, 1);
    for (i=0; i<1024; i++) {
        q->preamble_pn[i]  = (msequence_advance(q->ms) ? M_SQRT1_2 : -M_SQRT1_2);
        q->preamble_pn[i] += (msequence_advance(q->ms) ? M_SQRT1_2 : -M_SQRT1_2)*_Complex_I;
    }

    // create payload encoder/modulator object
    int check      = LIQUID_CRC_24;
    int fec0       = LIQUID_FEC_NONE;
    int fec1       = LIQUID_FEC_GOLAY2412;
    int mod_scheme = LIQUID_MODEM_QPSK;
    q->enc         = qpacketmodem_create();
    qpacketmodem_configure(q->enc, 72, check, fec0, fec1, mod_scheme);
    //qpacketmodem_print(q->enc);
    assert( qpacketmodem_get_frame_len(q->enc)==600 );

    // create pilot generator
    q->pilotgen = qpilotgen_create(600, 13);
    assert( qpilotgen_get_frame_len(q->pilotgen)==650 );

    // create pulse-shaping filter (k=2)
    q->interp = firinterp_crcf_create_prototype(LIQUID_FIRFILT_ARKAISER,2,q->m,q->beta,0);

    // return main object
    return q;
}

// copy object
dsssframe64gen dsssframe64gen_copy(dsssframe64gen q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("dsssframe64gen_copy(), object cannot be NULL");

    // create filter object and copy base parameters
    dsssframe64gen q_copy = (dsssframe64gen) malloc(sizeof(struct dsssframe64gen_s));
    memmove(q_copy, q_orig, sizeof(struct dsssframe64gen_s));

    // copy objects
    q_copy->enc      = qpacketmodem_copy  (q_orig->enc     );
    q_copy->pilotgen = qpilotgen_copy     (q_orig->pilotgen);
    q_copy->ms       = msequence_copy     (q_orig->ms      );
    q_copy->interp   = firinterp_crcf_copy(q_orig->interp  );
    return q_copy;
}

// destroy dsssframe64gen object
int dsssframe64gen_destroy(dsssframe64gen _q)
{
    // destroy internal objects
    msequence_destroy     (_q->ms);
    qpacketmodem_destroy  (_q->enc);
    qpilotgen_destroy     (_q->pilotgen);
    firinterp_crcf_destroy(_q->interp);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print dsssframe64gen object internals
int dsssframe64gen_print(dsssframe64gen _q)
{
    printf("<liquid.dsssframe64gen, m=%u, beta=%4.2f>\n", _q->m, _q->beta);
    return LIQUID_OK;
}

// get full frame length [samples]
unsigned int dsssframe64gen_get_frame_len(dsssframe64gen _q)
{
    // add a small amount of zero-padding to ensure entire frame gets pushed through synchronizer
    return 2*(1024 + 650*_q->sf + 2*_q->m) + 64;
}

// generate a frame
//  _q          :   frame generator object
//  _header     :   8-byte header data, NULL for random
//  _payload    :   64-byte payload data, NULL for random
//  _frame      :   output frame samples, [size: dsssframegen64gen_get_frame_len() x 1]
int dsssframe64gen_execute(dsssframe64gen        _q,
                           const unsigned char * _header,
                           const unsigned char * _payload,
                           float complex *       _buf)
{
    unsigned int i;

    // concatenate header and payload
    for (i=0; i<8; i++)
        _q->payload_dec[i] = _header==NULL ? rand() & 0xff : _header[i];
    for (i=0; i<64; i++)
        _q->payload_dec[i+8] = _payload==NULL ? rand() & 0xff : _payload[i];

    // run packet encoder and modulator
    qpacketmodem_encode(_q->enc, _q->payload_dec, _q->payload_sym);

    // add pilot symbols
    qpilotgen_execute(_q->pilotgen, _q->payload_sym, _q->payload_tx);

    // reset objects
    firinterp_crcf_reset(_q->interp);
    msequence_reset(_q->ms);

    // write frame to buffer
    return dsssframe64gen_write(_q, _buf);
}

// write samples to buffer
//  _q      : frame generator object
//  _buf    : output frame samples (full frame)
int dsssframe64gen_write(dsssframe64gen  _q,
                         float complex * _buf)
{
    unsigned int i, j, n=0;

    // p/n sequence
    for (i=0; i<1024; i++) {
        firinterp_crcf_execute(_q->interp, _q->preamble_pn[i], &_buf[n]);
        n+=2;
    }

    // frame payload
    for (i=0; i<650; i++) {
        float complex sym = _q->payload_tx[i]; // strip out raw payload symbol
        for (j=0; j<_q->sf; j++) {
            // generate pseudo-random symbol
            unsigned int  p = msequence_generate_symbol(_q->ms, 2);
            float complex s = cexpf(_Complex_I*2*M_PI*(float)p/(float)4);
            firinterp_crcf_execute(_q->interp, sym*s, &_buf[n]);
            n+=2;
        }
    }

    // interpolator settling
    for (i=0; i<2*_q->m; i++) {
        firinterp_crcf_execute(_q->interp, 0.0f, &_buf[n]);
        n+=2;
    }

    // zero-pad end to ensure synchronizer can receive full payload
    while (n < dsssframe64gen_get_frame_len(_q))
        _buf[n++] = 0;

    return LIQUID_OK;
}

