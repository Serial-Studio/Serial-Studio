/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// FEC (generic functions)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

// object-independent methods

const char * fec_scheme_str[LIQUID_FEC_NUM_SCHEMES][2] = {
    // short name,  long name
    {"unknown",     "unknown"},
    {"none",        "none"},
    {"rep3",        "repeat(3)"},
    {"rep5",        "repeat(5)"},
    {"h74",         "Hamming(7,4)"},
    {"h84",         "Hamming(8,4)"},
    {"h128",        "Hamming(12,8)"},
    {"g2412",       "Golay(24,12)"},
    {"secded2216",  "SEC-DEC(22,16)"},
    {"secded3932",  "SEC-DEC(39,32)"},
    {"secded7264",  "SEC-DEC(72,64)"},
    {"v27",         "convolutional r1/2 K=7"},
    {"v29",         "convolutional r1/2 K=9"},
    {"v39",         "convolutional r1/3 K=9"},
    {"v615",        "convolutional r1/6 K=15"},
    {"v27p23",      "convolutional r2/3 K=7 (punctured)"},
    {"v27p34",      "convolutional r3/4 K=7 (punctured)"},
    {"v27p45",      "convolutional r4/5 K=7 (punctured)"},
    {"v27p56",      "convolutional r5/6 K=7 (punctured)"},
    {"v27p67",      "convolutional r6/7 K=7 (punctured)"},
    {"v27p78",      "convolutional r7/8 K=7 (punctured)"},
    {"v29p23",      "convolutional r2/3 K=9 (punctured)"},
    {"v29p34",      "convolutional r3/4 K=9 (punctured)"},
    {"v29p45",      "convolutional r4/5 K=9 (punctured)"},
    {"v29p56",      "convolutional r5/6 K=9 (punctured)"},
    {"v29p67",      "convolutional r6/7 K=9 (punctured)"},
    {"v29p78",      "convolutional r7/8 K=9 (punctured)"},
    {"rs8",         "Reed-Solomon, 223/255"}
};

// Print compact list of existing and available fec schemes
int liquid_print_fec_schemes()
{
    unsigned int i;
    unsigned int len = 10;

    // print all available MOD schemes
    printf("          ");
    for (i=0; i<LIQUID_FEC_NUM_SCHEMES; i++) {
#if !LIBFEC_ENABLED
        if ( fec_scheme_is_convolutional(i) || fec_scheme_is_reedsolomon(i) )
            continue;
#endif
        printf("%s", fec_scheme_str[i][0]);

        if (i != LIQUID_FEC_NUM_SCHEMES-1)
            printf(", ");

        len += strlen(fec_scheme_str[i][0]);
        if (len > 48 && i != LIQUID_FEC_NUM_SCHEMES-1) {
            len = 10;
            printf("\n          ");
        }
    }
    printf("\n");
    return LIQUID_OK;
}


fec_scheme liquid_getopt_str2fec(const char * _str)
{
    // compare each string to short name
    unsigned int i;
    for (i=0; i<LIQUID_FEC_NUM_SCHEMES; i++) {
        if (strcmp(_str,fec_scheme_str[i][0])==0) {
            return i;
        }
    }

    liquid_error(LIQUID_EICONFIG,"liquid_getopt_str2fec(), unknown/unsupported crc scheme: %s", _str);
    return LIQUID_FEC_UNKNOWN;
}

// is scheme convolutional?
int fec_scheme_is_convolutional(fec_scheme _scheme)
{
    switch (_scheme) {
    // convolutional codes (punctured or otherwise)
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:
    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        return 1;

    default:;
    }

    return 0;
}

// is scheme punctured?
int fec_scheme_is_punctured(fec_scheme _scheme)
{
    switch (_scheme) {
    // convolutional codes (punctured)
    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        return 1;

    default:;
    }

    return 0;
}

// is scheme Reed-Solomon?
int fec_scheme_is_reedsolomon(fec_scheme _scheme)
{
    switch (_scheme) {
    // Reed-Solomon codes
    case LIQUID_FEC_RS_M8:
        return 1;
    default:;
    }
    return 0;
}

