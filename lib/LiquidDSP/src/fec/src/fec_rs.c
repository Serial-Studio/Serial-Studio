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
// Reed-Solomon (macros)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

#define VERBOSE_FEC_RS    0

#if LIBFEC_ENABLED
#include "fec.h"

fec fec_rs_create(fec_scheme _fs)
{
    fec q = (fec) malloc(sizeof(struct fec_s));

    q->scheme = _fs;
    q->rate = fec_get_rate(q->scheme);

    q->encode_func      = &fec_rs_encode;
    q->decode_func      = &fec_rs_decode;
    q->decode_soft_func = NULL;

    switch (q->scheme) {
    case LIQUID_FEC_RS_M8: fec_rs_init_p8(q); break;
    default: return liquid_error_config("fec_rs_create(), invalid type");
    }

    // initialize basic parameters
    q->nn = (1 << q->symsize) - 1;
    q->kk = q->nn - q->nroots;

    // lengths
    q->num_dec_bytes = 0;
    q->rs = NULL;

    // allocate memory for arrays
    q->tblock   = (unsigned char*) malloc(q->nn*sizeof(unsigned char));
    q->errlocs  = (int *) malloc(q->nn*sizeof(int));
    q->derrlocs = (int *) malloc(q->nn*sizeof(int));

    return q;
}

int fec_rs_destroy(fec _q)
{
    // delete internal Reed-Solomon decoder object
    if (_q->rs != NULL) {
        free_rs_char(_q->rs);
    }

    // delete internal memory arrays
    free(_q->tblock);
    free(_q->errlocs);
    free(_q->derrlocs);

    // delete fec object
    free(_q);
    return LIQUID_OK;
}

int fec_rs_encode(fec             _q,
                  unsigned int    _dec_msg_len,
                  unsigned char * _msg_dec,
                  unsigned char * _msg_enc)
{
    // validate input
    if (_dec_msg_len == 0)
        return liquid_error(LIQUID_EICONFIG,"fec_rs_encode(), input length must be > 0");

    // re-allocate resources if necessary
    fec_rs_setlength(_q, _dec_msg_len);

    unsigned int i;
    unsigned int n0=0;  // input index
    unsigned int n1=0;  // output index
    unsigned int block_size = _q->dec_block_len;
    for (i=0; i<_q->num_blocks; i++) {

        // the last block is smaller by the residual block length
        if (i == _q->num_blocks-1)
            block_size -= _q->res_block_len;

        // copy sequence
        memmove(_q->tblock, &_msg_dec[n0], block_size*sizeof(unsigned char));

        // last block: we could pad end with zeros, but it's not really
        // necessary as these bits are going to be thrown away anyway

        // encode data, appending parity bits to end of sequence
        encode_rs_char(_q->rs, _q->tblock, &_q->tblock[_q->dec_block_len]);

        // copy result to output
        memmove(&_msg_enc[n1], _q->tblock, _q->enc_block_len*sizeof(unsigned char));

        // increment counters
        n0 += block_size;
        n1 += _q->enc_block_len;
    }

    // sanity check
    assert( n0 == _q->num_dec_bytes );
    assert( n1 == _q->num_enc_bytes );
    return LIQUID_OK;
}

//unsigned int
int fec_rs_decode(fec             _q,
                  unsigned int    _dec_msg_len,
                  unsigned char * _msg_enc,
                  unsigned char * _msg_dec)
{
    // validate input
    if (_dec_msg_len == 0)
        return liquid_error(LIQUID_EICONFIG,"fec_rs_encode(), input length must be > 0");

    // re-allocate resources if necessary
    fec_rs_setlength(_q, _dec_msg_len);

    // set erasures, error locations to zero
    memset(_q->errlocs,  0x00, _q->nn*sizeof(unsigned char));
    memset(_q->derrlocs, 0x00, _q->nn*sizeof(unsigned char));
    _q->erasures = 0;

    unsigned int i;
    unsigned int n0=0;
    unsigned int n1=0;
    unsigned int block_size = _q->dec_block_len;
    //int derrors; // number of decoder errors
    for (i=0; i<_q->num_blocks; i++) {

        // the last block is smaller by the residual block length
        if (i == _q->num_blocks-1)
            block_size -= _q->res_block_len;

        // copy sequence
        memmove(_q->tblock, &_msg_enc[n0], _q->enc_block_len*sizeof(unsigned char));

        // decode block
        //derrors = 
        decode_rs_char(_q->rs,
                       _q->tblock,
                       _q->derrlocs,
                       _q->erasures);

        // copy result
        memmove(&_msg_dec[n1], _q->tblock, block_size*sizeof(unsigned char));

        // increment counters
        n0 += _q->enc_block_len;
        n1 += block_size;
    }

    // sanity check
    assert( n0 == _q->num_enc_bytes );
    assert( n1 == _q->num_dec_bytes );
    return LIQUID_OK;
}

