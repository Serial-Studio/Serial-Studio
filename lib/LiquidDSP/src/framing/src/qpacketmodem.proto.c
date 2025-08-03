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

// qpacketmodem: convenient modulator/demodulator and packet encoder/decoder combination

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <complex.h>

#include "liquid.internal.h"

struct QPACKETMODEM(_s) {
    // properties
    MODEM()         mod_payload;        // payload modulator/demodulator
    packetizer      p;                  // packet encoder/decoder
    unsigned int    bits_per_symbol;    // modulator bits/symbol
    unsigned int    payload_dec_len;    // number of decoded payload bytes
    unsigned char * payload_enc;        // payload data (encoded bytes)
    unsigned char * payload_mod;        // payload symbols (modulator output, demod input)
    unsigned int    payload_enc_len;    // number of encoded payload bytes
    unsigned int    payload_bit_len;    // number of bits in encoded payload
    unsigned int    payload_mod_len;    // number of symbols in encoded payload
    unsigned int    n;                  // index into partially-received payload data
    float           evm;                // estimated error vector magnitude
};

// create packet encoder
QPACKETMODEM() QPACKETMODEM(_create)()
{
    // allocate memory for main object
    QPACKETMODEM() q = (QPACKETMODEM()) malloc(sizeof(struct QPACKETMODEM(_s)));

    // create payload modem (initially QPSK, overridden by properties)
    q->mod_payload = MODEM(_create)(LIQUID_MODEM_QPSK);
    q->bits_per_symbol = 2;

    // initial memory allocation for payload
    q->payload_dec_len = 1;
    q->p = packetizer_create(q->payload_dec_len,
                             LIQUID_CRC_NONE,
                             LIQUID_FEC_NONE,
                             LIQUID_FEC_NONE);

    // number of bytes in encoded payload
    q->payload_enc_len = packetizer_get_enc_msg_len(q->p);

    // number of bits in encoded payload
    q->payload_bit_len = 8*q->payload_enc_len;

    // number of symbols in encoded payload
    div_t d = div(q->payload_bit_len, q->bits_per_symbol);
    q->payload_mod_len = d.quot + (d.rem ? 1 : 0);

    // soft demodulator uses one byte to represent each soft bit
    q->payload_enc = (unsigned char*) malloc(q->bits_per_symbol*q->payload_mod_len*sizeof(unsigned char));

    // set symbol length appropriately
    q->payload_mod_len = q->payload_enc_len * q->bits_per_symbol;   // for QPSK
    q->payload_mod = (unsigned char*) malloc(q->payload_mod_len*sizeof(unsigned char));

    q->n = 0;

    // return pointer to main object
    return q;
}

// copy object
QPACKETMODEM() QPACKETMODEM(_copy)(QPACKETMODEM() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("qpacketmodem_copy(), object cannot be NULL");

    // create new object
    QPACKETMODEM() q_copy = QPACKETMODEM(_create)();

    // configure identically as original
    unsigned int payload_len = q_orig->payload_dec_len;
    crc_scheme   check       = packetizer_get_crc (q_orig->p);
    fec_scheme   fec0        = packetizer_get_fec0(q_orig->p);
    fec_scheme   fec1        = packetizer_get_fec1(q_orig->p);
    int          ms          = MODEM(_get_scheme )(q_orig->mod_payload);
    QPACKETMODEM(_configure)(q_copy, payload_len, check, fec0, fec1, ms);

    // return new object
    return q_copy;
}


// destroy object, freeing all internal arrays
int QPACKETMODEM(_destroy)(QPACKETMODEM() _q)
{
    // free objects
    packetizer_destroy(_q->p);
    MODEM(_destroy)(_q->mod_payload);

    // free arrays
    free(_q->payload_enc);
    free(_q->payload_mod);

    free(_q);
    return LIQUID_OK;
}

// reset object
int QPACKETMODEM(_reset)(QPACKETMODEM() _q)
{
    return MODEM(_reset)(_q->mod_payload);
}

// print object internals
int QPACKETMODEM(_print)(QPACKETMODEM() _q)
{
    printf("<liquid.qpacketmodem");
    printf(", check=\"%s\"", crc_scheme_str[packetizer_get_crc(_q->p)][0]);
    printf(", fec_0=\"%s\"", fec_scheme_str[packetizer_get_fec0(_q->p)][0]);
    printf(", fec_1=\"%s\"", fec_scheme_str[packetizer_get_fec1(_q->p)][0]);
    printf(", ms=\"%s\"", modulation_types[MODEM(_get_scheme)(_q->mod_payload)].name);
    printf(", dec=%u", _q->payload_dec_len);
    printf(", enc=%u", _q->payload_enc_len);
    printf(", bit=%u", _q->payload_bit_len);
    printf(", mod=%u", _q->payload_mod_len);
    printf(">\n");
    return LIQUID_OK;
}