// is scheme Hamming?
int fec_scheme_is_hamming(fec_scheme _scheme)
{
    switch (_scheme) {
    case LIQUID_FEC_HAMMING74:
    case LIQUID_FEC_HAMMING84:
    case LIQUID_FEC_HAMMING128:
        return 1;
    default:;
    }
    return 0;
}

// is scheme repeat?
int fec_scheme_is_repeat(fec_scheme _scheme)
{
    switch (_scheme) {
    case LIQUID_FEC_REP3:
    case LIQUID_FEC_REP5:
        return 1;
    default:;
    }
    return 0;
}


// return the encoded message length using a particular error-
// correction scheme (object-independent method)
//  _scheme     :   forward error-correction scheme
//  _msg_len    :   raw, uncoded message length
unsigned int fec_get_enc_msg_length(fec_scheme   _scheme,
                                    unsigned int _msg_len)
{
    switch (_scheme) {
    case LIQUID_FEC_UNKNOWN:        return 0;
    case LIQUID_FEC_NONE:           return _msg_len;
    case LIQUID_FEC_REP3:           return 3*_msg_len;
    case LIQUID_FEC_REP5:           return 5*_msg_len;
    case LIQUID_FEC_HAMMING74:      return fec_block_get_enc_msg_len(_msg_len,4,7);
    case LIQUID_FEC_HAMMING84:      return fec_block_get_enc_msg_len(_msg_len,4,8);
    case LIQUID_FEC_HAMMING128:     return fec_block_get_enc_msg_len(_msg_len,8,12);
    case LIQUID_FEC_GOLAY2412:      return fec_block_get_enc_msg_len(_msg_len,12,24);
    case LIQUID_FEC_SECDED2216:     return _msg_len + _msg_len/2 + ((_msg_len%2) ? 1 : 0);
    case LIQUID_FEC_SECDED3932:     return _msg_len + _msg_len/4 + ((_msg_len%4) ? 1 : 0);
    case LIQUID_FEC_SECDED7264:     return _msg_len + _msg_len/8 + ((_msg_len%8) ? 1 : 0);

#if LIBFEC_ENABLED
    // convolutional codes
    case LIQUID_FEC_CONV_V27:       return 2*_msg_len + 2;  // (K-1)/r=12, round up to 2 bytes
    case LIQUID_FEC_CONV_V29:       return 2*_msg_len + 2;  // (K-1)/r=16, 2 bytes
    case LIQUID_FEC_CONV_V39:       return 3*_msg_len + 3;  // (K-1)/r=24, 3 bytes
    case LIQUID_FEC_CONV_V615:      return 6*_msg_len + 11; // (K-1)/r=84, round up to 11 bytes
    case LIQUID_FEC_CONV_V27P23:    return fec_conv_get_enc_msg_len(_msg_len,7,2);
    case LIQUID_FEC_CONV_V27P34:    return fec_conv_get_enc_msg_len(_msg_len,7,3);
    case LIQUID_FEC_CONV_V27P45:    return fec_conv_get_enc_msg_len(_msg_len,7,4);
    case LIQUID_FEC_CONV_V27P56:    return fec_conv_get_enc_msg_len(_msg_len,7,5);
    case LIQUID_FEC_CONV_V27P67:    return fec_conv_get_enc_msg_len(_msg_len,7,6);
    case LIQUID_FEC_CONV_V27P78:    return fec_conv_get_enc_msg_len(_msg_len,7,7);

    case LIQUID_FEC_CONV_V29P23:    return fec_conv_get_enc_msg_len(_msg_len,9,2);
    case LIQUID_FEC_CONV_V29P34:    return fec_conv_get_enc_msg_len(_msg_len,9,3);
    case LIQUID_FEC_CONV_V29P45:    return fec_conv_get_enc_msg_len(_msg_len,9,4);
    case LIQUID_FEC_CONV_V29P56:    return fec_conv_get_enc_msg_len(_msg_len,9,5);
    case LIQUID_FEC_CONV_V29P67:    return fec_conv_get_enc_msg_len(_msg_len,9,6);
    case LIQUID_FEC_CONV_V29P78:    return fec_conv_get_enc_msg_len(_msg_len,9,7);

    // Reed-Solomon codes
    case LIQUID_FEC_RS_M8:          return fec_rs_get_enc_msg_len(_msg_len,32,255,223);
#else
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:

    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        liquid_error(LIQUID_EUMODE,"fec_get_enc_msg_length(), convolutional codes unavailable (install libfec)");
        break;
    case LIQUID_FEC_RS_M8:
        liquid_error(LIQUID_EUMODE,"fec_get_enc_msg_length(), Reed-Solomon codes unavailable (install libfec)");
        break;
#endif
    default:
        liquid_error(LIQUID_EIMODE,"fec_get_enc_msg_length(), unknown/unsupported scheme: %d\n", _scheme);
        break;
    }
    return 0;
}