// Set dec_msg_len, re-allocating resources as necessary.  Effectively, it
// divides the input message into several blocks and allows the decoder to
// pad each block appropriately.
//
// For example : if we are using the 8-bit code,
//      nroots  = 32
//      nn      = 255
//      kk      = 223
// Let _dec_msg_len = 1024, then
//      num_blocks = ceil(1024/223)
//                 = ceil(4.5919)
//                 = 5
//      dec_block_len = ceil(1024/num_blocks)
//                    = ceil(204.8)
//                    = 205
//      enc_block_len = dec_block_len + nroots
//                    = 237
//      res_block_len = mod(num_blocks*dec_block_len,_dec_msg_len)
//                    = mod(5*205,1024)
//                    = mod(1025,1024)
//                    = 1 (cannot evenly divide input sequence)
//      pad = kk - dec_block_len
//          = 223 - 205
//          = 18
//
// Thus, the 1024-byte input message is broken into 5 blocks, the first
// four have a length 205, and the last block has a length 204 (which is
// externally padded to 205, e.g. res_block_len = 1). This code adds 32
// parity symbols, so each block is extended to 237 bytes. libfec auto-
// matically extends the internal data to 255 bytes by padding with 18
// symbols.  Therefore, the final output length is 237 * 5 = 1185 symbols.
int fec_rs_setlength(fec _q, unsigned int _dec_msg_len)
{
    // return if length has not changed
    if (_dec_msg_len == _q->num_dec_bytes)
        return LIQUID_OK;

    // reset lengths
    _q->num_dec_bytes = _dec_msg_len;

    div_t d;

    // compute the total number of blocks necessary: ceil(num_dec_bytes / kk)
    d = div(_q->num_dec_bytes, _q->kk);
    _q->num_blocks = d.quot + (d.rem==0 ? 0 : 1);

    // compute the decoded block length: ceil(num_dec_bytes / num_blocks)
    d = div(_dec_msg_len, _q->num_blocks);
    _q->dec_block_len = d.quot + (d.rem == 0 ? 0 : 1);

    // compute the encoded block length: dec_block_len + nroots
    _q->enc_block_len = _q->dec_block_len + _q->nroots;

    // compute the residual padding symbols in the last block:
    // mod(num_blocks*dec_block_len, num_dec_bytes)
    _q->res_block_len = (_q->num_blocks*_q->dec_block_len) % _q->num_dec_bytes;

    // compute the internal libfec padding factor: kk - dec_block_len
    _q->pad = _q->kk - _q->dec_block_len;

    // compute the final encoded block length: enc_block_len * num_blocks
    _q->num_enc_bytes = _q->enc_block_len * _q->num_blocks;
    
#if VERBOSE_FEC_RS
    printf("dec_msg_len     :   %u\n", _q->num_dec_bytes);
    printf("num_blocks      :   %u\n", _q->num_blocks);
    printf("dec_block_len   :   %u\n", _q->dec_block_len);
    printf("enc_block_len   :   %u\n", _q->enc_block_len);
    printf("res_block_len   :   %u\n", _q->res_block_len);
    printf("pad             :   %u\n", _q->pad);
    printf("enc_msg_len     :   %u\n", _q->num_enc_bytes);
#endif

    // delete old decoder if necessary
    if (_q->rs != NULL)
        free_rs_char(_q->rs);

    // Reed-Solomon specific decoding
    _q->rs = init_rs_char(_q->symsize,
                          _q->genpoly,
                          _q->fcs,
                          _q->prim,
                          _q->nroots,
                          _q->pad);
    return LIQUID_OK;
}

// 
// internal
//

int fec_rs_init_p8(fec _q)
{
    _q->symsize = 8;
    _q->genpoly = 0x11d;
    _q->fcs = 1;
    _q->prim = 1;
    _q->nroots = 32;
    return LIQUID_OK;
}

#else   // LIBFEC_ENABLED

fec fec_rs_create(fec_scheme _fs)
{
    return liquid_error_config("fec_rs_create(), libfec not installed");
}

int fec_rs_destroy(fec _q)
{
    return liquid_error(LIQUID_EUMODE,"fec_rs_destroy(), libfec not installed");
}

int fec_rs_encode(fec _q,
                   unsigned int _dec_msg_len,
                   unsigned char *_msg_dec,
                   unsigned char *_msg_enc)
{
    return liquid_error(LIQUID_EUMODE,"fec_rs_encode(), libfec not installed");
}

//unsigned int
int fec_rs_decode(fec _q,
                   unsigned int _dec_msg_len,
                   unsigned char *_msg_enc,
                   unsigned char *_msg_dec)
{
    return liquid_error(LIQUID_EUMODE,"fec_rs_decode(), libfec not installed");
}

#endif  // LIBFEC_ENABLED

