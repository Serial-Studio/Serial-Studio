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
// bpacketgen
//
// binary packet generator/encoder
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

// bpacketgen object structure
struct bpacketgen_s {
    // options
    unsigned int g;                 // p/n sequence genpoly
    unsigned int pnsequence_len;    // p/n sequence length (bytes)
    unsigned int dec_msg_len;       // payload length
    crc_scheme crc;                 // payload check
    fec_scheme fec0;                // payload fec (inner)
    fec_scheme fec1;                // payload fec (outer)
    
    // derived values
    unsigned int enc_msg_len;       // encoded message length
    unsigned int header_len;        // header length (12 bytes encoded)
    unsigned int packet_len;        // total packet length

    // arrays
    unsigned char * pnsequence;     // p/n sequence

    // bpacket header
    //  0   :   version number
    //  1   :   crc
    //  2   :   fec0
    //  3   :   fec1
    //  4:5 :   payload length
    unsigned char header_dec[6];    // uncoded bytes
    unsigned char header_enc[12];   // 12 = 6 + crc16 at hamming(12,8)

    // objects
    msequence ms;
    packetizer p_header;
    packetizer p_payload;
};


// create bpacketgen object
//  _m              :   p/n sequence length (ignored)
//  _dec_msg_len    :   decoded message length (original uncoded data)
//  _crc            :   data validity check (e.g. cyclic redundancy check)
//  _fec0           :   inner forward error-correction code scheme
//  _fec1           :   outer forward error-correction code scheme
bpacketgen bpacketgen_create(unsigned int _m,
                             unsigned int _dec_msg_len,
                             int _crc,
                             int _fec0,
                             int _fec1)
{
    // validate input

    // create bpacketgen object
    bpacketgen q = (bpacketgen) malloc(sizeof(struct bpacketgen_s));
    q->dec_msg_len  = _dec_msg_len;
    q->crc          = _crc;
    q->fec0         = _fec0;
    q->fec1         = _fec1;

    // implied values
    q->g = 0;
    q->pnsequence_len = 8;

    // derived values
    q->enc_msg_len = packetizer_compute_enc_msg_len(q->dec_msg_len,
                                                    q->crc,
                                                    q->fec0,
                                                    q->fec1);
    q->header_len = packetizer_compute_enc_msg_len(6, LIQUID_CRC_16, LIQUID_FEC_NONE, LIQUID_FEC_HAMMING128);
    bpacketgen_compute_packet_len(q);

    // arrays
    q->pnsequence = (unsigned char*) malloc((q->pnsequence_len)*sizeof(unsigned char));

    // create m-sequence generator
    // TODO : configure sequence from generator polynomial
    q->ms = msequence_create_default(6);

    // create header packet encoder
    q->p_header = packetizer_create(6, LIQUID_CRC_16, LIQUID_FEC_NONE, LIQUID_FEC_HAMMING128);
    assert(q->header_len == packetizer_get_enc_msg_len(q->p_header));

    // create payload packet encoder
    q->p_payload = packetizer_create(q->dec_msg_len,
                                     q->crc,
                                     q->fec0,
                                     q->fec1);

    // assemble semi-static framing structures
    bpacketgen_assemble_header(q);
    bpacketgen_assemble_pnsequence(q);

    return q;
}

// re-create bpacketgen object from old object
//  _q              :   old bpacketgen object
//  _m              :   p/n sequence length (ignored)
//  _dec_msg_len    :   decoded message length (original uncoded data)
//  _crc            :   data validity check (e.g. cyclic redundancy check)
//  _fec0           :   inner forward error-correction code scheme
//  _fec1           :   outer forward error-correction code scheme
bpacketgen bpacketgen_recreate(bpacketgen _q,
                               unsigned int _m,
                               unsigned int _dec_msg_len,
                               int _crc,
                               int _fec0,
                               int _fec1)
{
    // validate input

    // re-create internal packetizer object
    _q->dec_msg_len = _dec_msg_len;
    _q->crc         = _crc;
    _q->fec0        = _fec0;
    _q->fec1        = _fec1;

    // derived values
    _q->enc_msg_len = packetizer_compute_enc_msg_len(_q->dec_msg_len,
                                                     _q->crc,
                                                     _q->fec0,
                                                     _q->fec1);
    _q->header_len = packetizer_compute_enc_msg_len(6, LIQUID_CRC_16, LIQUID_FEC_NONE, LIQUID_FEC_HAMMING128);
    bpacketgen_compute_packet_len(_q);

    // arrays
    _q->g = 0;
    _q->pnsequence_len = 8;
    _q->pnsequence = (unsigned char*) realloc(_q->pnsequence, (_q->pnsequence_len)*sizeof(unsigned char));

    // re-create m-sequence generator
    // TODO : configure sequence from generator polynomial
    msequence_destroy(_q->ms);
    _q->ms = msequence_create_default(6);

    // re-create payload packet encoder
    _q->p_payload = packetizer_recreate(_q->p_payload,
                                        _q->dec_msg_len,
                                        _q->crc,
                                        _q->fec0,
                                        _q->fec1);

    // assemble semi-static framing structures
    bpacketgen_assemble_header(_q);
    bpacketgen_assemble_pnsequence(_q);

    return _q;
}