//
int QPACKETMODEM(_configure)(QPACKETMODEM() _q,
                           unsigned int _payload_len,
                           crc_scheme   _check,
                           fec_scheme   _fec0,
                           fec_scheme   _fec1,
                           int          _ms)
{
    // set new decoded message length
    _q->payload_dec_len = _payload_len;

    // recreate modem object and get new bits per symbol
    _q->mod_payload = MODEM(_recreate)(_q->mod_payload, _ms);
    _q->bits_per_symbol = MODEM(_get_bps)(_q->mod_payload);

    // recreate packetizer object and compute new encoded payload length
    _q->p = packetizer_recreate(_q->p, _q->payload_dec_len, _check, _fec0, _fec1);
    _q->payload_enc_len = packetizer_get_enc_msg_len(_q->p);

    // number of bits in encoded payload
    _q->payload_bit_len = 8*_q->payload_enc_len;

    // number of symbols in encoded payload
    div_t d = div(_q->payload_bit_len, _q->bits_per_symbol);
    _q->payload_mod_len = d.quot + (d.rem ? 1 : 0);

    // encoded payload array (leave room for soft-decision decoding)
    _q->payload_enc = (unsigned char*) realloc(_q->payload_enc,
            _q->bits_per_symbol*_q->payload_mod_len*sizeof(unsigned char));

    // reallocate memory for modem symbols
    _q->payload_mod = (unsigned char*) realloc(_q->payload_mod,
                                               _q->payload_mod_len*sizeof(unsigned char));

    _q->n = 0;
    _q->evm = 0.0f;
    return LIQUID_OK;
}

// get length of encoded frame in symbols
unsigned int QPACKETMODEM(_get_frame_len)(QPACKETMODEM() _q)
{
    return _q->payload_mod_len;
}

// get unencoded/decoded payload length (bytes)
unsigned int QPACKETMODEM(_get_payload_len)(QPACKETMODEM() _q)
{
    // number of decoded payload bytes
    return _q->payload_dec_len;
}

unsigned int QPACKETMODEM(_get_crc)(QPACKETMODEM() _q)
{
    return packetizer_get_crc(_q->p);
}

unsigned int QPACKETMODEM(_get_fec0)(QPACKETMODEM() _q)
{
    return packetizer_get_fec0(_q->p);
}

unsigned int QPACKETMODEM(_get_fec1)(QPACKETMODEM() _q)
{
    return packetizer_get_fec1(_q->p);
}

unsigned int QPACKETMODEM(_get_modscheme)(QPACKETMODEM() _q)
{
    return MODEM(_get_scheme)(_q->mod_payload);
}

float QPACKETMODEM(_get_demodulator_phase_error)(QPACKETMODEM() _q)
{
    return MODEM(_get_demodulator_phase_error)(_q->mod_payload);
}

float QPACKETMODEM(_get_demodulator_evm)(QPACKETMODEM() _q)
{
    return _q->evm;
}

// encode packet into un-modulated frame symbol indices
//  _q          :   qpacketmodem object
//  _payload    :   unencoded payload bytes
//  _syms       :   encoded but un-modulated payload symbol indices
int QPACKETMODEM(_encode_syms)(qpacketmodem          _q,
                             const unsigned char * _payload,
                             unsigned char *       _syms)
{
    // encode payload
    packetizer_encode(_q->p, _payload, _q->payload_enc);

    // clear internal payload
    memset(_q->payload_mod, 0x00, _q->payload_mod_len);

    // repack 8-bit payload bytes into 'bps'-bit payload symbols
    unsigned int bps = _q->bits_per_symbol;
    unsigned int num_written;
    liquid_repack_bytes(_q->payload_enc,  8,  _q->payload_enc_len,
                        _syms,           bps, _q->payload_mod_len,
                        &num_written);
    if (num_written != _q->payload_mod_len)
        return liquid_error(LIQUID_EINT,"qpacketmodem_encode_syms(), internal unexpected number of symbols");
    return LIQUID_OK;
}

// decode packet from demodulated frame symbol indices (hard-decision decoding)
//  _q          :   qpacketmodem object
//  _syms       :   received hard-decision symbol indices
//  _payload    :   recovered decoded payload bytes
int QPACKETMODEM(_decode_syms)(qpacketmodem    _q,
                             unsigned char * _syms,
                             unsigned char * _payload)
{
    // pack bytes into payload array
    unsigned int bps = _q->bits_per_symbol;
    unsigned int num_written;
    liquid_repack_bytes(_syms,           bps, _q->payload_mod_len,
                        _q->payload_enc,   8, _q->payload_mod_len, // NOTE: payload_enc allocation is actually payload_mod_len bytes
                        &num_written);
    //assert(num_written == _q->payload_enc_len); // NOTE: this will fail for bps in {3,5,6,7}

    // decode payload
    return packetizer_decode(_q->p, _q->payload_enc, _payload);
}