// compute encoded message length for block codes
//  _dec_msg_len    :   decoded message length (bytes)
//  _m              :   input block size (bits)
//  _k              :   output block size (bits)
unsigned int fec_block_get_enc_msg_len(unsigned int _dec_msg_len,
                                       unsigned int _m,
                                       unsigned int _k)
{
    // validate input
    if (_m == 0) {
        fprintf(stderr,"fec_block_get_enc_msg_len(), input block size cannot be zero\n");
        return 0;
    } else if (_k < _m) {
        fprintf(stderr,"fec_block_get_enc_msg_len(), output block size cannot be smaller than input\n");
        return 0;
    }

    // compute total number of bits in decoded message
    unsigned int num_bits_in = _dec_msg_len*8;

    // compute total number of blocks: ceil(num_bits_in/_m)
    unsigned int num_blocks = num_bits_in / _m + (num_bits_in%_m ? 1 : 0);

    // compute total number of bits out
    unsigned int num_bits_out = num_blocks * _k;

    // compute total number of bytes out: ceil(num_bits_out/8)
    unsigned int num_bytes_out = num_bits_out/8 + (num_bits_out%8 ? 1 : 0);
#if 0
    printf("fec_block_get_enc_msg_len(%u,%u,%u)\n", _dec_msg_len, _m, _k);
    printf("    dec msg len :   %u bytes\n", _dec_msg_len);
    printf("    m           :   %u bits\n", _m);
    printf("    k           :   %u bits\n", _k);
    printf("    num bits in :   %u bits\n", num_bits_in);
    printf("    num blocks  :   %u\n", num_blocks);
    printf("    num bits out:   %u bits\n", num_bits_out);
    printf("    enc msg len :   %u bytes\n", num_bytes_out);
#endif
    return num_bytes_out;
}

// compute encoded message length for convolutional codes
//  _dec_msg_len    :   decoded message length
//  _K              :   constraint length
//  _p              :   puncturing rate, r = _p / (_p+1)
unsigned int fec_conv_get_enc_msg_len(unsigned int _dec_msg_len,
                                      unsigned int _K,
                                      unsigned int _p)
{
    unsigned int num_bits_in = _dec_msg_len*8;
    unsigned int n = num_bits_in + _K - 1;
    unsigned int num_bits_out = n + (n+_p-1)/_p;
    unsigned int num_bytes_out = num_bits_out/8 + (num_bits_out%8 ? 1 : 0);
#if 0
    printf("msg len :       %3u\n", _dec_msg_len);
    printf("num bits in :   %3u\n", num_bits_in);
    printf("n (constraint): %3u\n", n);
    printf("num bits out:   %3u", num_bits_out);
    printf(" = n+(n+p-1)/p = %u+(%u+%u-1)/%u\n", n,n,_p,_p);
    printf("num bytes out:  %3u\n", num_bytes_out);
#endif
    return num_bytes_out;
}

