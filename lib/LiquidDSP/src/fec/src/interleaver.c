/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
// interleaver_create.c
//
// Create and initialize interleaver objects
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// 
// internal methods
//

// permute one iteration
int interleaver_permute(unsigned char * _x,
                        unsigned int    _n,
                        unsigned int    _M,
                        unsigned int    _N);

// permute one iteration (soft bit input)
int interleaver_permute_soft(unsigned char * _x,
                             unsigned int    _n,
                             unsigned int    _M,
                             unsigned int    _N);

// permute one iteration with mask
int interleaver_permute_mask(unsigned char * _x,
                             unsigned int    _n,
                             unsigned int    _M,
                             unsigned int    _N,
                             unsigned char   _mask);

// permute one iteration (soft bit input) with mask
int interleaver_permute_mask_soft(unsigned char * _x,
                                  unsigned int    _n,
                                  unsigned int    _M,
                                  unsigned int    _N,
                                  unsigned char   _mask);

// structured interleaver object
struct interleaver_s {
    unsigned int n;     // number of bytes

    unsigned int M;     // row dimension
    unsigned int N;     // col dimension

    // interleaving depth (number of permutations)
    unsigned int depth;
};

// create interleaver of length _n input/output bytes
interleaver interleaver_create(unsigned int _n)
{
    interleaver q = (interleaver) malloc(sizeof(struct interleaver_s));
    q->n = _n;

    // set internal properties
    q->depth = 4;   // default depth to maximum 

    // compute block dimensions
    q->M = 1 + (unsigned int) floorf(sqrtf(q->n));

    q->N = q->n / q->M;
    while (q->n >= (q->M*q->N)) q->N++;  // ensures M*N >= n

    return q;
}

// copy interleaver object
interleaver interleaver_copy(interleaver q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("interleaver_copy(), object cannot be NULL");

    return interleaver_create(q_orig->n);
}

