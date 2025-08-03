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
// 2/3-rate (12,8) Hamming code
//
//  bit position    1   2   3   4   5   6   7   8   9   10  11  12
//  encoded bits    P1  P2  1   P4  2   3   4   P8  5   6   7   8
//
//  parity bit  P1  x   .   x   .   x   .   x   .   x   .   x   .
//  coveratge   P2  .   x   x   .   .   x   x   .   .   x   x   .
//              P4  .   .   .   x   x   x   x   .   .   .   .   x
//              P8  .   .   .   .   .   .   .   x   x   x   x   x

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FEC_HAMMING128        0   // debugging flag
#define FEC_HAMMING128_ENC_GENTAB   1   // use look-up table for encoding?

// parity bit coverage mask for encoder (collapsed version of figure
// above, stripping out parity bits P1, P2, P4, P8 and only including
// data bits 1:8)
//
// bit position     3   5   6   7   9   10  11  12
//
//  parity bit  P1  x   x   .   x   x   .   x   .   =   1101 1010
//  coverage    P2  x   .   x   x   .   x   x   .   =   1011 0110
//              P4  .   x   x   x   .   .   .   x   =   0111 0001
//              P8  .   .   .   .   x   x   x   x   =   0000 1111
#define HAMMING128_M1   0xda    // 1101 1010
#define HAMMING128_M2   0xb6    // 1011 0110
#define HAMMING128_M4   0x71    // 0111 0001
#define HAMMING128_M8   0x0f    // 0000 1111

// parity bit coverage mask for decoder; used to compute syndromes
// for decoding a received message (see first figure, above).
#define HAMMING128_S1   0x0aaa  // .... 1010 1010 1010
#define HAMMING128_S2   0x0666  // .... 0110 0110 0110
#define HAMMING128_S4   0x01e1  // .... 0001 1110 0001
#define HAMMING128_S8   0x001f  // .... 0000 0001 1111

unsigned int fec_hamming128_encode_symbol(unsigned int _sym_dec)
{
    // validate input
    if (_sym_dec >= (1<<8)) {
        liquid_error(LIQUID_EICONFIG,"fec_hamming128_encode(), input symbol too large");
        return 0;
    }

    // compute parity bits
    unsigned int p1 = liquid_bdotprod_uint16(_sym_dec, HAMMING128_M1);
    unsigned int p2 = liquid_bdotprod_uint16(_sym_dec, HAMMING128_M2);
    unsigned int p4 = liquid_bdotprod_uint16(_sym_dec, HAMMING128_M4);
    unsigned int p8 = liquid_bdotprod_uint16(_sym_dec, HAMMING128_M8);

#if DEBUG_FEC_HAMMING128
    printf("parity bits (p1,p2,p4,p8) : (%1u,%1u,%1u,%1u)\n", p1, p2, p4, p8);
#endif

    // encode symbol by inserting parity bits with data bits to
    // make a 12-bit symbol
    unsigned int sym_enc = ((_sym_dec & 0x000f) << 0) |
                           ((_sym_dec & 0x0070) << 1) |
                           ((_sym_dec & 0x0080) << 2) |
                           ( p1 << 11 ) |
                           ( p2 << 10 ) |
                           ( p4 << 8  ) |
                           ( p8 << 4  );

    return sym_enc;
}

unsigned int fec_hamming128_decode_symbol(unsigned int _sym_enc)
{
    // validate input
    if (_sym_enc >= (1<<12)) {
        liquid_error(LIQUID_EICONFIG,"fec_hamming128_decode(), input symbol too large");
        return 0;
    }

    // compute syndrome bits
    unsigned int s1 = liquid_bdotprod_uint16(_sym_enc, HAMMING128_S1);
    unsigned int s2 = liquid_bdotprod_uint16(_sym_enc, HAMMING128_S2);
    unsigned int s4 = liquid_bdotprod_uint16(_sym_enc, HAMMING128_S4);
    unsigned int s8 = liquid_bdotprod_uint16(_sym_enc, HAMMING128_S8);

    // index
    unsigned int z = (s8<<3) | (s4<<2) | (s2<<1) | s1;

#if DEBUG_FEC_HAMMING128
    printf("syndrome bits (s1,s2,s4,s8) : (%1u,%1u,%1u,%1u)\n", s1, s2, s4, s8);
    printf("syndrome z : %u\n", z);
#endif

    // flip bit at this position
    if (z) {
        if (z > 12) {
            // if this happens there are likely too many errors
            // to correct; just pass without trying to do anything
            //fprintf(stderr,"warning, fec_hamming128_decode_symbol(), syndrome index exceeds 12\n");
        } else {
            //printf("error detected!\n");
            _sym_enc ^= 1 << (12-z);
        }
    }

    // strip data bits (x) from encoded symbol with parity bits (.)
    //      symbol:  [..x. xxx. xxxx]
    //                0000 0000 1111     >  0x000f
    //                0000 1110 0000     >  0x00e0
    //                0010 0000 0000     >  0x0200
    unsigned int sym_dec = ((_sym_enc & 0x000f)     )   |
                           ((_sym_enc & 0x00e0) >> 1)   |
                           ((_sym_enc & 0x0200) >> 2);


    return sym_dec;
}

