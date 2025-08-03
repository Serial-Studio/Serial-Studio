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
// liquid.internal.h
//
// Internal header file for liquid DSP for SDR
//
// This file includes function declarations which are intended
// for internal use
//

#ifndef __LIQUID_INTERNAL_H__
#define __LIQUID_INTERNAL_H__

// Configuration file
#include "config.h"

#include <stdarg.h>
#include <complex.h>
#include "liquid.h"

#if defined HAVE_FEC_H && defined HAVE_LIBFEC
#  define LIBFEC_ENABLED 1
#endif

// report error
int liquid_error_fl(int _code, const char * _file, int _line, const char * _format, ...);

// report error specifically for invalid object configuration 
void * liquid_error_config_fl(const char * _file, int _line, const char * _format, ...);

// macro to get file name and line number for source of error
#define liquid_error(code, format, ...) \
    liquid_error_fl(code, __FILE__, __LINE__, format, ##__VA_ARGS__);

// macro to get file name and line number for source of error (invalid object)
#define liquid_error_config(format, ...) \
    liquid_error_config_fl(__FILE__, __LINE__, format, ##__VA_ARGS__);

//
// Debugging macros
//
#define DEBUG_PRINTF_FLOAT(F,STR,I,V)                           \
    fprintf(F,"%s(%4u) = %12.4e;\n",STR,I+1,V)
#define DEBUG_PRINTF_CFLOAT(F,STR,I,V)                          \
    fprintf(F,"%s(%4u) = %12.4e +j*%12.4e;\n",STR,I+1,crealf(V),cimagf(V))

#define PRINTVAL_FLOAT(X,F)     printf(#F,crealf(X));
#define PRINTVAL_CFLOAT(X,F)    printf(#F "+j*" #F, crealf(X), cimagf(X));

//
// MODULE : agc
//


//
// MODULE : audio
//


//
// MODULE : buffer
//


//
// MODULE : dotprod
//


//
// MODULE : fec (forward error-correction)
//

// fec : basic object
struct fec_s {
    // common
    fec_scheme scheme;
    //unsigned int dec_msg_len;
    //unsigned int enc_msg_len;
    float rate;

    // lengths: convolutional, Reed-Solomon
    unsigned int num_dec_bytes;
    unsigned int num_enc_bytes;

    // convolutional : internal memory structure
    unsigned char * enc_bits;
    void * vp;      // decoder object
    int * poly;     // polynomial
    unsigned int R; // primitive rate, inverted (e.g. R=3 for 1/3)
    unsigned int K; // constraint length
    unsigned int P; // puncturing rate (e.g. p=3 for 3/4)
    int * puncturing_matrix;

    // viterbi decoder function pointers
    void*(*create_viterbi)(int);
    //void (*set_viterbi_polynomial)(int*);
    int  (*init_viterbi)(void*,int);
    int  (*update_viterbi_blk)(void*,unsigned char*,int);
    int  (*chainback_viterbi)(void*,unsigned char*,unsigned int,unsigned int);
    void (*delete_viterbi)(void*);

    // Reed-Solomon
    int symsize;    // symbol size (bits per symbol)
    int genpoly;    // generator polynomial
    int fcs;        //
    int prim;       //
    int nroots;     // number of roots in the polynomial
    //int ntrials;    //
    unsigned int rspad; // number of implicit padded symbols
    int nn;         // 2^symsize - 1
    int kk;         // nn - nroots
    void * rs;      // Reed-Solomon internal object

    // Reed-Solomon decoder
    unsigned int num_blocks;    // number of blocks: ceil(dec_msg_len / nn)
    unsigned int dec_block_len; // number of decoded bytes per block: 
    unsigned int enc_block_len; // number of encoded bytes per block: 
    unsigned int res_block_len; // residual bytes in last block
    unsigned int pad;           // padding for each block
    unsigned char * tblock;     // decoder input sequence, [size: 1 x n]
    int * errlocs;              // error locations, [size: 1 x n]
    int * derrlocs;             // decoded error locations, [size: 1 x n]
    int erasures;               // number of erasures

    // encode function pointer
    int (*encode_func)(fec _q,
                       unsigned int _dec_msg_len,
                       unsigned char * _msg_dec,
                       unsigned char * _msg_enc);

    // decode function pointer
    int (*decode_func)(fec _q,
                       unsigned int _dec_msg_len,
                       unsigned char * _msg_enc,
                       unsigned char * _msg_dec);

    // decode function pointer (soft decision)
    int (*decode_soft_func)(fec _q,
                            unsigned int _dec_msg_len,
                            unsigned char * _msg_enc,
                            unsigned char * _msg_dec);
};

// simple type testing
int fec_scheme_is_convolutional(fec_scheme _scheme);
int fec_scheme_is_punctured(fec_scheme _scheme);
int fec_scheme_is_reedsolomon(fec_scheme _scheme);
int fec_scheme_is_hamming(fec_scheme _scheme);
int fec_scheme_is_repeat(fec_scheme _scheme);

// Pass
fec fec_pass_create(void *_opts);
int fec_pass_destroy(fec _q);
int fec_pass_print(fec _q);
int fec_pass_encode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_dec,
                    unsigned char * _msg_enc);
int fec_pass_decode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_enc,
                    unsigned char * _msg_dec);

// Repeat (3)
fec fec_rep3_create(void *_opts);
int fec_rep3_destroy(fec _q);
int fec_rep3_print(fec _q);
int fec_rep3_encode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_dec,
                    unsigned char * _msg_enc);
int fec_rep3_decode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_enc,
                    unsigned char * _msg_dec);
int fec_rep3_decode_soft(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec);

// Repeat (5)
fec fec_rep5_create(void *_opts);
int fec_rep5_destroy(fec _q);
int fec_rep5_print(fec _q);
int fec_rep5_encode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_dec,
                    unsigned char * _msg_enc);
int fec_rep5_decode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_enc,
                    unsigned char * _msg_dec);
int fec_rep5_decode_soft(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec);

// Hamming(7,4)
extern unsigned char hamming74_enc_gentab[16];
extern unsigned char hamming74_dec_gentab[128];
fec fec_hamming74_create(void *_opts);
int fec_hamming74_destroy(fec _q);
int fec_hamming74_print(fec _q);
int fec_hamming74_encode(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_dec,
                         unsigned char * _msg_enc);
int fec_hamming74_decode(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec);
int fec_hamming74_decode_soft(fec _q,
                              unsigned int _dec_msg_len,
                              unsigned char * _msg_enc,
                              unsigned char * _msg_dec);
// soft decoding of one symbol
unsigned char fecsoft_hamming74_decode(unsigned char * _soft_bits);

// Hamming(8,4)
extern unsigned char hamming84_enc_gentab[16];
extern unsigned char hamming84_dec_gentab[256];
fec fec_hamming84_create(void *_opts);
int fec_hamming84_destroy(fec _q);
int fec_hamming84_print(fec _q);
int fec_hamming84_encode(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_dec,
                         unsigned char * _msg_enc);
int fec_hamming84_decode(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec);
int fec_hamming84_decode_soft(fec _q,
                              unsigned int _dec_msg_len,
                              unsigned char * _msg_enc,
                              unsigned char * _msg_dec);
// soft decoding of one symbol
unsigned char fecsoft_hamming84_decode(unsigned char * _soft_bits);

// Hamming(12,8)

unsigned int fec_hamming128_encode_symbol(unsigned int _sym_dec);
unsigned int fec_hamming128_decode_symbol(unsigned int _sym_enc);
extern unsigned short int hamming128_enc_gentab[256];   // encoding table