// decode packet from demodulated frame bits (soft-decision decoding)
//  _q          :   qpacketmodem object
//  _bits       :   received soft-decision bits
//  _payload    :   recovered decoded payload bytes
int QPACKETMODEM(_decode_bits)(qpacketmodem    _q,
                             unsigned char * _bits,
                             unsigned char * _payload)
{
    // decode payload (soft-decision)
    return packetizer_decode_soft(_q->p, _bits, _payload);
}

// encode and modulate packet into modulated frame samples
//  _q          :   qpacketmodem object
//  _payload    :   unencoded payload bytes
//  _frame      :   encoded/modulated payload symbols
int QPACKETMODEM(_encode)(qpacketmodem        _q,
                        const unsigned char * _payload,
                        TO                  * _frame)
{
    // encode payload symbols into internal buffer
    QPACKETMODEM(_encode_syms)(_q, _payload, _q->payload_mod);

    // modulate symbols
    unsigned int i;
    for (i=0; i<_q->payload_mod_len; i++)
        MODEM(_modulate)(_q->mod_payload, _q->payload_mod[i], &_frame[i]);
    return LIQUID_OK;
}

// decode packet from modulated frame samples, returning flag if CRC passed
//  _q          :   qpacketmodem object
//  _frame      :   encoded/modulated payload symbols
//  _payload    :   recovered decoded payload bytes
int QPACKETMODEM(_decode)(qpacketmodem    _q,
                          TO *            _frame,
                          unsigned char * _payload)
{
    unsigned int i;

    // demodulate and pack bytes into decoder input buffer
    unsigned int sym;
    //memset(_q->payload_enc, 0x00, _q->payload_enc_len*sizeof(unsigned char));
    _q->evm = 0.0f;
    for (i=0; i<_q->payload_mod_len; i++) {
        // demodulate symbol
        MODEM(_demodulate)(_q->mod_payload, _frame[i], &sym);

        // accumulate error vector magnitude estimate
        float e = MODEM(_get_demodulator_evm)(_q->mod_payload);
        _q->evm += e*e;

        // pack decoded symbol into array
        liquid_pack_array(_q->payload_enc,
                          _q->payload_enc_len,
                          i * _q->bits_per_symbol,
                          _q->bits_per_symbol,
                          sym);
    }

    // update internal error vector magnitude estimate
    _q->evm = 10*log10f(_q->evm / (float)(_q->payload_mod_len));

    // decode payload, returning flag if decoded payload is valid
    return packetizer_decode(_q->p, _q->payload_enc, _payload);
}

// decode packet from modulated frame samples, returning flag if CRC passed
//  _q          :   qpacketmodem object
//  _frame      :   encoded/modulated payload symbols
//  _payload    :   recovered decoded payload bytes
int QPACKETMODEM(_decode_soft)(qpacketmodem    _q,
                               TO *            _frame,
                               unsigned char * _payload)
{
    unsigned int i;

    // demodulate and pack bytes into decoder input buffer
    unsigned int sym;
    //memset(_q->payload_enc, 0x00, _q->payload_enc_len*sizeof(unsigned char));
    unsigned int n = 0;
    _q->evm = 0.0f;
    for (i=0; i<_q->payload_mod_len; i++) {
        // demodulate symbol
        MODEM(_demodulate_soft)(_q->mod_payload, _frame[i], &sym, _q->payload_enc+n);
        n += _q->bits_per_symbol;

        // accumulate error vector magnitude estimate
        float e = MODEM(_get_demodulator_evm)(_q->mod_payload);
        _q->evm += e*e;
    }
    //printf("received %u bits (expected %u)\n", n, _q->payload_mod_len * _q->bits_per_symbol);
    assert( n == _q->payload_mod_len * _q->bits_per_symbol);

    // update internal error vector magnitude estimate
    _q->evm = 10*log10f(_q->evm / (float)(_q->payload_mod_len));

    // decode payload, returning flag if decoded payload is valid
    return packetizer_decode_soft(_q->p, _q->payload_enc, _payload);
}

// decode symbol from modulated frame samples, returning flag if all symbols received
int QPACKETMODEM(_decode_soft_sym)(qpacketmodem _q,
                                   TO           _symbol)
{
    unsigned int sym;
    MODEM(_demodulate_soft)(_q->mod_payload, _symbol, &sym, _q->payload_enc + _q->n);
    _q->n += _q->bits_per_symbol;
    return _q->n == _q->payload_mod_len * _q->bits_per_symbol;
}

int QPACKETMODEM(_decode_soft_payload)(qpacketmodem    _q,
                                       unsigned char * _payload)
{
    if ( _q->n != _q->payload_mod_len * _q->bits_per_symbol) {
        liquid_error(LIQUID_ENOINIT,"qpacketmodem_decode_soft_payload(), insufficient number of symbols received");
        return 0;
    }
    _q->n = 0;
    return packetizer_decode_soft(_q->p, _q->payload_enc, _payload);
}