// compute encoded message length for Reed-Solomon codes
//  _dec_msg_len    :   decoded message length
//  _nroots         :   number of roots in polynomial
//  _nn             :   
//  _kk             :   
// Example : if we are using the 8-bit code,
//      _nroots  = 32
//      _nn      = 255
//      _kk      = 223
// Let _dec_msg_len = 1024, then
//      num_blocks = ceil(1024/223)
//                 = ceil(4.5919)
//                 = 5
//      dec_block_len = ceil(1024/num_blocks)
//                    = ceil(204.8)
//                    = 205
//      enc_block_len = dec_block_len + nroots
//                    = 237
//      enc_msg_len = num_blocks * enc_block_len
//                  = 1185
unsigned int fec_rs_get_enc_msg_len(unsigned int _dec_msg_len,
                                    unsigned int _nroots,
                                    unsigned int _nn,
                                    unsigned int _kk)
{
    // validate input
    if (_dec_msg_len == 0) {
        liquid_error(LIQUID_EICONFIG,"fec_rs_get_enc_msg_len(), _dec_msg_len must be greater than 0");
        return 0;
    }

    div_t d;

    // compute the number of blocks in the full message sequence
    d = div(_dec_msg_len, _kk);
    unsigned int num_blocks = d.quot + (d.rem==0 ? 0 : 1);

    // compute the length of each decoded block
    d = div(_dec_msg_len, num_blocks);
    unsigned int dec_block_len = d.quot + (d.rem == 0 ? 0 : 1);

    // compute the encoded block length
    unsigned int enc_block_len = dec_block_len + _nroots;

    // compute the number of bytes in the full encoded message
    unsigned int enc_msg_len = enc_block_len * num_blocks;
#if 0
    printf("dec_msg_len     :   %u\n", _dec_msg_len);
    printf("num_blocks      :   %u\n",  num_blocks);
    printf("dec_block_len   :   %u\n",  dec_block_len);
    printf("enc_block_len   :   %u\n",  enc_block_len);
    printf("enc_msg_len     :   %u\n",  enc_msg_len);
#endif
    return enc_msg_len;
}


// get the theoretical rate of a particular forward error-
// correction scheme (object-independent method)
float fec_get_rate(fec_scheme _scheme)
{
    switch (_scheme) {
    case LIQUID_FEC_UNKNOWN:        return 0;
    case LIQUID_FEC_NONE:           return 1.;
    case LIQUID_FEC_REP3:           return 1./3.;
    case LIQUID_FEC_REP5:           return 1./5.;
    case LIQUID_FEC_HAMMING74:      return 4./7.;
    case LIQUID_FEC_HAMMING84:      return 4./8.;
    case LIQUID_FEC_HAMMING128:     return 8./12.;
    case LIQUID_FEC_GOLAY2412:      return 1./2.;
    case LIQUID_FEC_SECDED2216:     return 2./3.;   // ultimately 16/22 ~ 0.72727
    case LIQUID_FEC_SECDED3932:     return 4./5.;   // ultimately 32/39 ~ 0.82051
    case LIQUID_FEC_SECDED7264:     return 8./9.;

    // convolutional codes
#if LIBFEC_ENABLED
    case LIQUID_FEC_CONV_V27:       return 1./2.;
    case LIQUID_FEC_CONV_V29:       return 1./2.;
    case LIQUID_FEC_CONV_V39:       return 1./3.;
    case LIQUID_FEC_CONV_V615:      return 1./6.;
    case LIQUID_FEC_CONV_V27P23:    return 2./3.;
    case LIQUID_FEC_CONV_V27P34:    return 3./4.;
    case LIQUID_FEC_CONV_V27P45:    return 4./5.;
    case LIQUID_FEC_CONV_V27P56:    return 5./6.;
    case LIQUID_FEC_CONV_V27P67:    return 6./7.;
    case LIQUID_FEC_CONV_V27P78:    return 7./8.;
    case LIQUID_FEC_CONV_V29P23:    return 2./3.;
    case LIQUID_FEC_CONV_V29P34:    return 3./4.;
    case LIQUID_FEC_CONV_V29P45:    return 4./5.;
    case LIQUID_FEC_CONV_V29P56:    return 5./6.;
    case LIQUID_FEC_CONV_V29P67:    return 6./7.;
    case LIQUID_FEC_CONV_V29P78:    return 7./8.;

    // Reed-Solomon codes
    case LIQUID_FEC_RS_M8:          return 223./255.;
#else
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:

    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        liquid_error(LIQUID_EUMODE,"fec_get_rate(), convolutional codes unavailable (install libfec)");
        return 0.0f;
    case LIQUID_FEC_RS_M8:
        liquid_error(LIQUID_EUMODE,"fec_get_rate(), Reed-Solomon codes unavailable (install libfec)");
        return 0.0f;
#endif

    default:
        liquid_error(LIQUID_EIMODE,"fec_get_rate(), unknown/unsupported scheme: %d", _scheme);
        return 0.0f;
    }
    return liquid_error(LIQUID_EINT,"internal error");
    return 0.0f;
}

