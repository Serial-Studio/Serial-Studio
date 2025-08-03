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

// frequency-shift keying (FSK) frame generator

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <complex.h>

#include "liquid.internal.h"

#define DEBUG_FSKFRAMEGEN    0

// fskframegen
int fskframegen_encode_header    (fskframegen _q, unsigned char * _header);
int fskframegen_generate_symbol  (fskframegen _q);
int fskframegen_generate_zeros   (fskframegen _q);
int fskframegen_generate_preamble(fskframegen _q);
int fskframegen_generate_header  (fskframegen _q);
int fskframegen_generate_payload (fskframegen _q);

// fskframe object structure
struct fskframegen_s {
    unsigned int    m;                  // modulator bits/symbol
    unsigned int    k;                  // modulator samples/symbol
    float           bandwidth;          // modulator bandwidth
    unsigned int    M;                  // modulator constellation size, M=2^m
    fskmod          mod_header;         // modulator object for the header (BFSK)
    fskmod          mod;                // modulator object (M-FSK)
    float complex * buf;                // modulator transmit buffer [size: k x 1]

    // preamble
    unsigned int    preamble_sym_len;   // preamble symbols length
    unsigned char * preamble_sym;       // preamble symbol sequence

    // header
#if 0
    unsigned int    header_dec_len;     // header decoded message length
    crc_scheme      header_crc;         // header validity check
    fec_scheme      header_fec0;        // header inner code
    fec_scheme      header_fec1;        // header outer code
    packetizer      header_encoder;     // header encoder
    unsigned int    header_enc_len;     // header encoded message length
    unsigned char * header_dec;         // header uncoded [size: header_dec_len x 1]
    unsigned char * header_enc;         // header encoded [size: header_enc_len x 1]
    unsigned int    header_sym_len;     // header symbols length
#else
    unsigned int    header_dec_len;     //
    unsigned int    header_sym_len;     //
    unsigned char * header_dec;         // header uncoded [size: header_dec_len x 1]
    unsigned char * header_sym;         // header: unmodulated symbols
    qpacketmodem    header_encoder;     //
#endif

    // payload
#if 0
    unsigned int    payload_dec_len;    // payload decoded message length
    crc_scheme      payload_crc;        // payload validity check
    fec_scheme      payload_fec0;       // payload inner code
    fec_scheme      payload_fec1;       // payload outer code
    packetizer      payload_encoder;    // payload encoder
    unsigned int    payload_enc_len;    // payload encoded message length
    unsigned char * payload_enc;        // paylaod encoded [size: payload_enc_len x 1]
    unsigned int    payload_sym_len;    // payload symbols length
#else
    unsigned int    payload_dec_len;    //
    crc_scheme      payload_crc;        // payload validity check
    fec_scheme      payload_fec0;       // payload inner code
    fec_scheme      payload_fec1;       // payload outer code
    unsigned int    payload_sym_len;    //
    unsigned char * payload_sym;        //
    qpacketmodem    payload_encoder;    //
#endif

    // framing state
    enum {
                    STATE_OFF,          // unassembled
                    STATE_PREAMBLE,     // preamble
                    STATE_HEADER,       // header
                    STATE_PAYLOAD,      // payload (frame)
    }               state;
    int             frame_assembled;    // frame assembled flag
    int             frame_complete;     // frame completed flag
    unsigned int    symbol_counter;     // output symbol counter
    unsigned int    sample_counter;     // output sample counter
};