fec fec_hamming128_create(void *_opts);
int fec_hamming128_destroy(fec _q);
int fec_hamming128_print(fec _q);
int fec_hamming128_encode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc);
int fec_hamming128_decode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec);
int fec_hamming128_decode_soft(fec _q,
                               unsigned int _dec_msg_len,
                               unsigned char * _msg_enc,
                               unsigned char * _msg_dec);
// soft decoding of one symbol
unsigned int fecsoft_hamming128_decode(unsigned char * _soft_bits);
extern unsigned char fecsoft_hamming128_n3[256][17];
unsigned int fecsoft_hamming128_decode_n3(unsigned char * _soft_bits);


// Hamming(15,11)
unsigned int fec_hamming1511_encode_symbol(unsigned int _sym_dec);
unsigned int fec_hamming1511_decode_symbol(unsigned int _sym_enc);

// Hamming(31,26)
unsigned int fec_hamming3126_encode_symbol(unsigned int _sym_dec);
unsigned int fec_hamming3126_decode_symbol(unsigned int _sym_enc);


// Golay(24,12)

unsigned int fec_golay2412_encode_symbol(unsigned int _sym_dec);
unsigned int fec_golay2412_decode_symbol(unsigned int _sym_enc);
extern unsigned int golay2412_P[12];
extern unsigned int golay2412_Gt[24];
extern unsigned int golay2412_H[12];

// multiply input vector with matrix
unsigned int golay2412_matrix_mul(unsigned int   _v,
                                  unsigned int * _A,
                                  unsigned int   _n);

// search for p[i] such that w(v+p[i]) <= 2, return -1 on fail
int golay2412_parity_search(unsigned int _v);

fec fec_golay2412_create(void *_opts);
int fec_golay2412_destroy(fec _q);
int fec_golay2412_print(fec _q);
int fec_golay2412_encode(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_dec,
                         unsigned char * _msg_enc);
int fec_golay2412_decode(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec);

// SEC-DED (22,16)

// compute parity on 16-bit input
unsigned char fec_secded2216_compute_parity(unsigned char * _m);

// compute syndrome on 22-bit input
unsigned char fec_secded2216_compute_syndrome(unsigned char * _v);

// encode symbol
//  _sym_dec    :   decoded symbol, [size: 2 x 1]
//  _sym_enc    :   encoded symbol, [size: 3 x 1], _sym_enc[0] has only 6 bits
int fec_secded2216_encode_symbol(unsigned char * _sym_dec,
                                 unsigned char * _sym_enc);

// decode symbol, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol, [size: 3 x 1], _sym_enc[0] has only 6 bits
//  _sym_dec    :   decoded symbol, [size: 2 x 1]
int  fec_secded2216_decode_symbol(unsigned char * _sym_enc,
                                  unsigned char * _sym_dec);

// estimate error vector, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol, [size: 3 x 1], _sym_enc[0] has only 6 bits
//  _e_hat      :   estimated error vector, [size: 3 x 1]
int  fec_secded2216_estimate_ehat(unsigned char * _sym_enc,
                                  unsigned char * _e_hat);

// parity matrix [6 x 16 bits], [6 x 2 bytes]
extern unsigned char secded2216_P[12];

// syndrome vectors of errors with weight exactly equal to 1
extern unsigned char secded2216_syndrome_w1[22];

fec fec_secded2216_create(void *_opts);
int fec_secded2216_destroy(fec _q);
int fec_secded2216_print(fec _q);
int fec_secded2216_encode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc);
int fec_secded2216_decode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec);

// SEC-DED (39,32)

// compute parity on 32-bit input
unsigned char fec_secded3932_compute_parity(unsigned char * _m);

// compute syndrome on 39-bit input
unsigned char fec_secded3932_compute_syndrome(unsigned char * _v);

// encode symbol
//  _sym_dec    :   decoded symbol, [size: 4 x 1]
//  _sym_enc    :   encoded symbol, [size: 5 x 1], _sym_enc[0] has only 7 bits
int fec_secded3932_encode_symbol(unsigned char * _sym_dec,
                                 unsigned char * _sym_enc);

// estimate error vector, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol, [size: 5 x 1], _sym_enc[0] has only 7 bits
//  _e_hat      :   estimated error vector, [size: 5 x 1]
int  fec_secded3932_estimate_ehat(unsigned char * _sym_enc,
                                  unsigned char * _e_hat);

// decode symbol, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol (_sym_enc[0] has only 7 bits), [size: 5 x 1]
//  _sym_dec    :   decoded symbol, [size: 4 x 1]
int fec_secded3932_decode_symbol(unsigned char * _sym_enc,
                                unsigned char * _sym_dec);

// parity matrix [7 x 32 bits], [7 x 4 bytes]
extern unsigned char secded3932_P[28];

// syndrome vectors of errors with weight exactly equal to 1
extern unsigned char secded3932_syndrome_w1[39];

fec fec_secded3932_create(void *_opts);
int fec_secded3932_destroy(fec _q);
int fec_secded3932_print(fec _q);
int fec_secded3932_encode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc);
int fec_secded3932_decode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec);

// SEC-DED (72,64)

// compute parity byte on 64-byte input
unsigned char fec_secded7264_compute_parity(unsigned char * _v);

// compute syndrome on 72-bit input
unsigned char fec_secded7264_compute_syndrome(unsigned char * _v);

// encode symbol
//  _sym_dec    :   input symbol, [size: 8 x 1]
//  _sym_enc    :   input symbol, [size: 9 x 1]
int fec_secded7264_encode_symbol(unsigned char * _sym_dec,
                                 unsigned char * _sym_enc);

// estimate error vector, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol, [size: 9 x 1]
//  _e_hat      :   estimated error vector, [size: 9 x 1]
int fec_secded7264_estimate_ehat(unsigned char * _sym_enc,
                                 unsigned char * _e_hat);

// decode symbol, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   input symbol, [size: 8 x 1]
//  _sym_dec    :   input symbol, [size: 9 x 1]
int fec_secded7264_decode_symbol(unsigned char * _sym_enc,
                                 unsigned char * _sym_dec);

extern unsigned char secded7264_P[64];
extern unsigned char secded7264_syndrome_w1[72];

fec fec_secded7264_create(void *_opts);
int fec_secded7264_destroy(fec _q);
int fec_secded7264_print(fec _q);
int fec_secded7264_encode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_dec,
                          unsigned char * _msg_enc);
int fec_secded7264_decode(fec _q,
                          unsigned int _dec_msg_len,
                          unsigned char * _msg_enc,
                          unsigned char * _msg_dec);


// Convolutional: r1/2 K=7
//                r1/2 K=9
//                r1/3 K=9
//                r1/6 K=15

// compute encoded message length for block codes
//  _dec_msg_len    :   decoded message length (bytes)
//  _m              :   input block size (bits)
//  _k              :   output block size (bits)
unsigned int fec_block_get_enc_msg_len(unsigned int _dec_msg_len,
                                       unsigned int _m,
                                       unsigned int _k);

// compute encoded message length for convolutional codes
//  _dec_msg_len    :   decoded message length
//  _K              :   constraint length
//  _p              :   puncturing rate, r = _p / (_p+1)
unsigned int fec_conv_get_enc_msg_len(unsigned int _dec_msg_len,
                                      unsigned int _K,
                                      unsigned int _p);

// convolutional code polynomials
extern int fec_conv27_poly[2];
extern int fec_conv29_poly[2];
extern int fec_conv39_poly[3];
extern int fec_conv615_poly[6];

// convolutional code puncturing matrices  [R x P]
extern int fec_conv27p23_matrix[4];     // [2 x 2]
extern int fec_conv27p34_matrix[6];     // [2 x 3]
extern int fec_conv27p45_matrix[8];     // [2 x 4]
extern int fec_conv27p56_matrix[10];    // [2 x 5]
extern int fec_conv27p67_matrix[12];    // [2 x 6]
extern int fec_conv27p78_matrix[14];    // [2 x 7]