// create a fec object of a particular scheme
//  _scheme     :   error-correction scheme
//  _opts       :   (ignored)
fec fec_create(fec_scheme _scheme, void *_opts)
{
    switch (_scheme) {
    case LIQUID_FEC_UNKNOWN:
        return liquid_error_config("fec_create(), cannot create fec object of unknown type\n");
    case LIQUID_FEC_NONE:       return fec_pass_create(NULL);
    case LIQUID_FEC_REP3:       return fec_rep3_create(_opts);
    case LIQUID_FEC_REP5:       return fec_rep5_create(_opts);
    case LIQUID_FEC_HAMMING74:  return fec_hamming74_create(_opts);
    case LIQUID_FEC_HAMMING84:  return fec_hamming84_create(_opts);
    case LIQUID_FEC_HAMMING128: return fec_hamming128_create(_opts);
    case LIQUID_FEC_GOLAY2412:  return fec_golay2412_create(_opts);

    // SEC-DED codecs (single error correction, double error detection)
    case LIQUID_FEC_SECDED2216: return fec_secded2216_create(_opts);
    case LIQUID_FEC_SECDED3932: return fec_secded3932_create(_opts);
    case LIQUID_FEC_SECDED7264: return fec_secded7264_create(_opts);

    // convolutional codes
#if LIBFEC_ENABLED
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:
        return fec_conv_create(_scheme);

    // punctured
    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        return fec_conv_punctured_create(_scheme);

    // Reed-Solomon codes
    case LIQUID_FEC_RS_M8:
        return fec_rs_create(_scheme);
#else
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:

    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        liquid_error(LIQUID_EUMODE,"fec_create(), convolutional codes unavailable (install libfec)");
        return NULL;
    case LIQUID_FEC_RS_M8:
        liquid_error(LIQUID_EUMODE,"fec_create(), Reed-Solomon codes unavailable (install libfec)");
        return NULL;
#endif
    default:
        liquid_error(LIQUID_EIMODE,"fec_create(), unknown/unsupported scheme: %d", _scheme);
        return NULL;
    }

    // should never get to this point, but return NULL to keep
    // compiler happy
    liquid_error(LIQUID_EINT,"internal error");
    return NULL;
}

// recreate a fec object
//  _q      :   initial fec object
//  _scheme :   new scheme
//  _opts   :   options (ignored)
fec fec_recreate(fec _q,
                 fec_scheme _scheme,
                 void *_opts)
{
    if (_q->scheme != _scheme) {
        // destroy old object and create new one
        fec_destroy(_q);
        _q = fec_create(_scheme,_opts);
    }

    // scheme hasn't changed; just return original object
    return _q;
}

// copy object
fec fec_copy(fec q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("fec_copy(), object cannot be NULL");

    // TODO: ensure state is retained
    return fec_create(q_orig->scheme, NULL);
}