// create fskframegen object
fskframegen fskframegen_create()
{
    fskframegen q = (fskframegen) malloc(sizeof(struct fskframegen_s));

    // set static values
    q->m         = 4;           // modulation bits/symbol
    q->M         = 1 << q->m;   // modulation constellation size
    q->k         = 2 << q->m;   // samples per symbol
    q->bandwidth = 0.25f;       // occupied one-sided bandwidth (normalized to sample rate)

    // create modulators
    q->mod_header = fskmod_create(   1, q->k, q->bandwidth);
    q->mod        = fskmod_create(q->m, q->k, q->bandwidth);
    q->buf        = (float complex*) malloc( q->k * sizeof(float complex) );

    // preamble symbols (over-sampled by 2)
    msequence preamble_ms = msequence_create(6, 0x6d, 1);
    q->preamble_sym_len = 64;
    q->preamble_sym = (unsigned char*)malloc(2*q->preamble_sym_len*sizeof(unsigned char));
    unsigned int i;
    for (i=0; i<q->preamble_sym_len; i++) {
        q->preamble_sym[2*i+0] = msequence_advance(preamble_ms) ? 1 : 0;
        q->preamble_sym[2*i+1] = q->preamble_sym[2*i+0];
    }
    msequence_destroy(preamble_ms);

    // header objects/arrays
#if 0
    q->header_dec_len   = 10;
    q->header_crc       = LIQUID_CRC_32;
    q->header_fec0      = LIQUID_FEC_NONE;
    q->header_fec1      = LIQUID_FEC_GOLAY2412;
    q->header_encoder   = packetizer_create(q->header_dec_len,
                                            q->header_crc,
                                            q->header_fec0,
                                            q->header_fec1);
    q->header_enc_len   = packetizer_get_dec_msg_len(q->header_encoder);
    q->header_dec       = (unsigned char*)malloc(q->header_dec_len*sizeof(unsigned char));
    q->header_enc       = (unsigned char*)malloc(q->header_enc_len*sizeof(unsigned char));
    q->header_sym_len   = q->header_enc_len * 8 / q->m;
#else
    q->header_dec_len   = 10;
    q->header_dec       = (unsigned char*)malloc(q->header_dec_len*sizeof(unsigned char));
    q->header_encoder   = qpacketmodem_create();
    qpacketmodem_configure(q->header_encoder,
                           q->header_dec_len,
                           LIQUID_CRC_32,
                           LIQUID_FEC_NONE,
                           LIQUID_FEC_GOLAY2412,
                           LIQUID_MODEM_BPSK);
    q->header_sym_len   = qpacketmodem_get_frame_len(q->header_encoder);
    q->header_sym       = (unsigned char*)malloc(q->header_sym_len*sizeof(unsigned char));
#endif

    // payload objects/arrays
#if 0
    q->payload_dec_len  = 10;
    q->payload_crc      = LIQUID_CRC_32;
    q->payload_fec0     = LIQUID_FEC_NONE;
    q->payload_fec1     = LIQUID_FEC_GOLAY2412;
    q->payload_encoder  = packetizer_create(q->payload_dec_len,
                                            q->payload_crc,
                                            q->payload_fec0,
                                            q->payload_fec1);
    q->payload_enc_len  = packetizer_get_dec_msg_len(q->payload_encoder);
    q->payload_enc      = (unsigned char*)malloc(q->payload_enc_len*sizeof(unsigned char));
    q->payload_sym_len  = 0;    // TODO: set this appropriately
#else
    q->payload_dec_len  = 200;
    q->payload_crc      = LIQUID_CRC_32;
    q->payload_fec0     = LIQUID_FEC_NONE;
    q->payload_fec1     = LIQUID_FEC_HAMMING128;
    q->payload_encoder  = qpacketmodem_create();
    qpacketmodem_configure(q->payload_encoder,
                           q->payload_dec_len,
                           q->payload_crc,
                           q->payload_fec0,
                           q->payload_fec1,
                           LIQUID_MODEM_QAM16);  // TODO: set bits/sym appropriately
    q->payload_sym_len  = qpacketmodem_get_frame_len(q->payload_encoder);
    q->payload_sym      = (unsigned char*)malloc(q->payload_sym_len*sizeof(unsigned char));
#endif

    // reset framing object
    fskframegen_reset(q);

    // return object
    return q;
}

