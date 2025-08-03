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
// FEC, repeat code
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liquid.internal.h"

// create rep5 codec object
fec fec_rep5_create(void * _opts)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    q->scheme = LIQUID_FEC_REP5;
    q->rate = fec_get_rate(q->scheme);

    q->encode_func      = &fec_rep5_encode;
    q->decode_func      = &fec_rep5_decode;
    q->decode_soft_func = &fec_rep5_decode_soft;

    return q;
}

// destroy rep5 object
int fec_rep5_destroy(fec _q)
{
    free(_q);
    return LIQUID_OK;
}

// print rep5 object
int fec_rep5_print(fec _q)
{
    printf("fec_rep5 [r: %3.2f]\n", _q->rate);
    return LIQUID_OK;
}

// encode block of data using rep5 encoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//  _msg_enc        :   encoded message [size: 1 x 5*_dec_msg_len]
int fec_rep5_encode(fec             _q,
                    unsigned int    _dec_msg_len,
                    unsigned char * _msg_dec,
                    unsigned char * _msg_enc)
{
    unsigned int i;
    for (i=0; i<5; i++) {
        memcpy(&_msg_enc[i*_dec_msg_len], _msg_dec, _dec_msg_len);
    }
    return LIQUID_OK;
}

// decode block of data using rep5 decoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_enc        :   encoded message [size: 1 x 5*_dec_msg_len]
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
int fec_rep5_decode(fec             _q,
                    unsigned int    _dec_msg_len,
                    unsigned char * _msg_enc,
                    unsigned char * _msg_dec)
{
    unsigned char s0, s1, s2, s3, s4;
    unsigned int i;
    //unsigned int num_errors=0;
    for (i=0; i<_dec_msg_len; i++) {
        s0 = _msg_enc[i                 ];
        s1 = _msg_enc[i +   _dec_msg_len];
        s2 = _msg_enc[i + 2*_dec_msg_len];
        s3 = _msg_enc[i + 3*_dec_msg_len];
        s4 = _msg_enc[i + 4*_dec_msg_len];

        // compute all triplet combinations
        _msg_dec[i] =   (s0 & s1 & s2) |
                        (s0 & s1 & s3) |
                        (s0 & s1 & s4) |
                        (s0 & s2 & s3) |
                        (s0 & s2 & s4) |
                        (s0 & s3 & s4) |
                        (s1 & s2 & s3) |
                        (s1 & s2 & s4) |
                        (s1 & s3 & s4) |
                        (s2 & s3 & s4);
    
        //num_errors += (s0 ^ s1) | (s0 ^ s2) | (s1 ^ s2) ? 1 : 0;
        //num_errors += 0;
    }
    return LIQUID_OK;
}

// decode block of data using rep5 decoder (soft metrics)
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_enc        :   encoded message [size: 1 x 5*_dec_msg_len]
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
int fec_rep5_decode_soft(fec             _q,
                         unsigned int    _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec)
{
    unsigned char s0, s1, s2, s3, s4;
    unsigned int i;
    unsigned int j;
    unsigned int s_hat;
    //unsigned int num_errors=0;
    for (i=0; i<_dec_msg_len; i++) {

        // clear decoded message
        _msg_dec[i] = 0x00;

        for (j=0; j<8; j++) {
            s0 = _msg_enc[8*i                    + j];
            s1 = _msg_enc[8*(i +   _dec_msg_len) + j];
            s2 = _msg_enc[8*(i + 2*_dec_msg_len) + j];
            s3 = _msg_enc[8*(i + 3*_dec_msg_len) + j];
            s4 = _msg_enc[8*(i + 4*_dec_msg_len) + j];

            // average three symbols and make decision
            s_hat = (s0 + s1 + s2 + s3 + s4)/5;

            _msg_dec[i] |= (s_hat > LIQUID_SOFTBIT_ERASURE) ? (1 << (8-j-1)) : 0x00;
            
        }
    }
    return LIQUID_OK;
}
