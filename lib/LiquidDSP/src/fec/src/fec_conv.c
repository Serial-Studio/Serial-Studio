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
// convolutional code (macros)
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "liquid.internal.h"

#define VERBOSE_FEC_CONV    0

#if LIBFEC_ENABLED
#include "fec.h"

fec fec_conv_create(fec_scheme _fs)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    q->scheme = _fs;
    q->rate = fec_get_rate(q->scheme);

    q->encode_func      = &fec_conv_encode;
    q->decode_func      = &fec_conv_decode_hard; // default to hard-decision decoding
    q->decode_soft_func = &fec_conv_decode_soft;

    switch (q->scheme) {
    case LIQUID_FEC_CONV_V27:  fec_conv_init_v27(q);   break;
    case LIQUID_FEC_CONV_V29:  fec_conv_init_v29(q);   break;
    case LIQUID_FEC_CONV_V39:  fec_conv_init_v39(q);   break;
    case LIQUID_FEC_CONV_V615: fec_conv_init_v615(q);  break;
    default:
        return liquid_error_config("fec_conv_create(), invalid type");
    }

    // convolutional-specific decoding
    q->num_dec_bytes = 0;
    q->enc_bits = NULL;
    q->vp = NULL;

    return q;
}

int fec_conv_destroy(fec _q)
{
    // delete viterbi decoder
    if (_q->vp != NULL)
        _q->delete_viterbi(_q->vp);

    if (_q->enc_bits != NULL)
        free(_q->enc_bits);

    free(_q);
    return LIQUID_OK;
}

int fec_conv_encode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char *_msg_dec,
                    unsigned char *_msg_enc)
{
    unsigned int i,j,r; // bookkeeping
    unsigned int sr=0;  // convolutional shift register
    unsigned int n=0;   // output bit counter

    unsigned char bit;
    unsigned char byte_in;
    unsigned char byte_out=0;

    for (i=0; i<_dec_msg_len; i++) {
        byte_in = _msg_dec[i];

        // break byte into individual bits
        for (j=0; j<8; j++) {
            // shift bit starting with most significant
            bit = (byte_in >> (7-j)) & 0x01;
            sr = (sr << 1) | bit;

            // compute parity bits for each polynomial
            for (r=0; r<_q->R; r++) {
                byte_out = (byte_out<<1) | parity(sr & _q->poly[r]);
                _msg_enc[n/8] = byte_out;
                n++;
            }
        }
    }

    // tail bits
    for (i=0; i<(_q->K)-1; i++) {
        // shift register: push zeros
        sr = (sr << 1);

        // compute parity bits for each polynomial
        for (r=0; r<_q->R; r++) {
            byte_out = (byte_out<<1) | parity(sr & _q->poly[r]);
            _msg_enc[n/8] = byte_out;
            n++;
        }
    }

    // ensure even number of bytes
    while (n%8) {
        // shift zeros
        byte_out <<= 1;
        _msg_enc[n/8] = byte_out;
        n++;
    }

    assert(n == 8*fec_get_enc_msg_length(_q->scheme,_dec_msg_len));
    return LIQUID_OK;
}

//unsigned int
int fec_conv_decode_hard(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char *_msg_enc,
                         unsigned char *_msg_dec)
{
    // re-allocate resources if necessary
    fec_conv_setlength(_q, _dec_msg_len);

    // unpack bytes
    unsigned int num_written;
    liquid_unpack_bytes(_msg_enc,               // encoded message (bytes)
                        _q->num_enc_bytes,      // encoded message length (#bytes)
                        _q->enc_bits,           // encoded message (bits)
                        _q->num_enc_bytes*8,    // encoded message length (#bits)
                        &num_written);

#if VERBOSE_FEC_CONV
    unsigned int i;
    printf("msg encoded (bits):\n");
    for (i=0; i<8*_q->num_enc_bytes; i++) {
        printf("%1u", _q->enc_bits[i]);
        if (((i+1)%8)==0)
            printf(" ");
    }
    printf("\n");
#endif

    // invoke hard-decision scaling
    unsigned int k;
    for (k=0; k<8*_q->num_enc_bytes; k++)
        _q->enc_bits[k] = _q->enc_bits[k] ? LIQUID_SOFTBIT_1 : LIQUID_SOFTBIT_0;

    // run internal decoder
    return fec_conv_decode(_q, _msg_dec);
}