// destroy fskframegen object
int fskframegen_destroy(fskframegen _q)
{
    // destroy modulators
    fskmod_destroy(_q->mod_header);
    fskmod_destroy(_q->mod);
    free(_q->buf);

    // free preamble symbols array
    free(_q->preamble_sym);

    // destroy/free header objects/arrays
#if 0
    free(_q->header_dec);
    free(_q->header_enc);
    packetizer_destroy(_q->header_encoder);
#else
    free(_q->header_dec);
    free(_q->header_sym);
    qpacketmodem_destroy(_q->header_encoder);
#endif

    // destroy/free payload objects/arrays
#if 0
    free(_q->payload_enc);
    packetizer_destroy(_q->payload_encoder);
#else
    free(_q->payload_sym);
    qpacketmodem_destroy(_q->payload_encoder);
#endif

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// reset frame generator object
int fskframegen_reset(fskframegen _q)
{
    // reset modulator
    fskmod_reset(_q->mod_header);
    fskmod_reset(_q->mod);

    // reset states
    _q->state = STATE_OFF;
    _q->frame_assembled = 0;
    _q->frame_complete  = 0;
    _q->symbol_counter  = 0;
    _q->sample_counter  = _q->k;    // indicate we are ready for a new symbol
    return LIQUID_OK;
}

// print fskframegen object internals
int fskframegen_print(fskframegen _q)
{
#if 0
    printf("fskframegen:\n");
    printf("  physical properties\n");
    printf("    bits/symbol     :   %u\n", _q->m);
    printf("    samples/symbol  :   %u\n", _q->k);
    printf("    bandwidth       :   %-8.3f\n", _q->bandwidth);
    printf("  framing properties\n");
    printf("    preamble        :   %-4u symbols\n", _q->preamble_sym_len);
    printf("    header          :   %-4u symbols, %-4u bytes\n", _q->header_sym_len, _q->header_dec_len);
    printf("    payload         :   %-4u symbols, %-4u bytes\n", _q->payload_sym_len, _q->payload_dec_len);
    printf("  packet properties\n");
    printf("    crc             :   %s\n", crc_scheme_str[_q->payload_crc ][1]);
    printf("    fec (inner)     :   %s\n", fec_scheme_str[_q->payload_fec0][1]);
    printf("    fec (outer)     :   %s\n", fec_scheme_str[_q->payload_fec1][1]);
    printf("  total samples     :   %-4u samples\n", fskframegen_getframelen(_q));
#else
    printf("<liquid.fskframegen>\n");
#endif
    return LIQUID_OK;
}

// assemble frame
//  _q              :   frame generator object
//  _header         :   raw header
//  _payload        :   raw payload [size: _payload_len x 1]
//  _payload_len    :   raw payload length (bytes)
//  _check          :   data validity check
//  _fec0           :   inner forward error correction
//  _fec1           :   outer forward error correction
int fskframegen_assemble(fskframegen     _q,
                         unsigned char * _header,
                         unsigned char * _payload,
                         unsigned int    _payload_len,
                         crc_scheme      _check,
                         fec_scheme      _fec0,
                         fec_scheme      _fec1)
{
#if 1
    liquid_error(LIQUID_ENOIMP,"fskframegen_assemble(), base functionality not implemented; ignoring input parameters for now");
#else
    // set properties
    _q->payload_dec_len = _payload_len;
    _q->payload_crc     = _check;
    _q->payload_fec0    = _fec0;
    _q->payload_fec1    = _fec1;

    // re-create payload packetizer
    //int rc =
    qpacketmodem_configure(_q->payload_encoder,
                           _q->payload_dec_len,
                           _q->payload_crc,
                           _q->payload_fec0,
                           _q->payload_fec1,
                           LIQUID_MODEM_QPSK);
#endif
    
    // get packet length
    _q->payload_sym_len = qpacketmodem_get_frame_len(_q->payload_encoder);

    // re-allocate memory
    _q->payload_sym = (unsigned char*) realloc(_q->payload_sym, _q->payload_sym_len*sizeof(unsigned char));
    
    // set assembled flag
    _q->frame_assembled = 1;

    // encode header
    fskframegen_encode_header(_q, _header);

    // encode payload symbols
    qpacketmodem_encode_syms(_q->payload_encoder, _payload, _q->payload_sym);
#if 0
    printf("tx payload symbols (%u)\n", _q->payload_sym_len);
    unsigned int i;
    for (i=0; i<_q->payload_sym_len; i++)
        printf("%1x%s", _q->payload_sym[i], ((i+1)%64)==0 ? "\n" : "");
    printf("\n");
#endif

    // set state appropriately
    _q->state = STATE_PREAMBLE;
    return LIQUID_OK;
}

// get frame length (number of samples)
unsigned int fskframegen_getframelen(fskframegen _q)
{
    if (!_q->frame_assembled) {
        liquid_error(LIQUID_EICONFIG,"fskframegen_getframelen(), frame not assembled!");
        return 0;
    }

    unsigned int num_frame_symbols =
        _q->preamble_sym_len +  // number of preamble p/n symbols
        _q->header_sym_len   +  // number of preamble p/n symbols
        _q->payload_sym_len;    // number of payload symbols

    return num_frame_symbols*_q->k; // k samples/symbol
}

// write sample to output buffer
int fskframegen_write_samples(fskframegen     _q,
                              float complex * _buf,
                              unsigned int    _buf_len)
{
    unsigned int i;
    for (i=0; i<_buf_len; i++) {
        // buffer emptied; generate new symbol
        if (_q->sample_counter == _q->k) {
            fskframegen_generate_symbol(_q);
            _q->sample_counter = 0;
        }

        _buf[i] = _q->buf[_q->sample_counter++];
    }

    return _q->frame_assembled ? 0 : 1;
}


// 
// internal methods
//

int fskframegen_encode_header(fskframegen     _q,
                               unsigned char * _header)
{
    // first 8 bytes user data
    memmove(_q->header_dec, _header, 8);
    unsigned int n = 8;

#if 0
    // first byte is for expansion/version validation
    _q->header_dec[n+0] = 0;    // version

    // add payload length
    _q->header_dec[n+1] = (_q->payload_dec_len >> 8) & 0xff;
    _q->header_dec[n+2] = (_q->payload_dec_len     ) & 0xff;

    // add CRC, forward error-correction schemes
    //  CRC     : most-significant 3 bits of [n+4]
    //  fec0    : least-significant 5 bits of [n+4]
    //  fec1    : least-significant 5 bits of [n+5]
    _q->header_dec[n+3]  = (_q->payload_crc & 0x07) << 5;
    _q->header_dec[n+3] |= (_q->payload_fec0      ) & 0x1f;
    _q->header_dec[n+4]  = (_q->payload_fec1      ) & 0x1f;
#else
    while (n < _q->header_dec_len)
        _q->header_dec[n++] = 0xff;
#endif

    // run packet encoder, encoding into symbols
    qpacketmodem_encode_syms(_q->header_encoder, _q->header_dec, _q->header_sym);

#if 0
    printf("tx header symbols (%u):\n", _q->header_sym_len);
    unsigned int i;
    for (i=0; i<_q->header_sym_len; i++)
        printf("%1x", _q->header_sym[i]);
    printf("\n");


    printf("tx header decoded (%u):\n", _q->header_dec_len);
    for (i=0; i<_q->header_dec_len; i++)
        printf(" %2x", _q->header_dec[i]);
    printf("\n");
#endif
    // TODO: scramble header
    return LIQUID_OK;
}

// write single symbol to internal buffer
int fskframegen_generate_symbol(fskframegen _q)
{
    switch (_q->state) {
    case STATE_OFF:      return fskframegen_generate_zeros(_q);
    case STATE_PREAMBLE: return fskframegen_generate_preamble(_q);
    case STATE_HEADER:   return fskframegen_generate_header(_q);
    case STATE_PAYLOAD:  return fskframegen_generate_payload(_q);
    default:;
    }
    return liquid_error(LIQUID_EINT,"fskframegen_writesymbol(), unknown/unsupported internal state");
}

int fskframegen_generate_zeros(fskframegen _q)
{
    unsigned int i;
    for (i=0; i<_q->k; i++)
        _q->buf[i] = 0.0f;
    return LIQUID_OK;
}

int fskframegen_generate_preamble(fskframegen _q)
{
    unsigned char s = _q->preamble_sym[_q->symbol_counter];
    fskmod_modulate(_q->mod_header, s, _q->buf);

    // TODO: apply ramping for first symbol?

    _q->symbol_counter++;

    // NOTE: preamble is over-sampled by 2
    if (_q->symbol_counter == 2*_q->preamble_sym_len) {
        _q->symbol_counter = 0;
        _q->state = STATE_HEADER;
    }
    return LIQUID_OK;
}

int fskframegen_generate_header(fskframegen _q)
{
    unsigned int s = _q->header_sym[_q->symbol_counter];

    fskmod_modulate(_q->mod_header, s, _q->buf);

    _q->symbol_counter++;
    
    if (_q->symbol_counter == _q->header_sym_len) {
        _q->symbol_counter = 0;
        _q->state = STATE_PAYLOAD;
    }
    return LIQUID_OK;
}

int fskframegen_generate_payload(fskframegen _q)
{
    unsigned int s = _q->payload_sym[_q->symbol_counter];

    fskmod_modulate(_q->mod, s, _q->buf);

    _q->symbol_counter++;
    
    if (_q->symbol_counter == _q->payload_sym_len) {
        _q->symbol_counter = 0;
        _q->frame_assembled = 0;
        _q->state = STATE_OFF;
    }
    return LIQUID_OK;
}

