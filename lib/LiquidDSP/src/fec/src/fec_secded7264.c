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
// SEC-DED (72,64) 8/9-rate forward error-correction block code
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

#define DEBUG_FEC_SECDED7264 0

// P matrix [8 x 64]
//  11111111 00001111 00001111 00001100 01101000 10001000 10001000 10000000 : 
//  11110000 11111111 00000000 11110011 01100100 01000100 01000100 01000000 : 
//  00110000 11110000 11111111 00001111 00000010 00100010 00100010 00100110 : 
//  11001111 00000000 11110000 11111111 00000001 00010001 00010001 00010110 : 
//  01101000 10001000 10001000 10000000 11111111 00001111 00000000 11110011 : 
//  01100100 01000100 01000100 01000000 11110000 11111111 00001111 00001100 : 
//  00000010 00100010 00100010 00100110 11001111 00000000 11111111 00001111 : 
//  00000001 00010001 00010001 00010110 00110000 11110000 11110000 11111111 : 
unsigned char secded7264_P[64] = {
    0xFF, 0x0F, 0x0F, 0x0C, 0x68, 0x88, 0x88, 0x80,
    0xF0, 0xFF, 0x00, 0xF3, 0x64, 0x44, 0x44, 0x40,
    0x30, 0xF0, 0xFF, 0x0F, 0x02, 0x22, 0x22, 0x26,
    0xCF, 0x00, 0xF0, 0xFF, 0x01, 0x11, 0x11, 0x16,
    0x68, 0x88, 0x88, 0x80, 0xFF, 0x0F, 0x00, 0xF3,
    0x64, 0x44, 0x44, 0x40, 0xF0, 0xFF, 0x0F, 0x0C,
    0x02, 0x22, 0x22, 0x26, 0xCF, 0x00, 0xFF, 0x0F,
    0x01, 0x11, 0x11, 0x16, 0x30, 0xF0, 0xF0, 0xFF};

// syndrome vectors for errors of weight 1
unsigned char secded7264_syndrome_w1[72] = {
    0x0b, 0x3b, 0x37, 0x07, 0x19, 0x29, 0x49, 0x89,
    0x16, 0x26, 0x46, 0x86, 0x13, 0x23, 0x43, 0x83,
    0x1c, 0x2c, 0x4c, 0x8c, 0x15, 0x25, 0x45, 0x85,
    0x1a, 0x2a, 0x4a, 0x8a, 0x0d, 0xcd, 0xce, 0x0e,
    0x70, 0x73, 0xb3, 0xb0, 0x51, 0x52, 0x54, 0x58,
    0xa1, 0xa2, 0xa4, 0xa8, 0x31, 0x32, 0x34, 0x38,
    0xc1, 0xc2, 0xc4, 0xc8, 0x61, 0x62, 0x64, 0x68,
    0x91, 0x92, 0x94, 0x98, 0xe0, 0xec, 0xdc, 0xd0,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};


// compute parity byte on 64-byte input
unsigned char fec_secded7264_compute_parity(unsigned char * _v)
{
    // compute parity byte on message
    unsigned int i;
    unsigned char parity = 0x00;
    for (i=0; i<8; i++) {
        parity <<= 1;

        unsigned int p = liquid_c_ones[ secded7264_P[8*i+0] & _v[0] ] +
                         liquid_c_ones[ secded7264_P[8*i+1] & _v[1] ] +
                         liquid_c_ones[ secded7264_P[8*i+2] & _v[2] ] +
                         liquid_c_ones[ secded7264_P[8*i+3] & _v[3] ] +
                         liquid_c_ones[ secded7264_P[8*i+4] & _v[4] ] +
                         liquid_c_ones[ secded7264_P[8*i+5] & _v[5] ] +
                         liquid_c_ones[ secded7264_P[8*i+6] & _v[6] ] +
                         liquid_c_ones[ secded7264_P[8*i+7] & _v[7] ];

        parity |= p & 0x01;
    }

    // return parity byte
    return parity;
}