// destroy interleaver object
int interleaver_destroy(interleaver _q)
{
    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print interleaver internals
int interleaver_print(interleaver _q)
{
    printf("interleaver [block, %u bytes] :\n", _q->n);
    printf("    M       :   %u\n", _q->M);
    printf("    N       :   %u\n", _q->N);
    printf("    depth   :   %u\n", _q->depth);
    return LIQUID_OK;
}

// set depth (number of internal iterations)
int interleaver_set_depth(interleaver  _q,
                          unsigned int _depth)
{
    _q->depth = _depth;
    return LIQUID_OK;
}

// execute forward interleaver (encoder)
//  _q          :   interleaver object
//  _msg_dec    :   decoded (un-interleaved) message
//  _msg_enc    :   encoded (interleaved) message
int interleaver_encode(interleaver     _q,
                       unsigned char * _msg_dec,
                       unsigned char * _msg_enc)
{
    // copy data to output
    memmove(_msg_enc, _msg_dec, _q->n);

    if (_q->depth > 0) interleaver_permute(_msg_enc, _q->n, _q->M, _q->N);
    if (_q->depth > 1) interleaver_permute_mask(_msg_enc, _q->n, _q->M, _q->N+2, 0x0f);
    if (_q->depth > 2) interleaver_permute_mask(_msg_enc, _q->n, _q->M, _q->N+4, 0x55);
    if (_q->depth > 3) interleaver_permute_mask(_msg_enc, _q->n, _q->M, _q->N+8, 0x33);

    return LIQUID_OK;
}

// execute forward interleaver (encoder) on soft bits
//  _q          :   interleaver object
//  _msg_dec    :   decoded (un-interleaved) message
//  _msg_enc    :   encoded (interleaved) message
int interleaver_encode_soft(interleaver     _q,
                            unsigned char * _msg_dec,
                            unsigned char * _msg_enc)
{
    // copy data to output
    memmove(_msg_enc, _msg_dec, 8*_q->n);

    if (_q->depth > 0) interleaver_permute_soft(_msg_enc, _q->n, _q->M, _q->N);
    if (_q->depth > 1) interleaver_permute_mask_soft(_msg_enc, _q->n, _q->M, _q->N+2, 0x0f);
    if (_q->depth > 2) interleaver_permute_mask_soft(_msg_enc, _q->n, _q->M, _q->N+4, 0x55);
    if (_q->depth > 3) interleaver_permute_mask_soft(_msg_enc, _q->n, _q->M, _q->N+8, 0x33);

    return LIQUID_OK;
}

// execute reverse interleaver (decoder)
//  _q          :   interleaver object
//  _msg_enc    :   encoded (interleaved) message
//  _msg_dec    :   decoded (un-interleaved) message
int interleaver_decode(interleaver     _q,
                       unsigned char * _msg_enc,
                       unsigned char * _msg_dec)
{
    // copy data to output
    memmove(_msg_dec, _msg_enc, _q->n);

    if (_q->depth > 3) interleaver_permute_mask(_msg_dec, _q->n, _q->M, _q->N+8, 0x33);
    if (_q->depth > 2) interleaver_permute_mask(_msg_dec, _q->n, _q->M, _q->N+4, 0x55);
    if (_q->depth > 1) interleaver_permute_mask(_msg_dec, _q->n, _q->M, _q->N+2, 0x0f);
    if (_q->depth > 0) interleaver_permute(_msg_dec, _q->n, _q->M, _q->N);

    return LIQUID_OK;
}

// execute reverse interleaver (decoder) on soft bits
//  _q          :   interleaver object
//  _msg_enc    :   encoded (interleaved) message
//  _msg_dec    :   decoded (un-interleaved) message
int interleaver_decode_soft(interleaver     _q,
                            unsigned char * _msg_enc,
                            unsigned char * _msg_dec)
{
    // copy data to output
    memmove(_msg_dec, _msg_enc, 8*_q->n);

    if (_q->depth > 3) interleaver_permute_mask_soft(_msg_dec, _q->n, _q->M, _q->N+8, 0x33);
    if (_q->depth > 2) interleaver_permute_mask_soft(_msg_dec, _q->n, _q->M, _q->N+4, 0x55);
    if (_q->depth > 1) interleaver_permute_mask_soft(_msg_dec, _q->n, _q->M, _q->N+2, 0x0f);
    if (_q->depth > 0) interleaver_permute_soft(_msg_dec, _q->n, _q->M, _q->N);

    return LIQUID_OK;
}

// 
// internal permutation methods
//

// permute one iteration
int interleaver_permute(unsigned char * _x,
                        unsigned int    _n,
                        unsigned int    _M,
                        unsigned int    _N)
{
    unsigned int i;
    unsigned int j;
    unsigned int m=0;
    unsigned int n=_n/3;
    unsigned int n2=_n/2;
    unsigned char tmp;
    for (i=0; i<n2; i++) {
        //j = m*N + n; // input
        do {
            j = m*_N + n; // output
            m++;
            if (m == _M) {
                n = (n+1) % (_N);
                m=0;
            }
        } while (j>=n2);

        // swap indices
        tmp = _x[2*j+1];
        _x[2*j+1] = _x[2*i+0];
        _x[2*i+0] = tmp;
    }

    return LIQUID_OK;
}

// permute one iteration (soft bit input)
int interleaver_permute_soft(unsigned char * _x,
                             unsigned int    _n,
                             unsigned int    _M,
                             unsigned int    _N)
{
    unsigned int i;
    unsigned int j;
    unsigned int m=0;
    unsigned int n=_n/3;
    unsigned int n2=_n/2;
    unsigned char tmp[8];
    for (i=0; i<n2; i++) {
        //j = m*N + n; // input
        do {
            j = m*_N + n; // output
            m++;
            if (m == _M) {
                n = (n+1) % (_N);
                m=0;
            }
        } while (j>=n2);
    
        // swap soft bits at indices
        memmove( tmp,            &_x[8*(2*j+1)], 8);
        memmove( &_x[8*(2*j+1)], &_x[8*(2*i+0)], 8);
        memmove( &_x[8*(2*i+0)], tmp,            8);
    }
    //printf("\n");

    return LIQUID_OK;
}


// permute one iteration with mask
int interleaver_permute_mask(unsigned char * _x,
                             unsigned int    _n,
                             unsigned int    _M,
                             unsigned int    _N,
                             unsigned char   _mask)
{
    unsigned int i;
    unsigned int j;
    unsigned int m=0;
    unsigned int n=_n/3;
    unsigned int n2=_n/2;
    unsigned char tmp0;
    unsigned char tmp1;
    for (i=0; i<n2; i++) {
        //j = m*N + n; // input
        do {
            j = m*_N + n; // output
            m++;
            if (m == _M) {
                n = (n+1) % (_N);
                m=0;
            }
        } while (j>=n2);

        // swap indices, applying mask
        tmp0 = (_x[2*i+0] & (~_mask)) | (_x[2*j+1] & ( _mask));
        tmp1 = (_x[2*i+0] & ( _mask)) | (_x[2*j+1] & (~_mask));
        _x[2*i+0] = tmp0;
        _x[2*j+1] = tmp1;
    }

    return LIQUID_OK;
}

// permute one iteration (soft bit input)
int interleaver_permute_mask_soft(unsigned char * _x,
                                  unsigned int    _n,
                                  unsigned int    _M,
                                  unsigned int    _N,
                                  unsigned char   _mask)
{
    unsigned int i;
    unsigned int j;
    unsigned int k;
    unsigned int m=0;
    unsigned int n=_n/3;
    unsigned int n2=_n/2;
    unsigned char tmp;
    for (i=0; i<n2; i++) {
        //j = m*N + n; // input
        do {
            j = m*_N + n; // output
            m++;
            if (m == _M) {
                n = (n+1) % (_N);
                m=0;
            }
        } while (j>=n2);

        // swap bits matching the mask
        for (k=0; k<8; k++) {
            if ( (_mask >> (8-k-1)) & 0x01 ) {
                tmp = _x[8*(2*j+1)+k];
                _x[8*(2*j+1)+k] = _x[8*(2*i+0)+k];
                _x[8*(2*i+0)+k] = tmp;
            }
        }
    }
    //printf("\n");

    return LIQUID_OK;
}