// create Hamming(12,8) codec object
fec fec_hamming128_create(void * _opts)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    // set scheme
    q->scheme = LIQUID_FEC_HAMMING128;
    q->rate = fec_get_rate(q->scheme);

    // set internal function pointers
    q->encode_func      = &fec_hamming128_encode;
    q->decode_func      = &fec_hamming128_decode;
    q->decode_soft_func = &fec_hamming128_decode_soft;

    return q;
}

// destroy Hamming(12,8) object
int fec_hamming128_destroy(fec _q)
{
    free(_q);
    return LIQUID_OK;
}

// encode block of data using Hamming(12,8) encoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
int fec_hamming128_encode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc)
{
    unsigned int i, j=0;    // input/output symbol counters
    unsigned char s0, s1;   // input 8-bit symbols
    unsigned int m0, m1;    // output 12-bit symbols

    // determine if input length is odd
    unsigned int r = _dec_msg_len % 2;

    for (i=0; i<_dec_msg_len-r; i+=2) {
        // strip two input bytes
        s0 = _msg_dec[i+0];
        s1 = _msg_dec[i+1];

        // encode each byte into 12-bit symbols
#if FEC_HAMMING128_ENC_GENTAB
        m0 = hamming128_enc_gentab[s0];
        m1 = hamming128_enc_gentab[s1];
#else
        m0 = fec_hamming128_encode_symbol(s0);
        m1 = fec_hamming128_encode_symbol(s1);
#endif

        // append both 12-bit symbols to output (three 8-bit bytes),
        // retaining order of bits in output
        _msg_enc[j+0] =  (m0 >> 4) & 0xff;
        _msg_enc[j+1] = ((m0 << 4) & 0xf0) | ((m1 >> 8) & 0x0f);
        _msg_enc[j+2] =  (m1     ) & 0xff;

        j += 3;
    }

    // if input length is even, encode last symbol by itself
    if (r) {
        // strip last input symbol
        s0 = _msg_dec[_dec_msg_len-1];

        // encode into 12-bit symbol
#if FEC_HAMMING128_ENC_GENTAB
        m0 = hamming128_enc_gentab[s0];
#else
        m0 = fec_hamming128_encode_symbol(s0);
#endif

        // append to output
        _msg_enc[j+0] = ( m0 & 0x0ff0 ) >> 4;
        _msg_enc[j+1] = ( m0 & 0x000f ) << 4;

        j += 2;
    }

    assert(j== fec_get_enc_msg_length(LIQUID_FEC_HAMMING128,_dec_msg_len));
    return LIQUID_OK;
}

// decode block of data using Hamming(12,8) decoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_enc        :   encoded message [size: 1 x 2*_dec_msg_len]
//  _msg_dec        :   decoded message [size: 1 x _dec_msg_len]
//
//unsigned int
int fec_hamming128_decode(fec             _q,
                          unsigned int    _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec)
{
    unsigned int i=0,j=0;
    unsigned int r = _dec_msg_len % 2;
    unsigned char r0, r1, r2;
    unsigned int m0, m1;

    for (i=0; i<_dec_msg_len-r; i+=2) {
        // strip three input symbols
        r0 = _msg_enc[j+0];
        r1 = _msg_enc[j+1];
        r2 = _msg_enc[j+2];

        // combine three 8-bit symbols into two 12-bit symbols
        m0 = ((r0 << 4) & 0x0ff0) | ((r1 >> 4) & 0x000f);
        m1 = ((r1 << 8) & 0x0f00) | ((r2     ) & 0x00ff);

        // decode each symbol into an 8-bit byte
        _msg_dec[i+0] = fec_hamming128_decode_symbol(m0);
        _msg_dec[i+1] = fec_hamming128_decode_symbol(m1);

        j += 3;
    }

    // if input length is even, decode last symbol by itself
    if (r) {
        // strip last two input bytes (last byte should only contain
        // for bits)
        r0 = _msg_enc[j+0];
        r1 = _msg_enc[j+1];

        // pack into 12-bit symbol
        m0 = ((r0 << 4) & 0x0ff0) | ((r1 >> 4) & 0x000f);

        // decode symbol into an 8-bit byte
        _msg_dec[i++] = fec_hamming128_decode_symbol(m0);

        j += 2;
    }

    assert(j== fec_get_enc_msg_length(LIQUID_FEC_HAMMING128,_dec_msg_len));
    return LIQUID_OK;
}


