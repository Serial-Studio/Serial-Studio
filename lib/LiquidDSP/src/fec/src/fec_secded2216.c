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
// SEC-DED (22,16) 8/11-rate forward error-correction block code
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

#define DEBUG_FEC_SECDED2216 0

// P matrix [6 x 16 bits], [6 x 2 bytes]
//  1001 1001 0011 1100 :
//  0011 1110 1000 1010 :
//  1110 1110 0110 0000 :
//  1110 0001 1101 0001 :
//  0001 0011 1100 0111 :
//  0100 0100 0011 1111 :
unsigned char secded2216_P[12] = {
    0x99, 0x3c,
    0x3e, 0x8a,
    0xee, 0x60,
    0xe1, 0xd1,
    0x13, 0xc7,
    0x44, 0x3f};

// syndrome vectors for errors of weight 1
unsigned char secded2216_syndrome_w1[22] = {
    0x07, 0x13, 0x23, 0x31, 
    0x25, 0x29, 0x0e, 0x16, 
    0x26, 0x1a, 0x19, 0x38, 
    0x32, 0x1c, 0x0d, 0x2c, 
    0x01, 0x02, 0x04, 0x08, 
    0x10, 0x20};

// compute parity on 16-bit input
unsigned char fec_secded2216_compute_parity(unsigned char * _m)
{
    // compute encoded/transmitted message: v = m*G
    unsigned char parity = 0x00;

    // TODO : unwrap this loop
    unsigned int i;
    for (i=0; i<6; i++) {
        parity <<= 1;

        unsigned int p = liquid_c_ones[ secded2216_P[2*i+0] & _m[0] ] +
                         liquid_c_ones[ secded2216_P[2*i+1] & _m[1] ];

        parity |= p & 0x01;
    }

    return parity;
}

// compute syndrome on 22-bit input
unsigned char fec_secded2216_compute_syndrome(unsigned char * _v)
{
    // TODO : unwrap this loop
    unsigned int i;
    unsigned char syndrome = 0x00;
    for (i=0; i<6; i++) {
        syndrome <<= 1;

        unsigned int p =
            ( (_v[0] & (1<<(6-i-1))) ? 1 : 0 )+
            liquid_c_ones[ secded2216_P[2*i+0] & _v[1] ] +
            liquid_c_ones[ secded2216_P[2*i+1] & _v[2] ];

        syndrome |= p & 0x01;
    }

    return syndrome;
}

// encode symbol
//  _sym_dec    :   decoded symbol [size: 2 x 1]
//  _sym_enc    :   encoded symbol [size: 3 x 1], _sym_enc[0] has only 6 bits
int fec_secded2216_encode_symbol(unsigned char * _sym_dec,
                                 unsigned char * _sym_enc)
{
    // first six bits is parity block
    _sym_enc[0] = fec_secded2216_compute_parity(_sym_dec);

    // copy last two values
    _sym_enc[1] = _sym_dec[0];
    _sym_enc[2] = _sym_dec[1];
    return LIQUID_OK;
}

// decode symbol, returning 0/1/2 for zero/one/multiple errors
// detected, respectively
//  _sym_enc    :   encoded symbol [size: 3 x 1], _sym_enc[0] has only 6 bits
//  _sym_dec    :   decoded symbol [size: 2 x 1]
int fec_secded2216_decode_symbol(unsigned char * _sym_enc,
                                 unsigned char * _sym_dec)
{
#if 0
    // validate input
    if (_sym_enc[0] >= (1<<6)) {
        fprintf(stderr,"warning, fec_secded2216_decode_symbol(), input symbol too large\n");
    }
#endif

    // estimate error vector
    unsigned char e_hat[3] = {0,0,0};
    int syndrome_flag = fec_secded2216_estimate_ehat(_sym_enc, e_hat);

    // compute estimated transmit vector (last 64 bits of encoded message)
    // NOTE: indices take into account first element in _sym_enc and e_hat
    //       arrays holds the parity bits
    _sym_dec[0] = _sym_enc[1] ^ e_hat[1];
    _sym_dec[1] = _sym_enc[2] ^ e_hat[2];

#if DEBUG_FEC_SECDED2216
    if (syndrome_flag == 1) {
        printf("secded2216_decode_symbol(): single error detected!\n");
    } else if (syndrome_flag == 2) {
        printf("secded2216_decode_symbol(): no match found (multiple errors detected)\n");
    }
#endif

    // return syndrome flag
    return syndrome_flag;
}

