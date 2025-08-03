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
// cyclic redundancy check (and family)
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

#define CRC8_POLY 0x07
#define CRC16_POLY 0x8005
#define CRC24_POLY 0x5D6DCB
#define CRC32_POLY 0x04C11DB7

unsigned int checksum_generate_key(unsigned char * _msg, unsigned int _msg_len);
unsigned int crc8_generate_key(unsigned char * _msg, unsigned int _msg_len);
unsigned int crc16_generate_key(unsigned char * _msg, unsigned int _msg_len);
unsigned int crc24_generate_key(unsigned char * _msg, unsigned int _msg_len);
unsigned int crc32_generate_key(unsigned char * _msg, unsigned int _msg_len);

// object-independent methods

const char * crc_scheme_str[LIQUID_CRC_NUM_SCHEMES][2] = {
    // short name,  long name
    {"unknown",     "unknown"},
    {"none",        "none"},
    {"checksum",    "checksum (8-bit)"},
    {"crc8",        "CRC (8-bit)"},
    {"crc16",       "CRC (16-bit)"},
    {"crc24",       "CRC (24-bit)"},
    {"crc32",       "CRC (32-bit)"}
};


// Print compact list of existing and available crc schemes
int liquid_print_crc_schemes()
{
    unsigned int i;
    unsigned int len = 10;

    // print all available CRC schemes
    printf("          ");
    for (i=0; i<LIQUID_CRC_NUM_SCHEMES; i++) {
        printf("%s", crc_scheme_str[i][0]);

        if (i != LIQUID_CRC_NUM_SCHEMES-1)
            printf(", ");

        len += strlen(crc_scheme_str[i][0]);
        if (len > 48 && i != LIQUID_CRC_NUM_SCHEMES-1) {
            len = 10;
            printf("\n          ");
        }
    }
    printf("\n");
    return LIQUID_OK;
}

crc_scheme liquid_getopt_str2crc(const char * _str)
{
    // compare each string to short name
    unsigned int i;
    for (i=0; i<LIQUID_CRC_NUM_SCHEMES; i++) {
        if (strcmp(_str,crc_scheme_str[i][0])==0) {
            return i;
        }
    }

    liquid_error(LIQUID_EICONFIG,"liquid_getopt_str2crc(), unknown/unsupported crc scheme: %s", _str);
    return LIQUID_CRC_UNKNOWN;
}

// get length of CRC (bytes)
unsigned int crc_get_length(crc_scheme _scheme)
{
    switch (_scheme) {
    case LIQUID_CRC_UNKNOWN:   return 0;
    case LIQUID_CRC_NONE:      return 0;
    case LIQUID_CRC_CHECKSUM:  return 1;
    case LIQUID_CRC_8:         return 1;
    case LIQUID_CRC_16:        return 2;
    case LIQUID_CRC_24:        return 3;
    case LIQUID_CRC_32:        return 4;
    default:
        liquid_error(LIQUID_EICONFIG,"crc_get_length(), unknown/unsupported scheme: %d", _scheme);
    }
    return 0;
}

// generate error-detection key
//  _scheme     :   error-detection scheme
//  _msg        :   input data message, [size: _n x 1]
//  _n          :   input data message size
unsigned int crc_generate_key(crc_scheme      _scheme,
                              unsigned char * _msg,
                              unsigned int    _n)
{
    switch (_scheme) {
    case LIQUID_CRC_UNKNOWN:
        liquid_error(LIQUID_EIMODE,"crc_generate_key(), cannot generate key with CRC unknown type");
        return 0;
    case LIQUID_CRC_NONE:      return 0;
    case LIQUID_CRC_CHECKSUM:  return checksum_generate_key(_msg, _n);
    case LIQUID_CRC_8:         return crc8_generate_key(_msg, _n);
    case LIQUID_CRC_16:        return crc16_generate_key(_msg, _n);
    case LIQUID_CRC_24:        return crc24_generate_key(_msg, _n);
    case LIQUID_CRC_32:        return crc32_generate_key(_msg, _n);
    default:
        liquid_error(LIQUID_EICONFIG,"crc_generate_key(), unknown/unsupported scheme: %d", _scheme);
        return 0;
    }
    return 0;
}

// generate error-detection key and append to end of message
//  _scheme     :   error-detection scheme (resulting in 'p' bytes)
//  _msg        :   input data message, [size: _n+p x 1]
//  _n          :   input data message size (excluding key at end)
int crc_append_key(crc_scheme      _scheme,
                   unsigned char * _msg,
                   unsigned int    _n)
{
    // get key size
    unsigned int len = crc_sizeof_key(_scheme);

    // generate key
    unsigned int key = crc_generate_key(_scheme, _msg, _n);

    // append key to end of message
    unsigned int i;
    for (i=0; i<len; i++)
        _msg[_n+i] = (key >> (len - i - 1)*8) & 0xff;
    return LIQUID_OK;
}

// validate message using error-detection key
//  _scheme     :   error-detection scheme
//  _msg        :   input data message, [size: _n x 1]
//  _n          :   input data message size
//  _key        :   error-detection key
int crc_validate_message(crc_scheme      _scheme,
                         unsigned char * _msg,
                         unsigned int    _n,
                         unsigned int    _key)
{
    if (_scheme == LIQUID_CRC_UNKNOWN) {
        liquid_error(LIQUID_EIMODE,"crc_validate_message(), cannot validate with CRC unknown type");
        return 0;
    } else if (_scheme == LIQUID_CRC_NONE) {
        return 1;
    }

    return crc_generate_key(_scheme, _msg, _n) == _key;
}