extern int fec_conv29p23_matrix[4];     // [2 x 2]
extern int fec_conv29p34_matrix[6];     // [2 x 3]
extern int fec_conv29p45_matrix[8];     // [2 x 4]
extern int fec_conv29p56_matrix[10];    // [2 x 5]
extern int fec_conv29p67_matrix[12];    // [2 x 6]
extern int fec_conv29p78_matrix[14];    // [2 x 7]

fec fec_conv_create(fec_scheme _fs);
int fec_conv_destroy(fec _q);
int fec_conv_print(fec _q);
int fec_conv_encode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_dec,
                    unsigned char * _msg_enc);
int fec_conv_decode_hard(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec);
int fec_conv_decode_soft(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec);
int fec_conv_decode(fec _q, unsigned char * _msg_dec);
int fec_conv_setlength(fec _q, unsigned int _dec_msg_len);

// internal initialization methods (sets r, K, viterbi methods)
int fec_conv_init_v27(fec _q);
int fec_conv_init_v29(fec _q);
int fec_conv_init_v39(fec _q);
int fec_conv_init_v615(fec _q);

// punctured convolutional codes
fec fec_conv_punctured_create(fec_scheme _fs);
int fec_conv_punctured_destroy(fec _q);
int fec_conv_punctured_print(fec _q);
int fec_conv_punctured_encode(fec _q,
                               unsigned int _dec_msg_len,
                               unsigned char * _msg_dec,
                               unsigned char * _msg_enc);
int fec_conv_punctured_decode_hard(fec _q,
                                   unsigned int _dec_msg_len,
                                   unsigned char * _msg_enc,
                                   unsigned char * _msg_dec);
int fec_conv_punctured_decode_soft(fec _q,
                                   unsigned int _dec_msg_len,
                                   unsigned char * _msg_enc,
                                   unsigned char * _msg_dec);
int fec_conv_punctured_setlength(fec _q, unsigned int _dec_msg_len);

// internal initialization methods (sets r, K, viterbi methods,
// and puncturing matrix)
int fec_conv_init_v27p23(fec _q);
int fec_conv_init_v27p34(fec _q);
int fec_conv_init_v27p45(fec _q);
int fec_conv_init_v27p56(fec _q);
int fec_conv_init_v27p67(fec _q);
int fec_conv_init_v27p78(fec _q);

int fec_conv_init_v29p23(fec _q);
int fec_conv_init_v29p34(fec _q);
int fec_conv_init_v29p45(fec _q);
int fec_conv_init_v29p56(fec _q);
int fec_conv_init_v29p67(fec _q);
int fec_conv_init_v29p78(fec _q);

// Reed-Solomon

// compute encoded message length for Reed-Solomon codes
//  _dec_msg_len    :   decoded message length
//  _nroots         :   number of roots in polynomial
//  _nn             :   
//  _kk             :   
unsigned int fec_rs_get_enc_msg_len(unsigned int _dec_msg_len,
                                    unsigned int _nroots,
                                    unsigned int _nn,
                                    unsigned int _kk);


fec fec_rs_create(fec_scheme _fs);
int fec_rs_destroy(fec _q);
int fec_rs_init_p8(fec _q);
int fec_rs_setlength(fec _q,
                     unsigned int _dec_msg_len);
int fec_rs_encode(fec _q,
                  unsigned int _dec_msg_len,
                  unsigned char * _msg_dec,
                  unsigned char * _msg_enc);
int fec_rs_decode(fec _q,
                  unsigned int _dec_msg_len,
                  unsigned char * _msg_enc,
                  unsigned char * _msg_dec);

// phi(x) = -logf( tanhf( x/2 ) )
float sumproduct_phi(float _x);

// iterate over the sum-product algorithm:
// returns 1 if parity checks, 0 otherwise
//  _m          :   rows
//  _n          :   cols
//  _H          :   sparse binary parity check matrix, [size: _m x _n]
//  _LLR        :   received signal (soft bits, LLR) [size: _n x 1]
//  _c_hat      :   estimated transmitted signal, [size: _n x 1]
//  _max_steps  :   maximum number of steps before bailing
int fec_sumproduct(unsigned int    _m,
                   unsigned int    _n,
                   smatrixb        _H,
                   float *         _LLR,
                   unsigned char * _c_hat,
                   unsigned int    _max_steps);

// sum-product algorithm, returns 1 if parity checks, 0 otherwise
//  _m      :   rows
//  _n      :   cols
//  _H      :   sparse binary parity check matrix, [size: _m x _n]
//  _c_hat  :   estimated transmitted signal, [size: _n x 1]
//
// internal state arrays
//  _Lq     :   [size: _m x _n]
//  _Lr     :   [size: _m x _n]
//  _Lc     :   [size: _n x 1]
//  _LQ     :   [size: _n x 1]
//  _parity :   _H * _c_hat, [size: _m x 1]
int fec_sumproduct_step(unsigned int    _m,
                        unsigned int    _n,
                        smatrixb        _H,
                        unsigned char * _c_hat,
                        float *         _Lq,
                        float *         _Lr,
                        float *         _Lc,
                        float *         _LQ,
                        unsigned char * _parity);

//
// packetizer
//

#define PACKETIZER_VERSION (1)


//
// MODULE : fft (fast discrete Fourier transform)
//

// fast fourier transform method
typedef enum {
    LIQUID_FFT_METHOD_UNKNOWN=0,    // unknown method
    LIQUID_FFT_METHOD_RADIX2,       // Radix-2 (decimation in time)
    LIQUID_FFT_METHOD_MIXED_RADIX,  // Cooley-Tukey mixed-radix FFT (decimation in time)
    LIQUID_FFT_METHOD_RADER,        // Rader's method for FFTs of prime length
    LIQUID_FFT_METHOD_RADER2,       // Rader's method for FFTs of prime length (alternate)
    LIQUID_FFT_METHOD_DFT,          // regular discrete Fourier transform
} liquid_fft_method;

// Macro    :   FFT (internal)
//  FFT     :   name-mangling macro
//  T       :   primitive data type
//  TC      :   primitive data type (complex)
#define LIQUID_FFT_DEFINE_INTERNAL_API(FFT,T,TC)                \
                                                                \
/* print plan recursively */                                    \
int FFT(_print_plan_recursive)(FFT(plan)    _q,                 \
                               unsigned int _level);            \
                                                                \
/* type definitions for create/destroy/execute functions */     \
typedef FFT(plan)(FFT(_create_t)) (unsigned int _nfft,          \
                                   TC *         _x,             \
                                   TC *         _y,             \
                                   int          _dir,           \
                                   int          _flags);        \
typedef int (FFT(_destroy_t))(FFT(plan) _q);                    \
typedef int (FFT(_execute_t))(FFT(plan) _q);                    \
                                                                \
/* FFT create methods */                                        \
FFT(_create_t) FFT(_create_plan_dft);                           \
FFT(_create_t) FFT(_create_plan_radix2);                        \
FFT(_create_t) FFT(_create_plan_mixed_radix);                   \
FFT(_create_t) FFT(_create_plan_rader);                         \
FFT(_create_t) FFT(_create_plan_rader2);                        \
                                                                \
/* FFT destroy methods */                                       \
FFT(_destroy_t) FFT(_destroy_plan_dft);                         \
FFT(_destroy_t) FFT(_destroy_plan_radix2);                      \
FFT(_destroy_t) FFT(_destroy_plan_mixed_radix);                 \
FFT(_destroy_t) FFT(_destroy_plan_rader);                       \
FFT(_destroy_t) FFT(_destroy_plan_rader2);                      \
                                                                \