// compute syndrome on 72-bit input
unsigned char fec_secded7264_compute_syndrome(unsigned char * _v)
{
    // TODO : unwrap this loop
    unsigned int i;
    unsigned char syndrome = 0x00;
    for (i=0; i<8; i++) {
        syndrome <<= 1;

        // compute parity bit
        unsigned int p =
            ( (_v[0] & (1<<(8-i-1))) ? 1 : 0 ) +
            liquid_c_ones[ secded7264_P[8*i+0] & _v[1] ] +
            liquid_c_ones[ secded7264_P[8*i+1] & _v[2] ] +
            liquid_c_ones[ secded7264_P[8*i+2] & _v[3] ] +
            liquid_c_ones[ secded7264_P[8*i+3] & _v[4] ] +
            liquid_c_ones[ secded7264_P[8*i+4] & _v[5] ] +
            liquid_c_ones[ secded7264_P[8*i+5] & _v[6] ] +
            liquid_c_ones[ secded7264_P[8*i+6] & _v[7] ] +
            liquid_c_ones[ secded7264_P[8*i+7] & _v[8] ];

        syndrome |= p & 0x01;
    }

    return syndrome;
}

int fec_secded7264_encode_symbol(unsigned char * _sym_dec,
                                 unsigned char * _sym_enc)
{
    // compute parity on input
    _sym_enc[0] = fec_secded7264_compute_parity(_sym_dec);

    // copy input to output
    _sym_enc[1] = _sym_dec[0];
    _sym_enc[2] = _sym_dec[1];
    _sym_enc[3] = _sym_dec[2];
    _sym_enc[4] = _sym_dec[3];
    _sym_enc[5] = _sym_dec[4];
    _sym_enc[6] = _sym_dec[5];
    _sym_enc[7] = _sym_dec[6];
    _sym_enc[8] = _sym_dec[7];
    return LIQUID_OK;
}

// decode symbol, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol [size: 9 x 1]
//  _sym_dec    :   decoded symbol [size: 8 x 1]
int fec_secded7264_decode_symbol(unsigned char * _sym_enc,
                                 unsigned char * _sym_dec)
{
    // estimate error vector
    unsigned char e_hat[9] = {0,0,0,0,0,0,0,0,0};
    int syndrome_flag = fec_secded7264_estimate_ehat(_sym_enc, e_hat);

    // compute estimated transmit vector (last 64 bits of encoded message)
    // NOTE: indices take into account first element in _sym_enc and e_hat
    //       arrays holds the parity bits
    _sym_dec[0] = _sym_enc[1] ^ e_hat[1];
    _sym_dec[1] = _sym_enc[2] ^ e_hat[2];
    _sym_dec[2] = _sym_enc[3] ^ e_hat[3];
    _sym_dec[3] = _sym_enc[4] ^ e_hat[4];
    _sym_dec[4] = _sym_enc[5] ^ e_hat[5];
    _sym_dec[5] = _sym_enc[6] ^ e_hat[6];
    _sym_dec[6] = _sym_enc[7] ^ e_hat[7];
    _sym_dec[7] = _sym_enc[8] ^ e_hat[8];

#if DEBUG_FEC_SECDED7264
    if (syndrome_flag == 1) {
        printf("secded7264_decode_symbol(): single error detected!\n");
    } else if (syndrome_flag == 2) {
        printf("secded7264_decode_symbol(): no match found (multiple errors detected)\n");
    }
#endif

    // return syndrome flag
    return syndrome_flag;
}