// destroy bpacketgen object, freeing all internally-allocated memory
void bpacketgen_destroy(bpacketgen _q)
{
    // free arrays
    free(_q->pnsequence);

    // destroy internal objects
    msequence_destroy(_q->ms);
    packetizer_destroy(_q->p_header);
    packetizer_destroy(_q->p_payload);

    // free main object memory
    free(_q);
}

// print bpacketgen internals
void bpacketgen_print(bpacketgen _q)
{
    printf("<liquid.bpacketgen,");
    printf(", pn_poly0x%.4x", _q->g);
    printf(", pn_len%u", _q->pnsequence_len);
    printf(", header=%u", _q->header_len);
    printf(", payload=%u", _q->dec_msg_len);
    printf(", crc=\"%s\"", crc_scheme_str[_q->crc][0]);
    printf(", fec_0=\"%s\"", fec_scheme_str[_q->fec0][0]);
    printf(", fec_1=\"%s\"", fec_scheme_str[_q->fec1][0]);
    printf(", packet=%u", _q->packet_len);
    printf(", efficiency=%g", (float)_q->dec_msg_len/(float)_q->packet_len);
    printf(">\n");
}

// return length of full packet
unsigned int bpacketgen_get_packet_len(bpacketgen _q)
{
    return _q->packet_len;
}

// encode packet
void bpacketgen_encode(bpacketgen _q,
                       unsigned char * _msg_dec,
                       unsigned char * _packet)
{
    // output byte index counter
    unsigned int n=0;

    // copy p/n sequence
    memmove(&_packet[n], _q->pnsequence, _q->pnsequence_len*sizeof(unsigned char));
    n += _q->pnsequence_len;

    // copy header
    memmove(&_packet[n], _q->header_enc, _q->header_len*sizeof(unsigned char));
    n += _q->header_len;

    // encode payload
    packetizer_encode(_q->p_payload,
                      _msg_dec,
                      &_packet[n]);
    n += _q->enc_msg_len;

    // verify length is correct
    assert(n == _q->packet_len);
}


// 
// internal methods
//

// compute packet length
void bpacketgen_compute_packet_len(bpacketgen _q)
{
    _q->packet_len = _q->pnsequence_len +
                     _q->header_len +
                     _q->enc_msg_len;
}

// generate p/n sequence
void bpacketgen_assemble_pnsequence(bpacketgen _q)
{
    // reset m-sequence generator
    msequence_reset(_q->ms);

    unsigned int i;
    unsigned int j;
    for (i=0; i<_q->pnsequence_len; i++) {
        unsigned char byte = 0;
        for (j=0; j<8; j++) {
            byte <<= 1;
            byte |= msequence_advance(_q->ms);
        }
        _q->pnsequence[i] = byte;
    }
}

// assemble packet header
void bpacketgen_assemble_header(bpacketgen _q)
{
    _q->header_dec[0] = BPACKET_VERSION;
    _q->header_dec[1] = (unsigned char) _q->crc;
    _q->header_dec[2] = (unsigned char) _q->fec0;
    _q->header_dec[3] = (unsigned char) _q->fec1;
    _q->header_dec[4] = (unsigned char) (_q->dec_msg_len >> 8) & 0xff;
    _q->header_dec[5] = (unsigned char) (_q->dec_msg_len     ) & 0xff;

    // encode header
    packetizer_encode(_q->p_header,
                      _q->header_dec,
                      _q->header_enc);
}