/* FFT execute methods */                                       \
FFT(_execute_t) FFT(_execute_dft);                              \
FFT(_execute_t) FFT(_execute_radix2);                           \
FFT(_execute_t) FFT(_execute_mixed_radix);                      \
FFT(_execute_t) FFT(_execute_rader);                            \
FFT(_execute_t) FFT(_execute_rader2);                           \
                                                                \
/* specific codelets for small DFTs */                          \
FFT(_execute_t) FFT(_execute_dft_2);                            \
FFT(_execute_t) FFT(_execute_dft_3);                            \
FFT(_execute_t) FFT(_execute_dft_4);                            \
FFT(_execute_t) FFT(_execute_dft_5);                            \
FFT(_execute_t) FFT(_execute_dft_6);                            \
FFT(_execute_t) FFT(_execute_dft_7);                            \
FFT(_execute_t) FFT(_execute_dft_8);                            \
FFT(_execute_t) FFT(_execute_dft_16);                           \
                                                                \
/* additional methods */                                        \
unsigned int FFT(_estimate_mixed_radix)(unsigned int _nfft);    \
                                                                \
/* discrete cosine transform (DCT) prototypes */                \
int FFT(_execute_REDFT00)(FFT(plan) _q);    /* DCT-I   */       \
int FFT(_execute_REDFT10)(FFT(plan) _q);    /* DCT-II  */       \
int FFT(_execute_REDFT01)(FFT(plan) _q);    /* DCT-III */       \
int FFT(_execute_REDFT11)(FFT(plan) _q);    /* DCT-IV  */       \
                                                                \
/* discrete sine transform (DST) prototypes */                  \
int FFT(_execute_RODFT00)(FFT(plan) _q);    /* DST-I   */       \
int FFT(_execute_RODFT10)(FFT(plan) _q);    /* DST-II  */       \
int FFT(_execute_RODFT01)(FFT(plan) _q);    /* DST-III */       \
int FFT(_execute_RODFT11)(FFT(plan) _q);    /* DST-IV  */       \
                                                                \
/* destroy real-to-real one-dimensional plan */                 \
int FFT(_destroy_plan_r2r_1d)(FFT(plan) _q);                    \
                                                                \
/* print real-to-real one-dimensional plan */                   \
int FFT(_print_plan_r2r_1d)(FFT(plan) _q);                      \

// determine best FFT method based on size
liquid_fft_method liquid_fft_estimate_method(unsigned int _nfft);

// is input radix-2?
int fft_is_radix2(unsigned int _n);

// miscellaneous functions
unsigned int fft_reverse_index(unsigned int _i, unsigned int _n);


LIQUID_FFT_DEFINE_INTERNAL_API(LIQUID_FFT_MANGLE_FLOAT, float, liquid_float_complex)

// Use fftw library if installed (and not overridden with configuration),
// otherwise use internal (less efficient) fft library.
#if HAVE_FFTW3_H && !defined LIQUID_FFTOVERRIDE
#   include <fftw3.h>
#   define FFT_PLAN             fftwf_plan
#   define FFT_CREATE_PLAN      fftwf_plan_dft_1d
#   define FFT_DESTROY_PLAN     fftwf_destroy_plan
#   define FFT_EXECUTE          fftwf_execute
#   define FFT_DIR_FORWARD      FFTW_FORWARD
#   define FFT_DIR_BACKWARD     FFTW_BACKWARD
#   define FFT_METHOD           FFTW_ESTIMATE
#   define FFT_MALLOC           fftwf_malloc
#   define FFT_FREE             fftwf_free
#else
#   define FFT_PLAN             fftplan
#   define FFT_CREATE_PLAN      fft_create_plan
#   define FFT_DESTROY_PLAN     fft_destroy_plan
#   define FFT_EXECUTE          fft_execute
#   define FFT_DIR_FORWARD      LIQUID_FFT_FORWARD
#   define FFT_DIR_BACKWARD     LIQUID_FFT_BACKWARD
#   define FFT_METHOD           0
#   define FFT_MALLOC           malloc
#   define FFT_FREE             free
#endif



//
// MODULE : filter
//

// estimate required filter length given transition bandwidth and
// stop-band attenuation (algorithm from [Vaidyanathan:1993])
//  _df     :   transition bandwidth (0 < _df < 0.5)
//  _as     :   stop-band attenuation [dB] (_as > 0)
float estimate_req_filter_len_Kaiser(float _df,
                                     float _as);

// estimate required filter length given transition bandwidth and
// stop-band attenuation (algorithm from [Herrmann:1973])
//  _df     :   transition bandwidth (0 < _df < 0.5)
//  _as     :   stop-band attenuation [dB] (_as > 0)
float estimate_req_filter_len_Herrmann(float _df,
                                       float _as);


// firdes : finite impulse response filter design

// Find approximate bandwidth adjustment factor rho based on
// filter delay and desired excess bandwidth factor.
//
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
float rkaiser_approximate_rho(unsigned int _m,
                              float _beta);

// Design frequency-shifted root-Nyquist filter based on
// the Kaiser-windowed sinc using the quadratic method.
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter, [size: 2*_k*_m+1]
//  _rho    :   transition bandwidth adjustment, 0 < _rho < 1
int liquid_firdes_rkaiser_quadratic(unsigned int _k,
                                    unsigned int _m,
                                    float _beta,
                                    float _dt,
                                    float * _h,
                                    float * _rho);

// compute filter coefficients and determine resulting ISI
//  
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _rho    :   transition bandwidth adjustment, 0 < _rho < 1
//  _h      :   filter buffer, [size: 2*_k*_m+1]
float liquid_firdes_rkaiser_internal_isi(unsigned int _k,
                                         unsigned int _m,
                                         float _beta,
                                         float _dt,
                                         float _rho,
                                         float * _h);

// Design flipped Nyquist/root-Nyquist filters
int liquid_firdes_fnyquist(liquid_firfilt_type _type,
                           int                 _root,
                           unsigned int        _k,
                           unsigned int        _m,
                           float               _beta,
                           float               _dt,
                           float *             _h);

// flipped exponential frequency response
int liquid_firdes_fexp_freqresponse(unsigned int _k,
                                    unsigned int _m,
                                    float        _beta,
                                    float *      _H);

// flipped hyperbolic secant frequency response
int liquid_firdes_fsech_freqresponse(unsigned int _k,
                                     unsigned int _m,
                                     float        _beta,
                                     float *      _H);

// flipped hyperbolic secant frequency response
int liquid_firdes_farcsech_freqresponse(unsigned int _k,
                                        unsigned int _m,
                                        float        _beta,
                                        float *      _H);

// iirdes : infinite impulse response filter design

// Sorts array _z of complex numbers into complex conjugate pairs to
// within a tolerance. Conjugate pairs are ordered by increasing real
// component with the negative imaginary element first. All pure-real
// elements are placed at the end of the array.
//
// Example:
//      v:              liquid_cplxpair(v):
//      10 + j*3        -3 - j*4
//       5 + j*0         3 + j*4
//      -3 + j*4        10 - j*3
//      10 - j*3        10 + j*3
//       3 + j*0         3 + j*0
//      -3 + j*4         5 + j*0
//
//  _z      :   complex array (size _n)
//  _n      :   number of elements in _z
//  _tol    :   tolerance for finding complex pairs
//  _p      :   resulting pairs, pure real values of _z at end
int liquid_cplxpair(float complex * _z,
                    unsigned int    _n,
                    float           _tol,
                    float complex * _p);