//unsigned int
int fec_conv_decode_soft(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char *_msg_enc,
                         unsigned char *_msg_dec)
{
    // re-allocate resources if necessary
    fec_conv_setlength(_q, _dec_msg_len);

    // copy soft input bits
    unsigned int k;
    for (k=0; k<8*_q->num_enc_bytes; k++)
        _q->enc_bits[k] = _msg_enc[k];

    // run internal decoder
    return fec_conv_decode(_q, _msg_dec);
}

//unsigned int
int fec_conv_decode(fec _q, unsigned char * _msg_dec)
{
    // run decoder
    _q->init_viterbi(_q->vp,0);
    _q->update_viterbi_blk(_q->vp, _q->enc_bits, 8*_q->num_dec_bytes+_q->K-1);
    _q->chainback_viterbi(_q->vp, _msg_dec, 8*_q->num_dec_bytes, 0);

#if VERBOSE_FEC_CONV
    for (i=0; i<_dec_msg_len; i++)
        printf("%.2x ", _msg_dec[i]);
    printf("\n");
#endif
    return LIQUID_OK;
}

int fec_conv_setlength(fec _q, unsigned int _dec_msg_len)
{
    // re-allocate resources as necessary
    unsigned int num_dec_bytes = _dec_msg_len;

    // return if length has not changed
    if (num_dec_bytes == _q->num_dec_bytes)
        return LIQUID_OK;

#if VERBOSE_FEC_CONV
    printf("(re)creating viterbi decoder, %u frame bytes\n", num_dec_bytes);
#endif

    // reset number of framebits
    _q->num_dec_bytes = num_dec_bytes;
    _q->num_enc_bytes = fec_get_enc_msg_length(_q->scheme,
                                               _dec_msg_len);

    // delete old decoder if necessary
    if (_q->vp != NULL)
        _q->delete_viterbi(_q->vp);

    // re-create / re-allocate memory buffers
    _q->vp = _q->create_viterbi(8*_q->num_dec_bytes);
    //_q->set_viterbi_polynomial(_q->poly);
    _q->enc_bits = (unsigned char*) realloc(_q->enc_bits,
                                            _q->num_enc_bytes*8*sizeof(unsigned char));
    return LIQUID_OK;
}

// 
// internal
//

int fec_conv_init_v27(fec _q)
{
    _q->R=2;
    _q->K=7;
    _q->poly = fec_conv27_poly;
    _q->create_viterbi = create_viterbi27;
    //_q->set_viterbi_polynomial = set_viterbi27_polynomial;
    _q->init_viterbi = init_viterbi27;
    _q->update_viterbi_blk = update_viterbi27_blk;
    _q->chainback_viterbi = chainback_viterbi27;
    _q->delete_viterbi = delete_viterbi27;
    return LIQUID_OK;
}

int fec_conv_init_v29(fec _q)
{
    _q->R=2;
    _q->K=9;
    _q->poly = fec_conv29_poly;
    _q->create_viterbi = create_viterbi29;
    //_q->set_viterbi_polynomial = set_viterbi29_polynomial;
    _q->init_viterbi = init_viterbi29;
    _q->update_viterbi_blk = update_viterbi29_blk;
    _q->chainback_viterbi = chainback_viterbi29;
    _q->delete_viterbi = delete_viterbi29;
    return LIQUID_OK;
}

int fec_conv_init_v39(fec _q)
{
    _q->R=3;
    _q->K=9;
    _q->poly = fec_conv39_poly;
    _q->create_viterbi = create_viterbi39;
    //_q->set_viterbi_polynomial = set_viterbi39_polynomial;
    _q->init_viterbi = init_viterbi39;
    _q->update_viterbi_blk = update_viterbi39_blk;
    _q->chainback_viterbi = chainback_viterbi39;
    _q->delete_viterbi = delete_viterbi39;
    return LIQUID_OK;
}

int fec_conv_init_v615(fec _q)
{
    _q->R=6;
    _q->K=15;
    _q->poly = fec_conv615_poly;
    _q->create_viterbi = create_viterbi615;
    //_q->set_viterbi_polynomial = set_viterbi615_polynomial;
    _q->init_viterbi = init_viterbi615;
    _q->update_viterbi_blk = update_viterbi615_blk;
    _q->chainback_viterbi = chainback_viterbi615;
    _q->delete_viterbi = delete_viterbi615;
    return LIQUID_OK;
}



#else   // LIBFEC_ENABLED

fec fec_conv_create(fec_scheme _fs)
{
    return liquid_error_config("fec_conv_create(), libfec not installed");
}

int fec_conv_destroy(fec _q)
{
    return liquid_error(LIQUID_EUMODE,"fec_conv_destroy(), libfec not installed");
}

#endif  // LIBFEC_ENABLED

