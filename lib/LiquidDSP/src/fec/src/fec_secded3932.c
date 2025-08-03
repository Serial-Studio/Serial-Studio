/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
// SEC-DED (39,32) forward error-correction block code
//
// References:
//  [Lin:2004] Lin, Shu and Costello, Daniel L. Jr., "Error Control
//      Coding," Prentice Hall, New Jersey, 2nd edition, 2004.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FEC_SECDED3932 0

// P matrix [7 x 32 bits], [7 x 4 bytes]
//  1000 1010 1000 0010 0000 1111 0001 1011
//  0001 0000 0001 1111 0111 0001 0110 0001
//  0001 0110 1111 0000 1001 0010 1010 0110
//  1111 1111 0000 0001 1010 0100 0100 0100
//  0110 1100 1111 1111 0000 1000 0000 1000
//  0010 0001 0010 0100 1111 1111 1001 0000
//  1100 0001 0100 1000 0100 0000 1111 1111
unsigned char secded3932_P[28] = {
    0x8a, 0x82, 0x0f, 0x1b,
    0x10, 0x1f, 0x71, 0x61,
    0x16, 0xf0, 0x92, 0xa6,
    0xff, 0x01, 0xa4, 0x44,
    0x6c, 0xff, 0x08, 0x08,
    0x21, 0x24, 0xff, 0x90,
    0xc1, 0x48, 0x40, 0xff};


// syndrome vectors for errors of weight 1
unsigned char secded3932_syndrome_w1[39] = {
    0x61, 0x51, 0x19, 0x45, 
    0x43, 0x31, 0x29, 0x13, 
    0x62, 0x52, 0x4a, 0x46, 
    0x32, 0x2a, 0x23, 0x1a, 
    0x2c, 0x64, 0x26, 0x25, 
    0x34, 0x16, 0x15, 0x54, 
    0x0b, 0x58, 0x1c, 0x4c, 
    0x38, 0x0e, 0x0d, 0x49, 
    0x01, 0x02, 0x04, 0x08, 
    0x10, 0x20, 0x40};

// compute parity on 32-bit input
unsigned char fec_secded3932_compute_parity(unsigned char * _m)
{
    // compute encoded/transmitted message: v = m*G
    unsigned char parity = 0x00;

    // TODO : unwrap this loop
    unsigned int i;
    for (i=0; i<7; i++) {
        parity <<= 1;

        unsigned int p = liquid_c_ones[ secded3932_P[4*i+0] & _m[0] ] +
                         liquid_c_ones[ secded3932_P[4*i+1] & _m[1] ] +
                         liquid_c_ones[ secded3932_P[4*i+2] & _m[2] ] +
                         liquid_c_ones[ secded3932_P[4*i+3] & _m[3] ];

        parity |= p & 0x01;
    }

    return parity;
}

// compute syndrome on 39-bit input
unsigned char fec_secded3932_compute_syndrome(unsigned char * _v)
{
    // TODO : unwrap this loop
    unsigned int i;
    unsigned char syndrome = 0x00;
    for (i=0; i<7; i++) {
        syndrome <<= 1;

        unsigned int p =
            ( (_v[0] & (1<<(7-i-1))) ? 1 : 0 )+
            liquid_c_ones[ secded3932_P[4*i+0] & _v[1] ] +
            liquid_c_ones[ secded3932_P[4*i+1] & _v[2] ] +
            liquid_c_ones[ secded3932_P[4*i+2] & _v[3] ] +
            liquid_c_ones[ secded3932_P[4*i+3] & _v[4] ];

        syndrome |= p & 0x01;
    }

    return syndrome;
}

// encode symbol
//  _sym_dec    :   decoded symbol [size: 4 x 1]
//  _sym_enc    :   encoded symbol [size: 5 x 1], _sym_enc[0] has only 7 bits
int fec_secded3932_encode_symbol(unsigned char * _sym_dec,
                                 unsigned char * _sym_enc)
{
    // first six bits is parity block
    _sym_enc[0] = fec_secded3932_compute_parity(_sym_dec);

    // concatenate original message
    _sym_enc[1] = _sym_dec[0];
    _sym_enc[2] = _sym_dec[1];
    _sym_enc[3] = _sym_dec[2];
    _sym_enc[4] = _sym_dec[3];
    return LIQUID_OK;
}

// decode symbol, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol [size: 5 x 1], _sym_enc[0] has only 7 bits
//  _sym_dec    :   decoded symbol [size: 4 x 1]
int fec_secded3932_decode_symbol(unsigned char * _sym_enc,
                                 unsigned char * _sym_dec)
{
#if 0
    // validate input
    if (_sym_enc[0] >= (1<<7)) {
        fprintf(stderr,"warning, fec_secded3932_decode_symbol(), input symbol too large\n");
    }
#endif

    // estimate error vector
    unsigned char e_hat[5] = {0,0,0,0,0};    // estimated error vector
    int syndrome_flag = fec_secded3932_estimate_ehat(_sym_enc, e_hat);

    // compute estimated transmit vector (last 64 bits of encoded message)
    // NOTE: indices take into account first element in _sym_enc and e_hat
    //       arrays holds the parity bits
    _sym_dec[0] = _sym_enc[1] ^ e_hat[1];
    _sym_dec[1] = _sym_enc[2] ^ e_hat[2];
    _sym_dec[2] = _sym_enc[3] ^ e_hat[3];
    _sym_dec[3] = _sym_enc[4] ^ e_hat[4];

#if DEBUG_FEC_SECDED3932
    if (syndrome_flag == 1) {
        printf("secded3932_decode_symbol(): single error detected!\n");
    } else if (syndrome_flag == 2) {
        printf("secded3932_decode_symbol(): no match found (multiple errors detected)\n");
    }
#endif

    // return syndrome flag
    return syndrome_flag;
}