// post-process cleanup used with liquid_cplxpair
//
// once pairs have been identified and ordered, this method
// will clean up the result by ensuring the following:
//  * pairs are perfect conjugates
//  * pairs have negative imaginary component first
//  * pairs are ordered by increasing real component
//  * pure-real elements are ordered by increasing value
//
//  _p          :   pre-processed complex array, [size: _n x 1]
//  _n          :   array length
//  _num_pairs  :   number of complex conjugate pairs
int liquid_cplxpair_cleanup(float complex * _p,
                            unsigned int    _n,
                            unsigned int    _num_pairs);

// Jacobian elliptic functions (src/filter/src/ellip.c)

// Landen transformation (_n iterations)
int landenf(float _k,
            unsigned int _n,
            float * _v);

// compute elliptic integral K(k) for _n recursions
int ellipkf(float _k,
            unsigned int _n,
            float * _K,
            float * _Kp);

// elliptic degree
float ellipdegf(float _N,
                float _k1,
                unsigned int _n);

// elliptic cd() function (_n recursions)
float complex ellip_cdf(float complex _u,
                        float _k,
                        unsigned int _n);

// elliptic inverse cd() function (_n recursions)
float complex ellip_acdf(float complex _u,
                         float _k,
                         unsigned int _n);

// elliptic sn() function (_n recursions)
float complex ellip_snf(float complex _u,
                        float _k,
                        unsigned int _n);

// elliptic inverse sn() function (_n recursions)
float complex ellip_asnf(float complex _u,
                         float _k,
                         unsigned int _n);

//
// MODULE : framing
//

//
// bpacket
//

#define BPACKET_VERSION (101+PACKETIZER_VERSION)

// generator
void bpacketgen_compute_packet_len(bpacketgen _q);
void bpacketgen_assemble_pnsequence(bpacketgen _q);
void bpacketgen_assemble_header(bpacketgen _q);

// 
// flexframe
//

// flexframe protocol
#define FLEXFRAME_PROTOCOL  (101+PACKETIZER_VERSION)

// header description
#define FLEXFRAME_H_USER_DEFAULT (14)                    // default length for user-defined array
#define FLEXFRAME_H_DEC          (6)                     // decoded length
#define FLEXFRAME_H_CRC          (LIQUID_CRC_32)         // header CRC
#define FLEXFRAME_H_FEC0         (LIQUID_FEC_SECDED7264) // header FEC (inner)
#define FLEXFRAME_H_FEC1         (LIQUID_FEC_HAMMING84)  // header FEC (outer)
#define FLEXFRAME_H_MOD          (LIQUID_MODEM_QPSK)     // modulation scheme


// 
// gmskframe
//

#define GMSKFRAME_VERSION (3+PACKETIZER_VERSION)

// header description
#define GMSKFRAME_H_USER_DEFAULT   (8)                     // user-defined array
#define GMSKFRAME_H_DEC            (5)                     // decoded length
#define GMSKFRAME_H_CRC            (LIQUID_CRC_32)         // header CRC
#define GMSKFRAME_H_FEC            (LIQUID_FEC_HAMMING128) // header FEC


// 
// ofdmflexframe
//

#define OFDMFLEXFRAME_PROTOCOL  (104+PACKETIZER_VERSION)

// header description
#define OFDMFLEXFRAME_H_USER_DEFAULT (8)                         // default length for user-defined array
#define OFDMFLEXFRAME_H_DEC          (6)                         // decoded length
#define OFDMFLEXFRAME_H_CRC          (LIQUID_CRC_32)             // header CRC
#define OFDMFLEXFRAME_H_FEC0         (LIQUID_FEC_GOLAY2412)      // header FEC (inner)
#define OFDMFLEXFRAME_H_FEC1         (LIQUID_FEC_NONE)           // header FEC (outer)
#define OFDMFLEXFRAME_H_MOD          (LIQUID_MODEM_BPSK)         // modulation scheme


//
// dsssframe
//

#define DSSSFRAME_PROTOCOL (101 + PACKETIZER_VERSION)
#define DSSSFRAME_H_USER_DEFAULT (8)
#define DSSSFRAME_H_DEC          (5)
#define DSSSFRAME_H_CRC          (LIQUID_CRC_32)
#define DSSSFRAME_H_FEC0         (LIQUID_FEC_GOLAY2412)
#define DSSSFRAME_H_FEC1         (LIQUID_FEC_NONE)

//
// multi-signal source for testing (no meaningful data, just signals)
//
#define LIQUID_QSOURCE_MANGLE_CFLOAT(name) LIQUID_CONCAT(qsourcecf,name)

#define LIQUID_QSOURCE_DEFINE_API(QSOURCE,TO)                               \
                                                                            \
/* Multi-signal source generator object                                 */  \
typedef struct QSOURCE(_s) * QSOURCE();                                     \
                                                                            \
/* Create default qsource object, type uninitialized                    */  \
QSOURCE() QSOURCE(_create)(unsigned int _M,                                 \
                           unsigned int _m,                                 \
                           float        _as,                                \
                           float        _fc,                                \
                           float        _bw,                                \
                           float        _gain);                             \
                                                                            \
/* Copy object recursively, including all internal objects and state    */  \
QSOURCE() QSOURCE(_copy)(QSOURCE() _q);                                     \
                                                                            \
/* Initialize user-defined qsource object                               */  \
int QSOURCE(_init_user)(QSOURCE() _q,                                       \
                        void *    _userdata,                                \
                        void *    _callback);                               \
                                                                            \
/* Initialize qsource tone object                                       */  \
int QSOURCE(_init_tone)(QSOURCE() _q);                                      \
                                                                            \
/* Add chirp to signal generator, returning id of signal                */  \
/*  _q          : signal source object                                  */  \
/*  _duration   : duration of chirp [samples]                           */  \
/*  _negate     : negate frequency direction                            */  \
/*  _repeat     : repeat signal? or just run once                       */  \
int QSOURCE(_init_chirp)(QSOURCE() _q,                                      \
                         float     _duration,                               \
                         int       _negate,                                 \
                         int       _repeat);                                \
                                                                            \
/* Initialize qsource noise object                                      */  \
int QSOURCE(_init_noise)(QSOURCE() _q);                                     \
                                                                            \
/* Initialize qsource linear modem object                               */  \
int QSOURCE(_init_modem)(QSOURCE()    _q,                                   \
                         int          _ms,                                  \
                         unsigned int _m,                                   \
                         float        _beta);                               \
                                                                            \
/* Initialize frequency-shift keying modem signal source                */  \
/*  _q      : signal source object                                      */  \
/*  _m      : bits per symbol, _bps > 0                                 */  \
/*  _k      : samples/symbol, _k >= 2^_m                                */  \
int QSOURCE(_init_fsk)(QSOURCE()    _q,                                     \
                       unsigned int _m,                                     \
                       unsigned int _k);                                    \
                                                                            \
/* Initialize qsource GMSK modem object                                 */  \
int QSOURCE(_init_gmsk)(QSOURCE()    _q,                                    \
                        unsigned int _m,                                    \
                        float        _bt);                                  \
                                                                            \
/* Destroy qsource object                                               */  \
int QSOURCE(_destroy)(QSOURCE() _q);                                        \
                                                                            \
/* Print qsource object                                                 */  \
int QSOURCE(_print)(QSOURCE() _q);                                          \
                                                                            \
/* Reset qsource object                                                 */  \
int QSOURCE(_reset)(QSOURCE() _q);                                          \
                                                                            \
/* Get/set source id                                                    */  \
int QSOURCE(_set_id)(QSOURCE() _q, int _id);                                \
int QSOURCE(_get_id)(QSOURCE() _q);                                         \
                                                                            \
int QSOURCE(_enable)(QSOURCE() _q);                                         \
int QSOURCE(_disable)(QSOURCE() _q);                                        \
                                                                            \