// check message with key appended to end of array
//  _scheme     :   error-detection scheme (resulting in 'p' bytes)
//  _msg        :   input data message, [size: _n+p x 1]
//  _n          :   input data message size (excluding key at end)
int crc_check_key(crc_scheme      _scheme,
                  unsigned char * _msg,
                  unsigned int    _n)
{
    // get key size
    unsigned int len = crc_sizeof_key(_scheme);

    // extract key from end of message
    unsigned int key = 0;
    unsigned int i;
    for (i=0; i<len; i++) {
        key <<= 8;
        key |= _msg[_n+i];
    }

    // validate message against key
    return crc_validate_message(_scheme, _msg, _n, key);
}

// get size of key (bytes)
unsigned int crc_sizeof_key(crc_scheme _scheme)
{
    switch (_scheme) {
    case LIQUID_CRC_UNKNOWN:
        liquid_error(LIQUID_EICONFIG,"crc_sizeof_key(), cannot get size of type 'LIQUID_CRC_UNKNOWN'");
        return 0;
    case LIQUID_CRC_NONE:      return 0;
    case LIQUID_CRC_CHECKSUM:  return 1;
    case LIQUID_CRC_8:         return 1;
    case LIQUID_CRC_16:        return 2;
    case LIQUID_CRC_24:        return 3;
    case LIQUID_CRC_32:        return 4;
    default:
        liquid_error(LIQUID_EICONFIG,"crc_sizeof_key(), unknown/unsupported scheme: %d", _scheme);
        return 0;
    }
    return 0;
}


//
// Checksum
//

// generate 8-bit checksum key
//
//  _scheme     :   error-detection scheme
//  _msg        :   input data message, [size: _n x 1]
//  _n          :   input data message size
unsigned int checksum_generate_key(unsigned char *_data,
                                   unsigned int _n)
{
    unsigned int i, sum=0;
    for (i=0; i<_n; i++)
        sum += (unsigned int) (_data[i]);

    // mask and convert to 2's complement
    unsigned char key = ~(sum&0x00ff) + 1;

    return key;
}


// 
// CRC-8
//

// generate 8-bit cyclic redundancy check key.
//
// slow method, operates one bit at a time
// algorithm from: http://www.hackersdelight.org/crc.pdf
//
//  _msg    :   input data message [size: _n x 1]
//  _n      :   input data message size
unsigned int crc8_generate_key(unsigned char *_msg,
                               unsigned int _n)
{
    unsigned int i, j, b, mask, key8=~0;
    unsigned int poly = liquid_reverse_byte_gentab[CRC8_POLY];
    for (i=0; i<_n; i++) {
        b = _msg[i];
        key8 ^= b;
        for (j=0; j<8; j++) {
            mask = -(key8 & 1);
            key8 = (key8>>1) ^ (poly & mask);
        }
    }
    return (~key8) & 0xff;
}


// 
// CRC-16
//

// generate 16-bit cyclic redundancy check key.
//
// slow method, operates one bit at a time
// algorithm from: http://www.hackersdelight.org/crc.pdf
//
//  _msg    :   input data message [size: _n x 1]
//  _n      :   input data message size
unsigned int crc16_generate_key(unsigned char *_msg,
                                unsigned int _n)
{
    unsigned int i, j, b, mask, key16=~0;
    unsigned int poly = liquid_reverse_uint16(CRC16_POLY);
    for (i=0; i<_n; i++) {
        b = _msg[i];
        key16 ^= b;
        for (j=0; j<8; j++) {
            mask = -(key16 & 1);
            key16 = (key16>>1) ^ (poly & mask);
        }
    }
    return (~key16) & 0xffff;
}


// 
// CRC-24
//

// generate 24-bit cyclic redundancy check key.
//
// slow method, operates one bit at a time
// algorithm from: http://www.hackersdelight.org/crc.pdf
//
//  _msg    :   input data message [size: _n x 1]
//  _n      :   input data message size
unsigned int crc24_generate_key(unsigned char *_msg,
                                unsigned int _n)
{
    unsigned int i, j, b, mask, key24=~0;
    unsigned int poly = liquid_reverse_uint24(CRC24_POLY);
    for (i=0; i<_n; i++) {
        b = _msg[i];
        key24 ^= b;
        for (j=0; j<8; j++) {
            mask = -(key24 & 1);
            key24 = (key24>>1) ^ (poly & mask);
        }
    }
    return (~key24) & 0xffffff;
}


// 
// CRC-32
//

// generate 32-bit cyclic redundancy check key.
//
// slow method, operates one bit at a time
// algorithm from: http://www.hackersdelight.org/crc.pdf
//
//  _msg    :   input data message [size: _n x 1]
//  _n      :   input data message size
unsigned int crc32_generate_key(unsigned char *_msg,
                                unsigned int _n)
{
    unsigned int i, j, b, mask, key32=~0;
    unsigned int poly = liquid_reverse_uint32(CRC32_POLY);
    for (i=0; i<_n; i++) {
        b = _msg[i];
        key32 ^= b;
        for (j=0; j<8; j++) {
            mask = -(key32 & 1);
            key32 = (key32>>1) ^ (poly & mask);
        }
    }
    return (~key32) & 0xffffffff;
}

#if 0
int crc32_generate_key(unsigned char *_msg,
                       unsigned int _n,
                       unsigned char *_key)
{
    unsigned int key32 = crc32_generate_key32(_msg,_n);
    _key[0] = (key32 & 0xFF000000) >> 24;
    _key[1] = (key32 & 0x00FF0000) >> 16;
    _key[2] = (key32 & 0x0000FF00) >> 8;
    _key[3] = (key32 & 0x000000FF);
    return LIQUID_OK;
}
#endif