// decode block of data using Hamming(12,8) soft decoder
//
//  _q              :   encoder/decoder object
//  _dec_msg_len    :   decoded message length (number of bytes)
//  _msg_enc        :   encoded message [size: 8*_enc_msg_len x 1]
//  _msg_dec        :   decoded message [size: _dec_msg_len x 1]
//
//unsigned int
int fec_hamming128_decode_soft(fec             _q,
                               unsigned int    _dec_msg_len,
                               unsigned char * _msg_enc,
                               unsigned char * _msg_dec)
{
    unsigned int i;
    unsigned int k=0;       // array bit index
    unsigned int r = _dec_msg_len % 2;

    // compute encoded message length
    unsigned int enc_msg_len = (3*_dec_msg_len)/2 + r;

    unsigned char s;    // decoded 8-bit symbol

    //unsigned char num_errors=0;
    for (i=0; i<_dec_msg_len; i++) {
#if 0
        // use true ML soft decoding: about 1.45 dB improvement in Eb/N_0 for a BER of 10^-5
        // with a decoding complexity of 1.43M cycles/trial (64-byte block)
        s = fecsoft_hamming128_decode(&_msg_enc[k]);
#else
        // use n-3 nearest neighbors: about 0.54 dB improvement in Eb/N_0 for a BER of 10^-5
        // with a decoding complexity of 124k cycles/trial (64-byte block)
        s = fecsoft_hamming128_decode_n3(&_msg_enc[k]);
#endif
        k += 12;

        _msg_dec[i] = (unsigned char)(s & 0xff);

        //printf("  %3u : 0x%.2x > 0x%.2x,  0x%.2x > 0x%.2x (k=%u)\n", i, r0, s0, r1, s1, k);
    }
    k += r*4;   // for assert method
    assert(k == 8*enc_msg_len);
    return LIQUID_OK;
}

// 
// internal methods
//

// soft decoding of one symbol
// NOTE : because this method compares the received symbol to every
//        possible (256) encoded symbols, it is painfully slow to
//        run.
unsigned int fecsoft_hamming128_decode(unsigned char * _soft_bits)
{
    // find symbol with minimum distance from all 2^8 possible
    unsigned int d;             // distance metric
    unsigned int dmin = 0;      // minimum distance
    unsigned int s_hat = 0;     // estimated transmitted symbol
    unsigned int c;             // encoded symbol

    unsigned int s;
    for (s=0; s<256; s++) {
        // encode symbol
#if FEC_HAMMING128_ENC_GENTAB
        c = hamming128_enc_gentab[s];
#else
        c = fec_hamming128_encode_symbol(s);
#endif

        // compute distance metric
        d = 0;
        d += (c & 0x0800) ? 255 - _soft_bits[ 0] : _soft_bits[ 0];
        d += (c & 0x0400) ? 255 - _soft_bits[ 1] : _soft_bits[ 1];
        d += (c & 0x0200) ? 255 - _soft_bits[ 2] : _soft_bits[ 2];
        d += (c & 0x0100) ? 255 - _soft_bits[ 3] : _soft_bits[ 3];
        d += (c & 0x0080) ? 255 - _soft_bits[ 4] : _soft_bits[ 4];
        d += (c & 0x0040) ? 255 - _soft_bits[ 5] : _soft_bits[ 5];
        d += (c & 0x0020) ? 255 - _soft_bits[ 6] : _soft_bits[ 6];
        d += (c & 0x0010) ? 255 - _soft_bits[ 7] : _soft_bits[ 7];
        d += (c & 0x0008) ? 255 - _soft_bits[ 8] : _soft_bits[ 8];
        d += (c & 0x0004) ? 255 - _soft_bits[ 9] : _soft_bits[ 9];
        d += (c & 0x0002) ? 255 - _soft_bits[10] : _soft_bits[10];
        d += (c & 0x0001) ? 255 - _soft_bits[11] : _soft_bits[11];

        if (d < dmin || s==0) {
            s_hat = s;
            dmin = d;
        }
    }
    return s_hat;
}