/* Get number of samples generated by the object so far                 */  \
/*  _q      : msource object                                            */  \
/*  _gain   : signal gain output [dB]                                   */  \
uint64_t QSOURCE(_get_num_samples)(QSOURCE() _q);                           \
                                                                            \
int QSOURCE(_set_gain)(QSOURCE() _q,                                        \
                       float     _gain_dB);                                 \
                                                                            \
float QSOURCE(_get_gain)(QSOURCE() _q);                                     \
                                                                            \
int QSOURCE(_set_frequency)(QSOURCE() _q,                                   \
                            float     _dphi);                               \
                                                                            \
float QSOURCE(_get_frequency)(QSOURCE() _q);                                \
                                                                            \
/* get center frequency of signal applied by channelizer alignment */       \
float QSOURCE(_get_frequency_index)(QSOURCE() _q);                          \
                                                                            \
int QSOURCE(_generate)(QSOURCE() _q,                                        \
                       TO *      _v);                                       \
                                                                            \
int QSOURCE(_generate_into)(QSOURCE() _q,                                   \
                            TO *      _buf);                                \

LIQUID_QSOURCE_DEFINE_API(LIQUID_QSOURCE_MANGLE_CFLOAT, liquid_float_complex)

//
// MODULE : math
//

// 
// basic trigonometric functions
//
float liquid_sinf(float _x);
float liquid_cosf(float _x);
float liquid_tanf(float _x);
void  liquid_sincosf(float _x,
                     float * _sinf,
                     float * _cosf);
float liquid_expf(float _x);
float liquid_logf(float _x);

// 
// complex math operations
//

// complex square root
float complex liquid_csqrtf(float complex _z);

// complex exponent, logarithm
float complex liquid_cexpf(float complex _z);
float complex liquid_clogf(float complex _z);

// complex arcsin, arccos, arctan
float complex liquid_casinf(float complex _z);
float complex liquid_cacosf(float complex _z);
float complex liquid_catanf(float complex _z);

// faster approximation to arg{*}
//float liquid_cargf_approx(float complex _z);


// internal trig helper functions

// complex rotation vector: cexpf(_Complex_I*THETA)
#define liquid_cexpjf(THETA) (cosf(THETA) + _Complex_I*sinf(THETA))

// internal polynomial root-finding methods

// finds the complex roots of the polynomial using the Durand-Kerner method
//  _p      :   polynomial array, ascending powers, [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _roots  :   resulting complex roots, [size: _k-1 x 1]
int liquid_poly_findroots_durandkerner(double *         _p,
                                       unsigned int     _k,
                                       double complex * _roots);

// finds the complex roots of the polynomial using Bairstow's method
//  _p      :   polynomial array, ascending powers, [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _roots  :   resulting complex roots, [size: _k-1 x 1]
int liquid_poly_findroots_bairstow(double *         _p,
                                   unsigned int     _k,
                                   double complex * _roots);

// iterate over Bairstow's method, finding quadratic factor x^2 + u*x + v
//  _p      :   polynomial array, ascending powers, [size: _k x 1]
//  _k      :   polynomials length (poly order = _k - 1)
//  _p1     :   reduced polynomial (output) [size: _k-2 x 1]
//  _u      :   input: initial estimate for u; output: resulting u
//  _v      :   input: initial estimate for v; output: resulting v
int liquid_poly_findroots_bairstow_recursion(double *     _p,
                                             unsigned int _k,
                                             double *     _p1,
                                             double *     _u,
                                             double *     _v);

// run multiple iterations of Bairstow's method with different starting
// conditions looking for convergence
int liquid_poly_findroots_bairstow_persistent(double * _p,
                                         unsigned int  _k,
                                         double *      _p1,
                                         double *      _u,
                                         double *      _v);

// compare roots for sorting
int liquid_poly_sort_roots_compare(const void * _a,
                                   const void * _b);



//
// MODULE : matrix
//

// large macro
//   MATRIX : name-mangling macro
//   T      : data type
#define LIQUID_MATRIX_DEFINE_INTERNAL_API(MATRIX,T)             \
T    MATRIX(_det2x2)(T * _x,                                    \
                     unsigned int _rx,                          \
                     unsigned int _cx);


LIQUID_MATRIX_DEFINE_INTERNAL_API(LIQUID_MATRIX_MANGLE_FLOAT,   float)
LIQUID_MATRIX_DEFINE_INTERNAL_API(LIQUID_MATRIX_MANGLE_DOUBLE,  double)

LIQUID_MATRIX_DEFINE_INTERNAL_API(LIQUID_MATRIX_MANGLE_CFLOAT,  liquid_float_complex)
LIQUID_MATRIX_DEFINE_INTERNAL_API(LIQUID_MATRIX_MANGLE_CDOUBLE, liquid_double_complex)

// search for index placement in list
unsigned short int smatrix_indexsearch(unsigned short int * _list,
                                       unsigned int         _num_elements,
                                       unsigned short int   _value);




//
// MODULE : modem
//

// 'Square' QAM
#define QAM4_ALPHA      (1./sqrt(2))
#define QAM8_ALPHA      (1./sqrt(6))
#define QAM16_ALPHA     (1./sqrt(10))
#define QAM32_ALPHA     (1./sqrt(20))
#define QAM64_ALPHA     (1./sqrt(42))
#define QAM128_ALPHA    (1./sqrt(82))
#define QAM256_ALPHA    (1./sqrt(170))
#define QAM1024_ALPHA   (1./sqrt(682))
#define QAM4096_ALPHA   (1./sqrt(2730))

// Rectangular QAM
#define RQAM4_ALPHA     QAM4_ALPHA
#define RQAM8_ALPHA     QAM8_ALPHA
#define RQAM16_ALPHA    QAM16_ALPHA
#define RQAM32_ALPHA    (1./sqrt(26))
#define RQAM64_ALPHA    QAM64_ALPHA
#define RQAM128_ALPHA   (1./sqrt(106))
#define RQAM256_ALPHA   QAM256_ALPHA
#define RQAM512_ALPHA   (1./sqrt(426))
#define RQAM1024_ALPHA  QAM1024_ALPHA
#define RQAM2048_ALPHA  (1./sqrt(1706))
#define RQAM4096_ALPHA  QAM4096_ALPHA

// ASK
#define ASK2_ALPHA      (1.)
#define ASK4_ALPHA      (1./sqrt(5))
#define ASK8_ALPHA      (1./sqrt(21))
#define ASK16_ALPHA     (1./sqrt(85))
#define ASK32_ALPHA     (1./sqrt(341))
#define ASK64_ALPHA     (1./sqrt(1365))
#define ASK128_ALPHA    (1./sqrt(5461))
#define ASK256_ALPHA    (1./sqrt(21845))

// Macro    :   MODEM
//  MODEM   :   name-mangling macro
//  T       :   primitive data type
//  TC      :   primitive data type (complex)
#define LIQUID_MODEM_DEFINE_INTERNAL_API(MODEM,T,TC)            \
                                                                \
/* initialize a generic modem object */                         \
int MODEM(_init)(MODEM() _q, unsigned int _bits_per_symbol);    \
                                                                \
/* initialize symbol map for fast modulation */                 \
int MODEM(_init_map)(MODEM() _q);                               \
                                                                \
/* generic modem create routines */                             \
MODEM() MODEM(_create_ask)( unsigned int _bits_per_symbol);     \
MODEM() MODEM(_create_qam)( unsigned int _bits_per_symbol);     \
MODEM() MODEM(_create_psk)( unsigned int _bits_per_symbol);     \
MODEM() MODEM(_create_dpsk)(unsigned int _bits_per_symbol);     \
MODEM() MODEM(_create_apsk)(unsigned int _bits_per_symbol);     \
MODEM() MODEM(_create_arb)( unsigned int _bits_per_symbol);     \
                                                                \