// destroy fec object
int fec_destroy(fec _q)
{
    switch (_q->scheme) {
    case LIQUID_FEC_UNKNOWN:    return liquid_error(LIQUID_EIMODE,"fec_destroy(), cannot destroy fec object of unknown type");
    case LIQUID_FEC_NONE:       return fec_pass_destroy(_q);
    case LIQUID_FEC_REP3:       return fec_rep3_destroy(_q);
    case LIQUID_FEC_REP5:       return fec_rep5_destroy(_q);
    case LIQUID_FEC_HAMMING74:  return fec_hamming74_destroy(_q);
    case LIQUID_FEC_HAMMING84:  return fec_hamming84_destroy(_q);
    case LIQUID_FEC_HAMMING128: return fec_hamming128_destroy(_q);
    case LIQUID_FEC_GOLAY2412:  return fec_golay2412_destroy(_q);

    // SEC-DED codecs (single error correction, double error detection)
    case LIQUID_FEC_SECDED2216: return fec_secded2216_destroy(_q);
    case LIQUID_FEC_SECDED3932: return fec_secded3932_destroy(_q);
    case LIQUID_FEC_SECDED7264: return fec_secded7264_destroy(_q);

    // convolutional codes
#if LIBFEC_ENABLED
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:
        return fec_conv_destroy(_q);

    // punctured
    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:
    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        return fec_conv_punctured_destroy(_q);

    // Reed-Solomon codes
    case LIQUID_FEC_RS_M8:
        return fec_rs_destroy(_q);
#else
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:
    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:
    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        return liquid_error(LIQUID_EUMODE,"fec_destroy(), convolutional codes unavailable (install libfec)");
    case LIQUID_FEC_RS_M8:
        return liquid_error(LIQUID_EUMODE,"fec_destroy(), Reed-Solomon codes unavailable (install libfec)");
#endif
    default:
        return liquid_error(LIQUID_EUMODE,"fec_destroy(), unknown/unsupported scheme: %d\n", _q->scheme);
    }
    return liquid_error(LIQUID_EINT,"internal error");
}

// print basic fec object internals
int fec_print(fec _q)
{
    printf("fec: %s [rate: %4.3f]\n",
        fec_scheme_str[_q->scheme][1],
        _q->rate);
    return LIQUID_OK;
}

// encode a block of data using a fec scheme
//  _q              :   fec object
//  _dec_msg_len    :   decoded message length
//  _msg_dec        :   decoded message
//  _msg_enc        :   encoded message
int fec_encode(fec _q,
               unsigned int _dec_msg_len,
               unsigned char * _msg_dec,
               unsigned char * _msg_enc)
{
    // call internal encoding method
    return _q->encode_func(_q, _dec_msg_len, _msg_dec, _msg_enc);
}

// decode a block of data using a fec scheme
//  _q              :   fec object
//  _dec_msg_len    :   decoded message length
//  _msg_enc        :   encoded message
//  _msg_dec        :   decoded message
int fec_decode(fec _q,
               unsigned int _dec_msg_len,
               unsigned char * _msg_enc,
               unsigned char * _msg_dec)
{
    // call internal decoding method
    return _q->decode_func(_q, _dec_msg_len, _msg_enc, _msg_dec);
}

// decode a block of data using a fec scheme
//  _q              :   fec object
//  _dec_msg_len    :   decoded message length
//  _msg_enc        :   encoded message
//  _msg_dec        :   decoded message
int fec_decode_soft(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_enc,
                    unsigned char * _msg_dec)
{
    if (_q->decode_soft_func != NULL) {
        // call internal decoding method
        return _q->decode_soft_func(_q, _dec_msg_len, _msg_enc, _msg_dec);
    } else {
        // pack bytes and use hard-decision decoding
        unsigned enc_msg_len = fec_get_enc_msg_length(_q->scheme, _dec_msg_len);
        unsigned char msg_enc_hard[enc_msg_len];
        unsigned int i;
        for (i=0; i<enc_msg_len; i++) {
            // TODO : use pack bytes
            msg_enc_hard[i] = 0;
            msg_enc_hard[i] |= (_msg_enc[8*i+0] >> 0) & 0x80;
            msg_enc_hard[i] |= (_msg_enc[8*i+1] >> 1) & 0x40;
            msg_enc_hard[i] |= (_msg_enc[8*i+2] >> 2) & 0x20;
            msg_enc_hard[i] |= (_msg_enc[8*i+3] >> 3) & 0x10;
            msg_enc_hard[i] |= (_msg_enc[8*i+4] >> 4) & 0x08;
            msg_enc_hard[i] |= (_msg_enc[8*i+5] >> 5) & 0x04;
            msg_enc_hard[i] |= (_msg_enc[8*i+6] >> 6) & 0x02;
            msg_enc_hard[i] |= (_msg_enc[8*i+7] >> 7) & 0x01;
        }

        // use hard-decoding method
        return fec_decode(_q, _dec_msg_len, msg_enc_hard, _msg_dec);
    }
}