// estimate error vector, returning 0/1/2 for zero/one/multiple errors
// detected, respectively
//  _sym_enc    :   encoded symbol [size: 5 x 1], _sym_enc[0] has only 6 bits
//  _e_hat      :   estimated error vector [size: 5 x 1]
int fec_secded3932_estimate_ehat(unsigned char * _sym_enc,
                                 unsigned char * _e_hat)
{
    // clear output array
    _e_hat[0] = 0x00;
    _e_hat[1] = 0x00;
    _e_hat[2] = 0x00;
    _e_hat[3] = 0x00;
    _e_hat[4] = 0x00;

    // compute syndrome vector, s = r*H^T = ( H*r^T )^T
    unsigned char s = fec_secded3932_compute_syndrome(_sym_enc);

    // compute weight of s
    unsigned int ws = liquid_c_ones[s];
    
    if (ws == 0) {
        // no errors detected
        return 0;
    } else {
        // estimate error location; search for syndrome with error
        // vector of weight one

        unsigned int n;
        // estimate error location
        for (n=0; n<39; n++) {
            if (s == secded3932_syndrome_w1[n]) {
                // single error detected at location 'n'
                div_t d = div(n,8);
                _e_hat[5-d.quot-1] = 1 << d.rem;

                return 1;
            }
        }

    }

    // no syndrome match; multiple errors detected
    return 2;
}

// create SEC-DED (39,32) codec object
fec fec_secded3932_create(void * _opts)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    // set scheme
    q->scheme = LIQUID_FEC_SECDED3932;
    q->rate = fec_get_rate(q->scheme);

    // set internal function pointers
    q->encode_func      = &fec_secded3932_encode;
    q->decode_func      = &fec_secded3932_decode;
    q->decode_soft_func = NULL;

    return q;
}

// destroy SEC-DEC (39,32) object
int fec_secded3932_destroy(fec _q)
{
    free(_q);
    return LIQUID_OK;
}

// encode block of data using SEC-DEC (39,32) encoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
int fec_secded3932_encode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc)
{
    unsigned int i=0;       // decoded byte counter
    unsigned int j=0;       // encoded byte counter

    // determine remainder of input length / 4
    unsigned int r = _dec_msg_len % 4;

    // for now simply encode as 4/5-rate codec (eat
    // last parity bit)
    // TODO : make more efficient

    for (i=0; i<_dec_msg_len-r; i+=4) {
        // compute parity (7 bits) on two input bytes (32 bits)
        _msg_enc[j+0] = fec_secded3932_compute_parity(&_msg_dec[i]);

        // copy remaining two input bytes (32 bits)
        _msg_enc[j+1] = _msg_dec[i+0];
        _msg_enc[j+2] = _msg_dec[i+1];
        _msg_enc[j+3] = _msg_dec[i+2];
        _msg_enc[j+4] = _msg_dec[i+3];

        // increment output counter
        j += 5;
    }

    // if input length isn't divisible by 4, encode last few bytes
    if (r) {
        // one 32-bit symbol (decoded)
        unsigned char m[4] = {0,0,0,0};
        unsigned int n;
        for (n=0; n<r; n++)
            m[n] = _msg_dec[i+n];

        // one 39-bit symbol (encoded)
        unsigned char v[5];

        // encode
        fec_secded3932_encode_symbol(m, v);

        // there is no need to actually send all five bytes;
        // the last few bytes are zero and can be artificially
        // inserted at the decoder
        _msg_enc[j+0] = v[0];
        for (n=0; n<r; n++)
            _msg_enc[j+n+1] = v[n+1];

        i += r;
        j += r+1;
    }

    assert( j == fec_get_enc_msg_length(LIQUID_FEC_SECDED3932,_dec_msg_len) );
    assert( i == _dec_msg_len);
    return LIQUID_OK;
}

// decode block of data using SEC-DEC (39,32) decoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//
//unsigned int
int fec_secded3932_decode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec)
{
    unsigned int i=0;       // decoded byte counter
    unsigned int j=0;       // encoded byte counter
    
    // determine remainder of input length / 4
    unsigned int r = _dec_msg_len % 4;

    for (i=0; i<_dec_msg_len-r; i+=4) {
        // decode straight to output
        fec_secded3932_decode_symbol(&_msg_enc[j], &_msg_dec[i]);

        j += 5;
    }

    // if input length isn't divisible by 4, decode last several bytes
    if (r) {
        // one 39-bit symbol
        unsigned char v[5] = {_msg_enc[j+0], 0, 0, 0, 0};
        unsigned int n;
        for (n=0; n<r; n++)
            v[n+1] = _msg_enc[j+n+1];

        // one 32-bit symbol (decoded)
        unsigned char m_hat[4];

        // decode symbol
        fec_secded3932_decode_symbol(v, m_hat);

        // copy non-zero bytes to output (ignore zeros artificially
        // inserted at receiver)
        for (n=0; n<r; n++)
            _msg_dec[i+n] = m_hat[n];

        i += r;
        j += r+1;
    }

    assert( j == fec_get_enc_msg_length(LIQUID_FEC_SECDED3932,_dec_msg_len) );
    assert( i == _dec_msg_len);
    return LIQUID_OK;
}

