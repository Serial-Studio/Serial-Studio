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
// FEC, none/pass
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "liquid.internal.h"

fec fec_pass_create(void * _opts)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    q->scheme = LIQUID_FEC_NONE;
    q->rate = fec_get_rate(q->scheme);

    q->encode_func      = &fec_pass_encode;
    q->decode_func      = &fec_pass_decode;
    q->decode_soft_func = NULL;

    return q;
}

int fec_pass_destroy(fec _q)
{
    free(_q);
    return LIQUID_OK;
}

int fec_pass_print(fec _q)
{
    printf("fec_pass [r: %3.2f]\n", _q->rate);
    return LIQUID_OK;
}

int fec_pass_encode(fec _q, unsigned int _dec_msg_len, unsigned char *_msg_dec, unsigned char *_msg_enc)
{
    memmove(_msg_enc, _msg_dec, _dec_msg_len);
    return LIQUID_OK;
}

int fec_pass_decode(fec _q, unsigned int _dec_msg_len, unsigned char *_msg_enc, unsigned char *_msg_dec)
{
    memmove(_msg_dec, _msg_enc, _dec_msg_len);
    return LIQUID_OK;
}