// estimate error vector, returning 0/1/2 for zero/one/multiple errors
// detected, respectively
//  _sym_enc    :   encoded symbol [size: 3 x 1], _sym_enc[0] has only 6 bits
//  _e_hat      :   estimated error vector [size: 3 x 1]
int fec_secded2216_estimate_ehat(unsigned char * _sym_enc,
                                 unsigned char * _e_hat)
{
    // clear output array
    _e_hat[0] = 0x00;
    _e_hat[1] = 0x00;
    _e_hat[2] = 0x00;

    // compute syndrome vector, s = r*H^T = ( H*r^T )^T
    unsigned char s = fec_secded2216_compute_syndrome(_sym_enc);

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
        for (n=0; n<22; n++) {
            if (s == secded2216_syndrome_w1[n]) {
                // single error detected at location 'n'
                div_t d = div(n,8);
                _e_hat[3-d.quot-1] = 1 << d.rem;

                return 1;
            }
        }

    }

    // no syndrome match; multiple errors detected
    return 2;
}

// create SEC-DED (22,16) codec object
fec fec_secded2216_create(void * _opts)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    // set scheme
    q->scheme = LIQUID_FEC_SECDED2216;
    q->rate = fec_get_rate(q->scheme);

    // set internal function pointers
    q->encode_func      = &fec_secded2216_encode;
    q->decode_func      = &fec_secded2216_decode;
    q->decode_soft_func = NULL;

    return q;
}

// destroy SEC-DEC (22,16) object
int fec_secded2216_destroy(fec _q)
{
    free(_q);
    return LIQUID_OK;
}

// encode block of data using SEC-DEC (22,16) encoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
int fec_secded2216_encode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc)
{
    unsigned int i=0;       // decoded byte counter
    unsigned int j=0;       // encoded byte counter

    // determine remainder of input length / 8
    unsigned int r = _dec_msg_len % 2;

    // for now simply encode as 2/3-rate codec (eat
    // 2 bits of parity)
    // TODO : make more efficient

    for (i=0; i<_dec_msg_len-r; i+=2) {
        // compute parity (6 bits) on two input bytes (16 bits)
        _msg_enc[j+0] = fec_secded2216_compute_parity(&_msg_dec[i]);

        // copy remaining two input bytes (16 bits)
        _msg_enc[j+1] = _msg_dec[i+0];
        _msg_enc[j+2] = _msg_dec[i+1];

        // increment output counter
        j += 3;
    }

    // if input length isn't divisible by 2, encode last few bytes
    if (r) {
        // one 16-bit symbol (decoded)
        unsigned char m[2] = {_msg_dec[i], 0x00};

        // one 22-bit symbol (encoded)
        unsigned char v[3];

        // encode
        fec_secded2216_encode_symbol(m, v);

        // there is no need to actually send all three bytes;
        // the last byte is zero and can be artificially
        // inserted at the decoder
        _msg_enc[j+0] = v[0];
        _msg_enc[j+1] = v[1];

        i += r;
        j += r+1;
    }

    assert( j == fec_get_enc_msg_length(LIQUID_FEC_SECDED2216,_dec_msg_len) );
    assert( i == _dec_msg_len);
    return LIQUID_OK;
}

// decode block of data using SEC-DEC (22,16) decoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//
//unsigned int
int fec_secded2216_decode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec)
{
    unsigned int i=0;       // decoded byte counter
    unsigned int j=0;       // encoded byte counter
    
    // determine remainder of input length / 8
    unsigned int r = _dec_msg_len % 2;

    for (i=0; i<_dec_msg_len-r; i+=2) {
        // decode straight to output
        fec_secded2216_decode_symbol(&_msg_enc[j], &_msg_dec[i]);

        j += 3;
    }

    // if input length isn't divisible by 2, decode last several bytes
    if (r) {
        // one 22-bit symbol (encoded), with last byte artificially
        // set to '00000000'
        unsigned char v[3] = {_msg_enc[j+0], _msg_enc[j+1], 0x00};

        // one 16-bit symbol (decoded)
        unsigned char m_hat[2];

        // decode symbol
        fec_secded2216_decode_symbol(v, m_hat);

        // copy just first byte to output
        _msg_dec[i] = m_hat[0];

        i += r;
        j += r+1;
    }

    assert( j == fec_get_enc_msg_length(LIQUID_FEC_SECDED2216,_dec_msg_len) );
    assert( i == _dec_msg_len);
    return LIQUID_OK;
}