/* Initialize arbitrary modem constellation */                  \
int MODEM(_arb_init)(MODEM()         _q,                        \
                      float complex * _symbol_map,              \
                      unsigned int    _len);                    \
                                                                \
/* Initialize arb modem constellation from external file */     \
int MODEM(_arb_init_file)(MODEM() _q, char * _filename);        \
                                                                \
/* specific modem create routines */                            \
MODEM() MODEM(_create_bpsk)(void);                              \
MODEM() MODEM(_create_qpsk)(void);                              \
MODEM() MODEM(_create_ook)(void);                               \
MODEM() MODEM(_create_sqam32)(void);                            \
MODEM() MODEM(_create_sqam128)(void);                           \
MODEM() MODEM(_create_V29)(void);                               \
MODEM() MODEM(_create_arb16opt)(void);                          \
MODEM() MODEM(_create_arb32opt)(void);                          \
MODEM() MODEM(_create_arb64opt)(void);                          \
MODEM() MODEM(_create_arb128opt)(void);                         \
MODEM() MODEM(_create_arb256opt)(void);                         \
MODEM() MODEM(_create_arb64vt)(void);                           \
MODEM() MODEM(_create_pi4dqpsk)(void);                          \
                                                                \
/* Scale arbitrary modem energy to unity */                     \
int MODEM(_arb_scale)(MODEM() _q);                              \
                                                                \
/* Balance I/Q */                                               \
int MODEM(_arb_balance_iq)(MODEM() _q);                         \
                                                                \
/* modulate using symbol map (look-up table) */                 \
int MODEM(_modulate_map)(MODEM()      _q,                       \
                          unsigned int _sym_in,                 \
                          TC *         _y);                     \
                                                                \
/* modem modulate routines */                                   \
int MODEM(_modulate_ask)      ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_qam)      ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_psk)      ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_dpsk)     ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_arb)      ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_apsk)     ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_bpsk)     ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_qpsk)     ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_ook)      ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_sqam32)   ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_sqam128)  ( MODEM(), unsigned int, TC *);   \
int MODEM(_modulate_pi4dqpsk) ( MODEM(), unsigned int, TC *);   \
                                                                \
/* modem demodulate routines */                                 \
int MODEM(_demodulate_ask)    ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_qam)    ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_psk)    ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_dpsk)   ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_arb)    ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_apsk)   ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_bpsk)   ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_qpsk)   ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_ook)    ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_sqam32) ( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_sqam128)( MODEM(), TC, unsigned int *);   \
int MODEM(_demodulate_pi4dqpsk)(MODEM(), TC, unsigned int *);   \
                                                                \
/* modem demodulate (soft) routines */                          \
int MODEM(_demodulate_soft_bpsk)(MODEM()         _q,            \
                                 TC              _x,            \
                                 unsigned int *  _sym_out,      \
                                 unsigned char * _soft_bits);   \
int MODEM(_demodulate_soft_qpsk)(MODEM()         _q,            \
                                 TC              _x,            \
                                 unsigned int *  _sym_out,      \
                                 unsigned char * _soft_bits);   \
int MODEM(_demodulate_soft_pi4dqpsk)(MODEM()         _q,        \
                                     TC              _x,        \
                                     unsigned int *  _sym_out,  \
                                     unsigned char* _soft_bits);\
int MODEM(_demodulate_soft_arb)( MODEM()         _q,            \
                                 TC              _x,            \
                                 unsigned int *  _sym_out,      \
                                 unsigned char * _soft_bits);   \
                                                                \
/* generate soft demodulation look-up table */                  \
int MODEM(_demodsoft_gentab)(MODEM()      _q,                   \
                              unsigned int _p);                 \
                                                                \
/* generic soft demodulation routine using nearest-neighbors */ \
/* look-up table                                             */ \
int MODEM(_demodulate_soft_table)(MODEM()         _q,           \
                                  TC              _x,           \
                                  unsigned int *  _sym_out,     \
                                  unsigned char * _soft_bits);  \
                                                                \
/* Demodulate a linear symbol constellation using           */  \
/* referenced lookup table                                  */  \
/*  _v      :   input value             */                      \
/*  _m      :   bits per symbol         */                      \
/*  _ref    :   array of thresholds     */                      \
/*  _s      :   demodulated symbol      */                      \
/*  _res    :   residual                */                      \
int MODEM(_demodulate_linear_array_ref)(T              _v,      \
                                        unsigned int   _m,      \
                                        T *            _ref,    \
                                        unsigned int * _s,      \
                                        T *            _res);   \



// define internal modem APIs
LIQUID_MODEM_DEFINE_INTERNAL_API(LIQUID_MODEM_MANGLE_FLOAT,float,float complex)

// APSK constants (container for apsk structure definitions)
struct liquid_apsk_s {
    modulation_scheme scheme;   // APSK modulation scheme
    unsigned int    num_levels; // number of rings
    unsigned int *  p;          // number of symbols per ring
    float *         r;          // radius of each ring
    float *         phi;        // phase offset of each ring
    float *         r_slicer;   // radius slicer
    unsigned char * map;        // symbol mapping
};

extern struct liquid_apsk_s liquid_apsk4;
extern struct liquid_apsk_s liquid_apsk8;
extern struct liquid_apsk_s liquid_apsk16;
extern struct liquid_apsk_s liquid_apsk32;
extern struct liquid_apsk_s liquid_apsk64;
extern struct liquid_apsk_s liquid_apsk128;
extern struct liquid_apsk_s liquid_apsk256;


// 'square' 32-QAM (first quadrant)
extern const float complex modem_arb_sqam32[8];

// 'square' 128-QAM (first quadrant)
extern const float complex modem_arb_sqam128[32];

// V.29 star constellation
extern const float complex modem_arb_V29[16];

// Virginia Tech logo
extern const float complex modem_arb_vt64[64];

// optimal QAM constellations
extern const float complex modem_arb16opt[16];
extern const float complex modem_arb32opt[32];
extern const float complex modem_arb64opt[64];
extern const float complex modem_arb128opt[128];
extern const float complex modem_arb256opt[256];


//
// MODULE : multichannel
//

// ofdm frame (common)

// generate short sequence symbols
//  _p      :   subcarrier allocation array
//  _M      :   total number of subcarriers
//  _S0     :   output symbol (freq)
//  _s0     :   output symbol (time)
//  _M_S0   :   total number of enabled subcarriers in S0
int ofdmframe_init_S0(unsigned char * _p,
                      unsigned int    _M,
                      float complex * _S0,
                      float complex * _s0,
                      unsigned int *  _M_S0);

// generate long sequence symbols
//  _p      :   subcarrier allocation array
//  _M      :   total number of subcarriers
//  _S1     :   output symbol (freq)
//  _s1     :   output symbol (time)
//  _M_S1   :   total number of enabled subcarriers in S1
int ofdmframe_init_S1(unsigned char * _p,
                      unsigned int    _M,
                      float complex * _S1,
                      float complex * _s1,
                      unsigned int *  _M_S1);

// 
// MODULE : nco (numerically-controlled oscillator)
//

// Numerically-controlled synthesizer (direct digital synthesis)
#define LIQUID_SYNTH_DEFINE_INTERNAL_API(SYNTH,T,TC)            \
                                                                \
/* constrain phase/frequency to be in [-pi,pi)          */      \
void SYNTH(_constrain_phase)(SYNTH() _q);                       \
void SYNTH(_constrain_frequency)(SYNTH() _q);                   \
void SYNTH(_compute_synth)(SYNTH() _q);                         \
                                                                \
/* reset internal phase-locked loop filter              */      \
void SYNTH(_pll_reset)(SYNTH() _q);                             \