// estimate error vector, returning 0/1/2 for zero/one/multiple errors
// detected, respectively
//  _sym_enc    :   encoded symbol [size: 9 x 1],
//  _e_hat      :   estimated error vector [size: 9 x 1]
int fec_secded7264_estimate_ehat(unsigned char * _sym_enc,
                                 unsigned char * _e_hat)
{
    // clear output array
    memset(_e_hat, 0x00, 9*sizeof(unsigned char));

    // compute syndrome vector, s = r*H^T = ( H*r^T )^T
    unsigned char s = fec_secded7264_compute_syndrome(_sym_enc);

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
        for (n=0; n<72; n++) {
            if (s == secded7264_syndrome_w1[n]) {
                // single error detected at location 'n'
                div_t d = div(n,8);
                _e_hat[9-d.quot-1] = 1 << d.rem;

                return 1;
            }
        }

    }

    // no syndrome match; multiple errors detected
    return 2;
}

// create SEC-DED (72,64) codec object
fec fec_secded7264_create(void * _opts)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    // set scheme
    q->scheme = LIQUID_FEC_SECDED7264;
    q->rate = fec_get_rate(q->scheme);

    // set internal function pointers
    q->encode_func      = &fec_secded7264_encode;
    q->decode_func      = &fec_secded7264_decode;
    q->decode_soft_func = NULL;

    return q;
}

// destroy SEC-DEC (72,64) object
int fec_secded7264_destroy(fec _q)
{
    free(_q);
    return LIQUID_OK;
}

// encode block of data using SEC-DEC (72,64) encoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
int fec_secded7264_encode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc)
{
    unsigned int i=0;       // decoded byte counter
    unsigned int j=0;       // encoded byte counter
    unsigned char parity;   // parity byte

    // determine remainder of input length / 8
    unsigned int r = _dec_msg_len % 8;

    // TODO : devise more efficient way of doing this
    for (i=0; i<_dec_msg_len-r; i+=8) {
        // encode directly to output
        fec_secded7264_encode_symbol(&_msg_dec[i], &_msg_enc[j]);

        j += 9;
    }

    // if input length isn't divisible by 8, encode last few bytes
    if (r) {
        unsigned char v[8] = {0,0,0,0,0,0,0,0};
        unsigned int n;
        for (n=0; n<r; n++)
            v[n] = _msg_dec[i+n];

        // compute parity
        parity = fec_secded7264_compute_parity(v);
        
        // there is no need to actually send all the bytes; the
        // last 8-r bytes are zeros and can be added at the
        // decoder
        _msg_enc[j+0] = parity;
        for (n=0; n<r; n++)
            _msg_enc[j+n+1] = _msg_dec[i+n];

        i += r;
        j += r+1;
    }

    assert( j == fec_get_enc_msg_length(LIQUID_FEC_SECDED7264,_dec_msg_len) );
    assert( i == _dec_msg_len);
    return LIQUID_OK;
}

// decode block of data using SEC-DEC (72,64) decoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//
//unsigned int
int fec_secded7264_decode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec)
{
    unsigned int i=0;       // decoded byte counter
    unsigned int j=0;       // encoded byte counter
    
    // determine remainder of input length / 8
    unsigned int r = _dec_msg_len % 8;

    for (i=0; i<_dec_msg_len-r; i+=8) {
        // decode nine input bytes
        fec_secded7264_decode_symbol(&_msg_enc[j], &_msg_dec[i]);

        j += 9;
    }


    // if input length isn't divisible by 8, decode last several bytes
    if (r) {
        unsigned char v[9] = {0,0,0,0,0,0,0,0,0};   // received message
        unsigned char c[8] = {0,0,0,0,0,0,0,0};     // decoded message

        unsigned int n;
        // output length is input + 1 (parity byte)
        for (n=0; n<r+1; n++)
            v[n] = _msg_enc[j+n];

        // decode symbol
        fec_secded7264_decode_symbol(v,c);

        // store only relevant bytes
        for (n=0; n<r; n++)
            _msg_dec[i+n] = c[n];
        
        i += r;
        j += r+1;
    }

    assert( j == fec_get_enc_msg_length(LIQUID_FEC_SECDED7264,_dec_msg_len) );
    assert( i == _dec_msg_len);
    return LIQUID_OK;
}