// soft decoding of one symbol using nearest neighbors
unsigned int fecsoft_hamming128_decode_n3(unsigned char * _soft_bits)
{
    // find symbol with minimum distance from 17 nearest neighbors
    unsigned int d;             // distance metric
    unsigned int dmin = 0;      // minimum distance
    unsigned int s_hat = 0;     // estimated transmitted symbol
    unsigned int c;             // encoded symbol

    // compute hard-decoded symbol
    c = 0x0000;
    c |= (_soft_bits[ 0] > 127) ? 0x0800 : 0;
    c |= (_soft_bits[ 1] > 127) ? 0x0400 : 0;
    c |= (_soft_bits[ 2] > 127) ? 0x0200 : 0;
    c |= (_soft_bits[ 3] > 127) ? 0x0100 : 0;
    c |= (_soft_bits[ 4] > 127) ? 0x0080 : 0;
    c |= (_soft_bits[ 5] > 127) ? 0x0040 : 0;
    c |= (_soft_bits[ 6] > 127) ? 0x0020 : 0;
    c |= (_soft_bits[ 7] > 127) ? 0x0010 : 0;
    c |= (_soft_bits[ 8] > 127) ? 0x0008 : 0;
    c |= (_soft_bits[ 9] > 127) ? 0x0004 : 0;
    c |= (_soft_bits[10] > 127) ? 0x0002 : 0;
    c |= (_soft_bits[11] > 127) ? 0x0001 : 0;

    // decode symbol
    s_hat = fec_hamming128_decode_symbol(c);

    // re-encode and compute distance
#if FEC_HAMMING128_ENC_GENTAB
    c = hamming128_enc_gentab[s_hat];
#else
    c = fec_hamming128_encode_symbol(s_hat);
#endif

    // compute distance metric
    d = 0;
    d += (c & 0x0800) ? 255 - _soft_bits[ 0] : _soft_bits[ 0];
    d += (c & 0x0400) ? 255 - _soft_bits[ 1] : _soft_bits[ 1];
    d += (c & 0x0200) ? 255 - _soft_bits[ 2] : _soft_bits[ 2];
    d += (c & 0x0100) ? 255 - _soft_bits[ 3] : _soft_bits[ 3];
    d += (c & 0x0080) ? 255 - _soft_bits[ 4] : _soft_bits[ 4];
    d += (c & 0x0040) ? 255 - _soft_bits[ 5] : _soft_bits[ 5];
    d += (c & 0x0020) ? 255 - _soft_bits[ 6] : _soft_bits[ 6];
    d += (c & 0x0010) ? 255 - _soft_bits[ 7] : _soft_bits[ 7];
    d += (c & 0x0008) ? 255 - _soft_bits[ 8] : _soft_bits[ 8];
    d += (c & 0x0004) ? 255 - _soft_bits[ 9] : _soft_bits[ 9];
    d += (c & 0x0002) ? 255 - _soft_bits[10] : _soft_bits[10];
    d += (c & 0x0001) ? 255 - _soft_bits[11] : _soft_bits[11];

    dmin = d;

    // search over 17 nearest neighbors
    unsigned int s;
    unsigned int i;
    for (i=0; i<17; i++) {
        // use look-up table for nearest neighbors
        s = fecsoft_hamming128_n3[s_hat][i];

        // encode symbol
#if FEC_HAMMING128_ENC_GENTAB
        c = hamming128_enc_gentab[s];
#else
        c = fec_hamming128_encode_symbol(s);
#endif

        // compute distance metric
        d = 0;
        d += (c & 0x0800) ? 255 - _soft_bits[ 0] : _soft_bits[ 0];
        d += (c & 0x0400) ? 255 - _soft_bits[ 1] : _soft_bits[ 1];
        d += (c & 0x0200) ? 255 - _soft_bits[ 2] : _soft_bits[ 2];
        d += (c & 0x0100) ? 255 - _soft_bits[ 3] : _soft_bits[ 3];
        d += (c & 0x0080) ? 255 - _soft_bits[ 4] : _soft_bits[ 4];
        d += (c & 0x0040) ? 255 - _soft_bits[ 5] : _soft_bits[ 5];
        d += (c & 0x0020) ? 255 - _soft_bits[ 6] : _soft_bits[ 6];
        d += (c & 0x0010) ? 255 - _soft_bits[ 7] : _soft_bits[ 7];
        d += (c & 0x0008) ? 255 - _soft_bits[ 8] : _soft_bits[ 8];
        d += (c & 0x0004) ? 255 - _soft_bits[ 9] : _soft_bits[ 9];
        d += (c & 0x0002) ? 255 - _soft_bits[10] : _soft_bits[10];
        d += (c & 0x0001) ? 255 - _soft_bits[11] : _soft_bits[11];

        if (d < dmin) {
            s_hat = s;
            dmin = d;
        }
    }

    return s_hat;
}