// Define nco internal APIs
LIQUID_SYNTH_DEFINE_INTERNAL_API(SYNTH_MANGLE_FLOAT,
                                 float,
                                 liquid_float_complex)
// 
// MODULE : optim (non-linear optimization)
//

// optimization threshold switch
//  _u0         :   first utility
//  _u1         :   second utility
//  _minimize   :   minimize flag
//
// returns:
//  (_u0 > _u1) if (_minimize == 1)
//  (_u0 < _u1) otherwise
int optim_threshold_switch(float _u0,
                           float _u1,
                           int _minimize);

// compute the gradient of a function at a particular point
//  _utility    :   user-defined function
//  _userdata   :   user-defined data object
//  _x          :   operating point, [size: _n x 1]
//  _n          :   dimensionality of search
//  _delta      :   step value for which to compute gradient
//  _gradient   :   resulting gradient
void gradsearch_gradient(utility_function _utility,
                         void  *          _userdata,
                         float *          _x,
                         unsigned int     _n,
                         float            _delta,
                         float *          _gradient);

// execute line search; loosely solve:
//
//    min|max phi(alpha) := f(_x - alpha*_p)
//
// and return best guess at alpha that achieves this
//
//  _utility    :   user-defined function
//  _userdata   :   user-defined data object
//  _direction  :   search direction (e.g. LIQUID_OPTIM_MINIMIZE)
//  _n          :   dimensionality of search
//  _x          :   operating point, [size: _n x 1]
//  _p          :   normalized gradient, [size: _n x 1]
//  _alpha      :   initial step size
float gradsearch_linesearch(utility_function _utility,
                            void  *          _userdata,
                            int              _direction,
                            unsigned int     _n,
                            float *          _x,
                            float *          _p,
                            float            _alpha);

// normalize vector, returning its l2-norm
float gradsearch_norm(float *      _v,
                      unsigned int _n);


// Chromosome structure used in genetic algorithm searches
struct chromosome_s {
    unsigned int num_traits;            // number of represented traits
    unsigned int * bits_per_trait;      // bits to represent each trait
    unsigned long * max_value;          // maximum representable integer value
    unsigned long * traits;             // chromosome data

    unsigned int num_bits;              // total number of bits
};

struct gasearch_s {
    chromosome * population;            // population of chromosomes
    unsigned int population_size;       // size of the population
    unsigned int selection_size;        // number of 
    float mutation_rate;                // rate of mutation

    unsigned int num_parameters;        // number of parameters to optimize
    unsigned int bits_per_chromosome;   // total number of bits in each chromosome

    float *utility;                     // utility array
    unsigned int *rank;                 // rank indices of chromosomes (best to worst)

    chromosome c;                       // copy of best chromosome, optimal solution
    float utility_opt;                  // optimum utility (fitness of best solution)

    // External utility function.
    //
    // The success of a GA search algorithm is contingent upon the
    // design of a good utility function.  It should meet the following
    // criteria:
    //   - monotonically increasing (never flat)
    //   - efficient to compute
    //   - maps the [0,1] bounded output vector to desired range
    //   - for multiple objectives, utility should be high \em only when
    //         all objectives are met (multiplicative, not additive)
    gasearch_utility get_utility;       // utility function pointer
    void * userdata;                    // object to optimize
    int minimize;                       // minimize/maximize utility (search direction)
};

//
// gasearch internal methods
//

// evaluate fitness of entire population
int gasearch_evaluate(gasearch _q);

// crossover population
int gasearch_crossover(gasearch _q);

// mutate population
int gasearch_mutate(gasearch _q);

// rank population by fitness
int gasearch_rank(gasearch _q);

// sort values by index
//  _v          :   input values, [size: _len x 1]
//  _rank       :   output rank array (indices) [size: _len x 1]
//  _len        :   length of input array
//  _descending :   descending/ascending
void optim_sort(float *_v,
                unsigned int * _rank,
                unsigned int _len,
                int _descending);


//
// MODULE : random
//

#define randf_inline() ((float) rand() / (float) RAND_MAX)

float complex icrandnf();

// generate x ~ Gamma(delta,1)
float randgammaf_delta(float _delta);

// data scrambler masks
#define LIQUID_SCRAMBLE_MASK0   (0xb4)
#define LIQUID_SCRAMBLE_MASK1   (0x6a)
#define LIQUID_SCRAMBLE_MASK2   (0x8b)
#define LIQUID_SCRAMBLE_MASK3   (0xc5)

//
// MODULE : sequence
//


//
// MODULE : utility
//

// number of ones in a byte
//  0   0000 0000   :   0
//  1   0000 0001   :   1
//  2   0000 0010   :   1
//  3   0000 0011   :   2
//  4   0000 0100   :   1
//  ...
//  126 0111 1110   :   6
//  127 0111 1111   :   7
//  128 1000 0000   :   1
//  129 1000 0001   :   2
//  ...
//  253 1111 1101   :   7
//  254 1111 1110   :   7
//  255 1111 1111   :   8
extern const unsigned char liquid_c_ones[256];

// Count the number of ones in an integer, inline insertion
#define liquid_count_ones_uint16(x) (           \
    liquid_c_ones[  (x)      & 0xff ] +         \
    liquid_c_ones[ ((x)>>8)  & 0xff ])

#define liquid_count_ones_uint24(x) (           \
    liquid_c_ones[  (x)      & 0xff ] +         \
    liquid_c_ones[ ((x)>> 8) & 0xff ] +         \
    liquid_c_ones[ ((x)>>16) & 0xff ])

#define liquid_count_ones_uint32(x) (           \
    liquid_c_ones[  (x)      & 0xff ] +         \
    liquid_c_ones[ ((x)>> 8) & 0xff ] +         \
    liquid_c_ones[ ((x)>>16) & 0xff ] +         \
    liquid_c_ones[ ((x)>>24) & 0xff ])


// number of ones in a byte, modulo 2
//  0   0000 0000   :   0
//  1   0000 0001   :   1
//  2   0000 0010   :   1
//  3   0000 0011   :   0
//  4   0000 0100   :   1
//  ...
//  126 0111 1110   :   0
//  127 0111 1111   :   1
//  128 1000 0000   :   1
//  129 1000 0001   :   0
//  ...
//  253 1111 1101   :   1
//  254 1111 1110   :   1
//  255 1111 1111   :   0
extern const unsigned char liquid_c_ones_mod2[256];

// Count the number of ones in an integer modulo 2, inline insertion
#define liquid_count_ones_mod2_uint16(x) ((         \
    liquid_c_ones_mod2[  (x)      & 0xff ] +        \
    liquid_c_ones_mod2[ ((x)>>8)  & 0xff ]) % 2)

#define liquid_count_ones_mod2_uint32(x) ((         \
    liquid_c_ones_mod2[  (x)       & 0xff ] +       \
    liquid_c_ones_mod2[ ((x)>> 8)  & 0xff ] +       \
    liquid_c_ones_mod2[ ((x)>>16)  & 0xff ] +       \
    liquid_c_ones_mod2[ ((x)>>24)  & 0xff ]) % 2)

// compute binary dot-products (inline pre-processor macros)
#define liquid_bdotprod_uint8(x,y)  liquid_c_ones_mod2[(x)&(y)]
#define liquid_bdotprod_uint16(x,y) liquid_count_ones_mod2_uint16((x)&(y))
#define liquid_bdotprod_uint32(x,y) liquid_count_ones_mod2_uint32((x)&(y))

// number of leading zeros in byte
extern unsigned int liquid_c_leading_zeros[256];

// byte reversal and manipulation
extern const unsigned char liquid_reverse_byte_gentab[256];
#endif // __LIQUID_INTERNAL_H__

