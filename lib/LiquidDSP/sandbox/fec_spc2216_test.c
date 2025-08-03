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
// SEC-DED(22,16) product code test (soft decoding)
//  ________________ 
// [            |   ]
// [            |   ]
// [   16 x 16  | 6 ]
// [            |   ]
// [____________|___]
// [        6       ]
// [________________]
//
// input:   16 x 16 bits = 256 bits = 32 bytes
// output:  22 x 22 bits = 484 bits = 60 bytes + 4 bits (61 bytes)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "liquid.internal.h"

#define DEBUG_SPC2216 0

#define OUTPUT_FILENAME "spc2216_ber_test.m"

// encode message
//  _msg_org    :   original message [size: 32 bytes]
//  _msg_enc    :   encoded message  [size: 61 bytes]
void spc2216_encode(unsigned char * _msg_org,
                    unsigned char * _msg_enc);

// decode message
//  _msg_rec    :   received message [size: 61 bytes]
//  _msg_dec    :   decoded message  [size: 32 bytes]
void spc2216_decode(unsigned char * _msg_rec,
                    unsigned char * _msg_dec);

// pack message
//  _m  :   input message   [size: 32 bytes]
//  _pr :   row parities    [size: 16 bytes, @ 6 bits]
//  _pc :   col parities    [size: 22 bytes, @ 6 bits]
//  _v  :   output message  [size: 61 bytes]
void spc2216_pack(unsigned char * _m,
                  unsigned char * _pr,
                  unsigned char * _pc,
                  unsigned char * _v);

// unpack message
//  _v  :   input message   [size: 61 bytes]
//  _pr :   row parities    [size: 16 bytes, @ 6 bits]
//  _pc :   col parities    [size: 22 bytes, @ 6 bits]
//  _m  :   output message  [size: 32 bytes]
void spc2216_unpack(unsigned char * _v,
                    unsigned char * _pr,
                    unsigned char * _pc,
                    unsigned char * _m);

// transpose square message block (generic)
//  _m  :   input message block [size: _n x _n bits, _n*_n/8 bytes]
//  _n  :   matrix dimension (must be divisble by 8)
void spc2216_transpose_block(unsigned char * _m,
                             unsigned int    _n,
                             unsigned char * _mT);

// transpose message block and row parities
//  _m  :   message block       [size: 16 x 16 bits, 32 bytes]
//  _pr :   row parities        [size: 16 x  6 bits, 16 bytes]
//  _w  :   transposed array    [size: 22 x 16 bits, 44 bytes]
void spc2216_transpose_row(unsigned char * _m,
                           unsigned char * _pr,
                           unsigned char * _w);

// transpose message block and row parities (reverse)
//  _w  :   transposed array    [size: 22 x 16 bits, 44 bytes]
//  _m  :   message block       [size: 16 x 16 bits, 32 bytes]
//  _pr :   row parities        [size: 16 x  6 bits, 16 bytes]
void spc2216_transpose_col(unsigned char * _w,
                           unsigned char * _m,
                           unsigned char * _pr);

// print decoded block
void spc2216_print_decoded(unsigned char * _m);

// print encoded block
void spc2216_print_encoded(unsigned char * _v);

// print unpacked block
void spc2216_print_unpacked(unsigned char * _m,
                            unsigned char * _pr,
                            unsigned char * _pc);

// decode message (soft bits)
//  _msg_rec        :   received soft message [size: 488 soft bits]
//  _msg_dec_hard   :   decoded hard message  [size: 32 bytes]
void spc2216_decode_soft(unsigned char * _msg_rec,
                         unsigned char * _msg_dec_hard);

// decode soft symbol
//  _msg_rec        :   received soft bits [size: 22 soft bits]
//  _msg_dec        :   decoded soft bits [size: 22 soft bits]
//  _msg_dec_hard   :   decoded hard bits [size: 32 bytes]
int spc2216_decode_sym_soft(unsigned char * _msg_rec,
                            unsigned char * _msg_dec);

void print_bitstring(unsigned char _x,
                     unsigned char _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        printf("%2s", ((_x >> (_n-i-1)) & 1) ? "1" : ".");
}

int main(int argc, char*argv[])
{
    srand(time(NULL));
    unsigned int i;
    
    // error vector
    unsigned char e[61];
#if 0
    for (i=0; i<61; i++)
        e[i] = (rand() % 8) == 0 ? 1 << rand() % 7 : 0;
#else
    memset(e, 0x00, 61);
    e[3] = 0x08;
#endif

    // original message [16 x 16 bits], 32 bytes
    unsigned char m[32];

    // derived values
    unsigned char v[61];    // encoded/transmitted message
    unsigned char r[61];    // received vector
    unsigned char m_hat[32];// estimated original message

    // generate random transmitted message
#if 0
    for (i=0; i<32; i++)
        m[i] = rand() & 0xff;
#else
    for (i=0; i<32; i++) {
        if (i < 16 && (i%2)==0)
            m[i] = 0xff >> (i/2); //1 << 7-(i/2);
        else if (i >= 16 && (i%2)==1)
            m[i] = 1 << (7-((i-16-1)/2));
        else
            m[i] = 0x00;
    }
#endif
    printf("m (original message):\n");
    spc2216_print_decoded(m);

    // encode
    spc2216_encode(m, v);
    printf("v (encoded message):\n");
    spc2216_print_encoded(v);

    printf("e (error message):\n");
    spc2216_print_encoded(e);

    // add errors
    for (i=0; i<61; i++)
        r[i] = v[i] ^ e[i];
    printf("r (received message):\n");
    spc2216_print_encoded(r);

    // decode
    spc2216_decode(r, m_hat);

    // compute errors between m, m_hat
    unsigned int num_errors_decoded = count_bit_errors_array(m, m_hat, 32);
    printf("decoding errors (original) : %2u / 256\n", num_errors_decoded);

    //
    // test soft decoding
    //
#if 1
    unsigned char msg_rec_soft[8*61];
    unsigned char soft0 =  64;
    unsigned char soft1 = 192;

    // modulate with BPSK
    for (i=0; i<61; i++) {
        msg_rec_soft[8*i+0] = (r[i] & 0x80) ? soft1 : soft0;
        msg_rec_soft[8*i+1] = (r[i] & 0x40) ? soft1 : soft0;
        msg_rec_soft[8*i+2] = (r[i] & 0x20) ? soft1 : soft0;
        msg_rec_soft[8*i+3] = (r[i] & 0x10) ? soft1 : soft0;
        msg_rec_soft[8*i+4] = (r[i] & 0x08) ? soft1 : soft0;
        msg_rec_soft[8*i+5] = (r[i] & 0x04) ? soft1 : soft0;
        msg_rec_soft[8*i+6] = (r[i] & 0x02) ? soft1 : soft0;
        msg_rec_soft[8*i+7] = (r[i] & 0x01) ? soft1 : soft0;
    }

    // add 'noise'...
    for (i=0; i<6*61; i++) {
        int LLR = msg_rec_soft[i] + 30*randnf();
        if (LLR <   0) LLR =   0;
        if (LLR > 255) LLR = 255;
        msg_rec_soft[i] = LLR;
    }

    // test soft-decision decoding
    memset(m_hat, 0x00, 32);
    spc2216_decode_soft(msg_rec_soft, m_hat);

    // compute errors between m, m_hat
    num_errors_decoded = count_bit_errors_array(m, m_hat, 32);
    printf("soft decoding errors (original) : %2u / 256\n", num_errors_decoded);
#else
    // 1 1 . . 1 1 |. . 1 1 1 1 1 1 . . . . . . . .
    unsigned char msg_rec_soft[22] = {
        255, 255, 0,   0,   255, 255,
        0,   0,   255, 255, 255, 255, 255, 255,
        0,   0,   0,   0,   0,   0,   0,   0,
    };

    // add noise
    for (i=0; i<22; i++) {
        int bit = msg_rec_soft[i] > 128 ? 192 : 64;
        bit += 30*randnf();
        if (bit < 0)   bit = 0;
        if (bit > 255) bit = 255;
        msg_rec_soft[i] = (unsigned char)bit;
    }
    int rc = spc2216_decode_sym_soft(msg_rec_soft, m_hat);
#endif

    // 
    // run SNR trials
    //
    float SNRdB_min = -5.0f;                // signal-to-noise ratio (minimum)
    float SNRdB_max = 10.0f;                // signal-to-noise ratio (maximum)
    unsigned int num_snr = 31;              // number of SNR steps
    unsigned int num_trials=10000;          // number of trials

    // arrays
    float complex sym_rec[8*61];            // received BPSK symbols
    unsigned int bit_errors_hard[num_snr];
    unsigned int bit_errors_soft[num_snr];

    unsigned int n_enc = 61;
    unsigned char msg_org[32];
    unsigned char msg_enc[61];
    unsigned char msg_cor[61];      // corrupted message (hard bits)
    unsigned char msg_LLR[8*61];    // corrupted message (soft-bit log-likelihood ratio)
    unsigned char msg_dec_hard[32];
    unsigned char msg_dec_soft[32];

    // set up parameters
    float SNRdB_step = (SNRdB_max - SNRdB_min) / (num_snr-1);

    printf("  %8s %8s [%8s] %8s %12s %8s %12s %12s\n",
            "SNR [dB]", "Eb/N0", "trials", "hard", "(hard BER)", "soft", "(soft BER)", "uncoded");
    unsigned int s;
    for (s=0; s<num_snr; s++) {
        // compute SNR for this level
        float SNRdB = SNRdB_min + s*SNRdB_step; // SNR in dB for this round
        float nstd = powf(10.0f, -SNRdB/20.0f); // noise standard deviation

        // reset results
        bit_errors_hard[s] = 0;
        bit_errors_soft[s] = 0;

        unsigned int t;
        for (t=0; t<num_trials; t++) {
            // generate data
            for (i=0; i<32; i++)
                msg_org[i] = rand() & 0xff;

            // encode message
            spc2216_encode(msg_org, msg_enc);

            // modulate with BPSK
            for (i=0; i<n_enc; i++) {
                sym_rec[8*i+0] = (msg_enc[i] & 0x80) ? 1.0f : -1.0f;
                sym_rec[8*i+1] = (msg_enc[i] & 0x40) ? 1.0f : -1.0f;
                sym_rec[8*i+2] = (msg_enc[i] & 0x20) ? 1.0f : -1.0f;
                sym_rec[8*i+3] = (msg_enc[i] & 0x10) ? 1.0f : -1.0f;
                sym_rec[8*i+4] = (msg_enc[i] & 0x08) ? 1.0f : -1.0f;
                sym_rec[8*i+5] = (msg_enc[i] & 0x04) ? 1.0f : -1.0f;
                sym_rec[8*i+6] = (msg_enc[i] & 0x02) ? 1.0f : -1.0f;
                sym_rec[8*i+7] = (msg_enc[i] & 0x01) ? 1.0f : -1.0f;
            }

            // add noise
            for (i=0; i<8*n_enc; i++)
                sym_rec[i] += nstd*(randnf() + _Complex_I*randf())*M_SQRT1_2;

            // convert to hard bits (hard decoding)
            for (i=0; i<n_enc; i++) {
                msg_cor[i] = 0x00;

                msg_cor[i] |= crealf(sym_rec[8*i+0]) > 0.0f ? 0x80 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+1]) > 0.0f ? 0x40 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+2]) > 0.0f ? 0x20 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+3]) > 0.0f ? 0x10 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+4]) > 0.0f ? 0x08 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+5]) > 0.0f ? 0x04 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+6]) > 0.0f ? 0x02 : 0x00;
                msg_cor[i] |= crealf(sym_rec[8*i+7]) > 0.0f ? 0x01 : 0x00;
            }
            spc2216_decode(msg_cor, msg_dec_hard);

            // convert to approximate LLR (soft decoding)
            for (i=0; i<8*n_enc; i++) {
                int LLR = (int)( 256*(crealf(sym_rec[i])*0.5f + 0.5f) );
                if (LLR < 0)   LLR = 0;
                if (LLR > 255) LLR = 255;
                msg_LLR[i] = (unsigned char) LLR;
            }
            spc2216_decode_soft(msg_LLR, msg_dec_soft);
            
            // tabulate results
            bit_errors_hard[s] += count_bit_errors_array(msg_org, msg_dec_hard, 32);
            bit_errors_soft[s] += count_bit_errors_array(msg_org, msg_dec_soft, 32);
        }

        // print results for this SNR step
        float rate = 32. / 61.; // true rate
        printf("  %8.3f %8.3f [%8u] %8u %12.4e %8u %12.4e %12.4e\n",
                SNRdB,
                SNRdB - 10*log10f(rate),
                8*32*num_trials,
                bit_errors_hard[s], (float)(bit_errors_hard[s]) / (float)(num_trials*32*8),
                bit_errors_soft[s], (float)(bit_errors_soft[s]) / (float)(num_trials*32*8),
                0.5f*erfcf(1.0f/nstd));
    }
    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"\n\n");
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"n = %u;    %% frame size [bytes]\n", 32);
    fprintf(fid,"k = %u;    %% encoded frame size [bytes]\n", 61);
    fprintf(fid,"r = n / k; %% true rate\n");
    fprintf(fid,"num_snr = %u;\n", num_snr);
    fprintf(fid,"num_trials = %u;\n", num_trials);
    fprintf(fid,"num_bit_trials = num_trials*n*8;\n");
    fprintf(fid,"bit_errors_hard = zeros(1,num_snr);\n");
    fprintf(fid,"bit_errors_soft = zeros(1,num_snr);\n");
    for (i=0; i<num_snr; i++) {
        fprintf(fid,"SNRdB(%4u) = %12.8f;\n",i+1, SNRdB_min + i*SNRdB_step);
        fprintf(fid,"bit_errors_hard(%6u) = %u;\n", i+1, bit_errors_hard[i]);
        fprintf(fid,"bit_errors_soft(%6u) = %u;\n", i+1, bit_errors_soft[i]);
    }
    fprintf(fid,"EbN0dB = SNRdB - 10*log10(r);\n");
    fprintf(fid,"EbN0dB_bpsk = -15:0.5:40;\n");
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(EbN0dB_bpsk, 0.5*erfc(sqrt(10.^[EbN0dB_bpsk/10]))+1e-12,'-x',\n");
    fprintf(fid,"         EbN0dB,      bit_errors_hard / num_bit_trials + 1e-12,  '-x',\n");
    fprintf(fid,"         EbN0dB,      bit_errors_soft / num_bit_trials + 1e-12,  '-x');\n");
    fprintf(fid,"axis([%f (%f-10*log10(r)) 1e-6 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"legend('uncoded','hard','soft');\n");
    fprintf(fid,"xlabel('E_b/N_0 [dB]');\n");
    fprintf(fid,"ylabel('Bit Error Rate');\n");
    fprintf(fid,"title('BER vs. E_b/N_0 for SPC(22,16)');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

void spc2216_encode(unsigned char * _msg_org,
                    unsigned char * _msg_enc)
{
    unsigned int i;

    unsigned char parity_row[16];
    unsigned char parity_col[22];

    // compute row parities
    for (i=0; i<16; i++)
        parity_row[i] = fec_secded2216_compute_parity(&_msg_org[2*i]);

    // compute column parities
    unsigned char w[44];
    spc2216_transpose_row(_msg_org, parity_row, w);
    for (i=0; i<22; i++)
        parity_col[i] = fec_secded2216_compute_parity(&w[2*i]);

    // pack encoded message
    spc2216_pack(_msg_org, parity_row, parity_col, _msg_enc);
}

void spc2216_decode(unsigned char * _msg_rec,
                    unsigned char * _msg_dec)
{
    unsigned char parity_row[16];
    unsigned char parity_col[22];
    unsigned char m_hat[32];
    unsigned char w[44];

    // unpack encoded message
    spc2216_unpack(_msg_rec, parity_row, parity_col, m_hat);

    // decode
    unsigned int i;
    unsigned char sym_enc[3];   // encoded 22-bit message
    unsigned char e_hat[3];     // estimated error vector
    
    // transpose
    spc2216_transpose_row(m_hat, parity_row, w);

    // compute syndromes on columns and decode
    for (i=0; i<22; i++) {
        sym_enc[0] = parity_col[i];
        sym_enc[1] = w[2*i+0];
        sym_enc[2] = w[2*i+1];

#if DEBUG_SPC2216
        int syndrome_flag = fec_secded2216_estimate_ehat(sym_enc, e_hat);
#else
        // same as above, but do not declare 'syndrome_flag' variable
        // (rid compiler of unused variable warning)
        fec_secded2216_estimate_ehat(sym_enc, e_hat);
#endif

        // apply error vector estimate to appropriate arrays
        parity_col[i] ^= e_hat[0];
        w[2*i+0]      ^= e_hat[1];
        w[2*i+1]      ^= e_hat[2];

#if DEBUG_SPC2216
        // print encoded symbol
        printf("%3u:", i);
        print_bitstring(w[2*i+0], 8);
        print_bitstring(w[2*i+1], 8);
        printf(" |");
        print_bitstring(parity_col[i], 6);
        if (syndrome_flag == 0) {
            printf(" (no errors detected)\n");
        } else if (syndrome_flag == 1) {
            printf(" (one error detected and corrected!)\n");
        } else {
            printf(" (multiple errors detected)\n");
        }
#endif
    }
#if DEBUG_SPC2216
    printf("******** transposed: **********\n");
    for (i=0; i<22; i++) {
        printf("    ");
        print_bitstring(w[2*i+0], 8);
        print_bitstring(w[2*i+1], 8);
        printf("\n");
        if (i==15)
            printf("    ---------------------------------\n");
    }
#endif

    // transpose back
    spc2216_transpose_col(w, m_hat, parity_row);

    // compute syndromes on rows and decode
#if DEBUG_SPC2216
    unsigned int num_uncorrected_errors = 0;
#endif
    for (i=0; i<16; i++) {
        sym_enc[0] = parity_row[i];
        sym_enc[1] = m_hat[2*i+0];
        sym_enc[2] = m_hat[2*i+1];
#if DEBUG_SPC2216
        int syndrome_flag = fec_secded2216_estimate_ehat(sym_enc, e_hat);
        if (syndrome_flag == 0) {
            printf("%3u : no errors detected\n", i);
        } else if (syndrome_flag == 1) {
            printf("%3u : one error detected and corrected!\n", i);
        } else {
            printf("%3u : multiple errors detected\n", i);
        }

        if (syndrome_flag == 2)
            num_uncorrected_errors++;
#endif

        // apply error vector estimate to appropriate arrays
        parity_col[i] ^= e_hat[0];
        m_hat[2*i+0]  ^= e_hat[1];
        m_hat[2*i+1]  ^= e_hat[2];
    }

#if DEBUG_SPC2216
    printf("number of uncorrected errors: %u\n", num_uncorrected_errors);
#endif
    
    // copy decoded message to output
    memmove(_msg_dec, m_hat, 32*sizeof(unsigned char));
}

// pack message
//  _m  :   input message   [size: 32 bytes]
//  _pr :   row parities    [size: 16 bytes, @ 6 bits]
//  _pc :   col parities    [size: 22 bytes, @ 6 bits]
//  _v  :   output message  [size: 61 bytes]
void spc2216_pack(unsigned char * _m,
                  unsigned char * _pr,
                  unsigned char * _pc,
                  unsigned char * _v)
{
    // copy input message to beginning of encoded vector
    memmove(_v, _m, 32*sizeof(unsigned char));

    // pack row parities
    unsigned int i;
    unsigned int k=0;
    for (i=0; i<16; i++) {
        liquid_pack_array(&_v[32], 29, k, 6, _pr[i]);
        k += 6;
    }

    // pack column parities
    for (i=0; i<22; i++) {
        liquid_pack_array(&_v[32], 29, k, 6, _pc[i]);
        k += 6;
    }
}

// unpack message
//  _v  :   input message   [size: 61 bytes]
//  _pr :   row parities    [size: 16 bytes, @ 6 bits]
//  _pc :   col parities    [size: 22 bytes, @ 6 bits]
//  _m  :   output message  [size: 32 bytes]
void spc2216_unpack(unsigned char * _v,
                    unsigned char * _pr,
                    unsigned char * _pc,
                    unsigned char * _m)
{
    // copy input message to beginning of encoded vector
    memmove(_m, _v, 32*sizeof(unsigned char));

    // unpack row parities
    unsigned int i;
    unsigned int k=0;
    for (i=0; i<16; i++) {
        //liquid_unpack_array(&_pr[k], &_v[32*8+k], 
        liquid_unpack_array(&_v[32], 29, k, 6, &_pr[i]);
        k += 6;
    }

    // unpack column parities
    for (i=0; i<22; i++) {
        //liquid_unpack_array(&_pr[k], &_v[32*8+k], 
        liquid_unpack_array(&_v[32], 29, k, 6, &_pc[i]);
        k += 6;
    }
}

// transpose message block and row parities
//  _m  :   message block       [size: 16 x 16 bits, 32 bytes]
//  _pr :   row parities        [size: 16 x  6 bits, 16 bytes]
//  _w  :   transposed array    [size: 22 x 16 bits, 44 bytes]
void spc2216_transpose_row(unsigned char * _m,
                           unsigned char * _pr,
                           unsigned char * _w)
{
    // transpose main input message, store in first 32
    // bytes of _w array
    spc2216_transpose_block(_m, 16, _w);

    // transpose row parities
    unsigned int i;
    unsigned int j;
    for (i=0; i<6; i++) {
        unsigned char mask = 1 << (6-i-1);
        unsigned char w0 = 0;
        unsigned char w1 = 0;
        for (j=0; j<8; j++) {
            w0 |= (_pr[j  ] & mask) ? 1 << (8-j-1) : 0;
            w1 |= (_pr[j+8] & mask) ? 1 << (8-j-1) : 0;
        }

        _w[32 + 2*i + 0] = w0;
        _w[32 + 2*i + 1] = w1;
    }

#if 0
    printf("transposed:\n");
    for (i=0; i<22; i++) {
        printf("    ");
        print_bitstring(_w[2*i+0], 8);
        print_bitstring(_w[2*i+1], 8);
        printf("\n");
        if (i==15)
            printf("    ---------------------------------\n");
    }
#endif
}

// transpose message block and row parities (reverse)
//  _w  :   transposed array    [size: 22 x 16 bits, 44 bytes]
//  _m  :   message block       [size: 16 x 16 bits, 32 bytes]
//  _pr :   row parities        [size: 16 x  6 bits, 16 bytes]
void spc2216_transpose_col(unsigned char * _w,
                           unsigned char * _m,
                           unsigned char * _pr)
{
    // transpose main message block from first 32
    // bytes of _w array
    spc2216_transpose_block(_w, 16, _m);

    // transpose row parities
    unsigned int i; // input row
    unsigned int j; // input column (rem 8)

    // initialize to zero
    for (i=0; i<16; i++)
        _pr[i] = 0;

    for (i=0; i<6; i++) {
        unsigned char mask = 1 << (6-i-1);
        for (j=0; j<8; j++) {
            _pr[j  ] |= (_w[32 + 2*i + 0] & (1 << (8-j-1)) ) ? mask : 0;
            _pr[j+8] |= (_w[32 + 2*i + 1] & (1 << (8-j-1)) ) ? mask : 0;
        }
    }

#if 0
    printf("transposed (reverse):\n");
    for (i=0; i<16; i++) {
        printf("    ");
        print_bitstring(_m[2*i+0], 8);
        print_bitstring(_m[2*i+1], 8);
        printf(" ");
        print_bitstring(_pr[i], 6);
        printf("\n");
    }
#endif
}


// transpose square message block (generic)
//  _m  :   input message block [size: _n x _n bits, _n*_n/8 bytes]
//  _n  :   matrix dimension (must be divisble by 8)
void spc2216_transpose_block(unsigned char * _m,
                             unsigned int    _n,
                             unsigned char * _mT)
{
    unsigned int i;
    unsigned int j;
    unsigned int k;

    // ensure that _n is divisible by 8
    if (_n % 8) {
        fprintf(stderr,"error: spc2216_transpose_block(), number of rows must be divisible by 8\n");
        exit(1);
    }
    unsigned int c = _n / 8;    // number of byte columns
    
    unsigned char w0;
    unsigned char w1;
    for (k=0; k<c; k++) {
        for (i=0; i<8; i++) {
            unsigned char mask = 1 << (8-i-1);
            w0 = 0;
            w1 = 0;
            for (j=0; j<8; j++) {
                w0 |= (_m[2*j + c*8*k + 0] & mask) ? 1 << (8-j-1) : 0;
                w1 |= (_m[2*j + c*8*k + 1] & mask) ? 1 << (8-j-1) : 0;
            }

            _mT[2*i       + k] = w0;
            _mT[2*i + c*8 + k] = w1;
        }
    }
}

// print decoded block
void spc2216_print_decoded(unsigned char * _m)
{
    unsigned int i;
    
    for (i=0; i<16; i++) {
        printf("    ");
        print_bitstring(_m[2*i+0],8);
        print_bitstring(_m[2*i+1],8);
        printf("\n");
    }
}

// print encoded block
void spc2216_print_encoded(unsigned char * _v)
{
    unsigned char parity_row[16];
    unsigned char parity_col[22];
    unsigned char m[32];

    // unpack encoded message
    spc2216_unpack(_v, parity_row, parity_col, m);

    // print unpacked version
#if 0
    spc2216_print_decoded(m);
    
    unsigned int i;
    printf("rows:\n");
    for (i=0; i<16; i++) {
        printf("  %3u : ", i);
        print_bitstring(parity_row[i], 6);
        printf("\n");
    }
    printf("cols:\n");
    for (i=0; i<22; i++) {
        printf("  %3u : ", i);
        print_bitstring(parity_col[i], 6);
        printf("\n");
    }
#else
    spc2216_print_unpacked(m, parity_row, parity_col);
#endif
}

// print unpacked block
void spc2216_print_unpacked(unsigned char * _m,
                            unsigned char * _pr,
                            unsigned char * _pc)
{
    unsigned int i;
    
    for (i=0; i<16; i++) {
        printf("    ");
        print_bitstring(_m[2*i+0],8);
        print_bitstring(_m[2*i+1],8);
        printf(" |");
        print_bitstring(_pr[i], 6);
        printf("\n");
    }
    printf("    ---------------------------------+-------------\n");

    // print column parities
    unsigned int j;
    for (j=0; j<6; j++) {
        printf("    ");
        for (i=0; i<22; i++) {
            printf("%2s", ((_pc[i] >> (6-j-1)) & 0x01) ? "1" : ".");

            if (i==15) printf("  ");
        }
        printf("\n");
    }
}

// decode message (soft bits)
//  _msg_rec        :   received soft message [size: 488 soft bits]
//  _msg_dec_hard   :   decoded hard message  [size: 32 bytes]
void spc2216_decode_soft(unsigned char * _msg_rec,
                         unsigned char * _msg_dec_hard)
{
    unsigned char w[256];           // 256 = 16 x 16 bits
    unsigned char parity_row[96];   //  96 = 16 rows @ 6 bits
    unsigned char parity_col[132];  // 132 = 22 cols @ 6 bits
    
    // unpack encoded soft bits:
    memmove(w,           _msg_rec,         256);
    memmove(parity_row, &_msg_rec[256],     96);
    memmove(parity_col, &_msg_rec[256+96], 132);

    unsigned int i;
    unsigned int j;
#if 0
    // print...
    for (i=0; i<16; i++) {
        printf("    ");
        for (j=0; j<16; j++)
            printf("%2s", w[16*i+j] > 128 ? "1" : ".");
        printf(" |");
        for (j=0; j<6; j++)
            printf("%2s", parity_row[6*i+j] > 128 ? "1" : ".");
        printf("\n");
    }
    printf("    ---------------------------------+-------------\n");
    for (i=0; i<6; i++) {
        printf("    ");
        for (j=0; j<22; j++)
            printf("%2s", parity_col[6*j + i] > 128 ? "1" : ".");
        printf("\n");
    }
#endif

    // 
    unsigned char sym_rec[22];  // encoded 22-bit message
    unsigned char sym_dec[22];  // decoded 22-bit message
    int syndrome_flag;

    // TODO : run multiple iterations...
    unsigned int n;
    for (n=0; n<6; n++) {
#if DEBUG_SPC2216
        printf("\n");
        // print soft values
        for (i=0; i<16; i++) {
            for (j=0; j<16; j++)
                printf("%3u ", w[16*i+j]);
            printf("\n");
        }
#endif
        // compute syndromes on columns and run soft decoder
        //printf("columns (w):\n");
        for (i=0; i<16; i++) {
            // extract encoded symbol
            for (j=0; j<6; j++)  sym_rec[j  ] = parity_col[j + 6*i];
            for (j=0; j<16; j++) sym_rec[j+6] = w[16*j + i];

            // run soft decoder
            syndrome_flag = spc2216_decode_sym_soft(sym_rec, sym_dec);

            // replace decoded symbol
            for (j=0; j<6; j++)  parity_col[j + 6*i] = sym_dec[j];
            for (j=0; j<16; j++) w[16*j + i]         = sym_dec[j+6];
        }

        // compute syndromes on row parities and run soft decoder
        //printf("row parities:\n");
        for (i=0; i<6; i++) {
            // extract encoded symbol
            for (j=0; j<6; j++)  sym_rec[j  ] = parity_col[j + 6*(i+16)];
            for (j=0; j<16; j++) sym_rec[j+6] = parity_row[6*j + i];

            // run soft decoder
            syndrome_flag = spc2216_decode_sym_soft(sym_rec, sym_dec);

            // replace decoded symbol
            for (j=0; j<6; j++)  parity_col[j + 6*(i+16)] = sym_rec[j];
            for (j=0; j<16; j++) parity_row[6*j + i]      = sym_rec[j+6];
        }

#if DEBUG_SPC2216
        printf("...\n");
        // print soft values
        for (i=0; i<16; i++) {
            for (j=0; j<16; j++)
                printf("%3u ", w[16*i+j]);
            printf("\n");
        }
#endif
        // compute syndromes on rows and run soft decoder
        //printf("rows:\n");
        unsigned int num_errors = 0;
        for (i=0; i<16; i++) {
            // extract encoded symbol
            for (j=0; j<6; j++)  sym_rec[j  ] = parity_row[6*i + j];
            for (j=0; j<16; j++) sym_rec[j+6] = w[16*i + j];

            // run soft decoder
            syndrome_flag = spc2216_decode_sym_soft(sym_rec, sym_dec);
            num_errors += syndrome_flag == 0 ? 0 : 1;

            // replace decoded symbol
            for (j=0; j<6; j++)  parity_row[6*i + j] = sym_rec[j];
            for (j=0; j<16; j++) w[16*i + j]         = sym_rec[j+6];
        }

        //printf("%3u, detected %u soft decoding errors\n", n, num_errors);
        if (num_errors == 0)
            break;
    }

    // hard-decision decoding
    for (i=0; i<16; i++) {
        _msg_dec_hard[2*i+0] = 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 0] > 128 ? 0x80 : 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 1] > 128 ? 0x40 : 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 2] > 128 ? 0x20 : 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 3] > 128 ? 0x10 : 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 4] > 128 ? 0x08 : 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 5] > 128 ? 0x04 : 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 6] > 128 ? 0x02 : 0;
        _msg_dec_hard[2*i+0] |= w[16*i+ 7] > 128 ? 0x01 : 0;

        _msg_dec_hard[2*i+1] = 0;
        _msg_dec_hard[2*i+1] |= w[16*i+ 8] > 128 ? 0x80 : 0;
        _msg_dec_hard[2*i+1] |= w[16*i+ 9] > 128 ? 0x40 : 0;
        _msg_dec_hard[2*i+1] |= w[16*i+10] > 128 ? 0x20 : 0;
        _msg_dec_hard[2*i+1] |= w[16*i+11] > 128 ? 0x10 : 0;
        _msg_dec_hard[2*i+1] |= w[16*i+12] > 128 ? 0x08 : 0;
        _msg_dec_hard[2*i+1] |= w[16*i+13] > 128 ? 0x04 : 0;
        _msg_dec_hard[2*i+1] |= w[16*i+14] > 128 ? 0x02 : 0;
        _msg_dec_hard[2*i+1] |= w[16*i+15] > 128 ? 0x01 : 0;
    }

    // print decoded block
    //spc2216_print_decoded(_msg_dec_hard);
}

// decode soft symbol
//  _msg_rec    :   received soft bits [size: 22 soft bits]
//  _msg_dec    :   decoded soft bits [size: 22 soft bits]
//  return value:   syndrome flag
int spc2216_decode_sym_soft(unsigned char * _msg_rec,
                            unsigned char * _msg_dec)
{
    // pack...
    unsigned char sym_enc[3] = {0, 0, 0};
    unsigned char e_hat[3];
    unsigned int i;

#if DEBUG_SPC2216
    printf(" msg_rec:");
    for (i=0; i<22; i++)
        printf("%3u,", _msg_rec[i]);
    printf("\n");
#endif

    for (i=0; i<6; i++) {
        sym_enc[0] <<= 1;
        sym_enc[0] |= _msg_rec[i] > 128 ? 1 : 0;
    }

    for (i=0; i<8; i++) {
        sym_enc[1] <<= 1;
        sym_enc[2] <<= 1;
        sym_enc[1] |= _msg_rec[ 6 + i] > 128 ? 1 : 0;
        sym_enc[2] |= _msg_rec[14 + i] > 128 ? 1 : 0;
    }
        
    int syndrome_flag = fec_secded2216_estimate_ehat(sym_enc, e_hat);

#if DEBUG_SPC2216
    if (syndrome_flag == 0) {
        printf(" (no errors detected)\n");
    } else if (syndrome_flag == 1) {
        printf(" (one error detected and corrected!)\n");
    } else {
        printf(" (multiple errors detected)\n");
    }

    printf(" input  : ");
    print_bitstring(sym_enc[0], 6);
    printf("|");
    print_bitstring(sym_enc[1], 8);
    print_bitstring(sym_enc[2], 8);
    printf("\n");
#endif

    // apply error vector estimate to appropriate arrays
    sym_enc[0] ^= e_hat[0];
    sym_enc[1] ^= e_hat[1];
    sym_enc[2] ^= e_hat[2];
    
#if DEBUG_SPC2216
    printf(" output : ");
    print_bitstring(sym_enc[0], 6);
    printf("|");
    print_bitstring(sym_enc[1], 8);
    print_bitstring(sym_enc[2], 8);
    printf("\n");
#endif

    // unpack...
    unsigned char sym_dec_soft[22];
    for (i=0; i<6; i++) {
        sym_dec_soft[i] = sym_enc[0] & (1 << (6-i-1)) ? 255 : 0;
    }
    for (i=0; i<8; i++) {
        sym_dec_soft[ 6 + i] = sym_enc[1] & (1 << (8-i-1)) ? 255 : 0;
        sym_dec_soft[14 + i] = sym_enc[2] & (1 << (8-i-1)) ? 255 : 0;
    }

    // combine...
    for (i=0; i<22; i++) {
        int delta = (int)sym_dec_soft[i] - 128;
        //_msg_dec[i] = 0.5*_msg_rec[i] + 0.5*sym_dec_soft[i];

        int bit = _msg_rec[i] + 0.2*delta;
        if (bit <   0) bit =   0;
        if (bit > 255) bit = 255;
        _msg_dec[i] = (unsigned char) bit;
    }

#if DEBUG_SPC2216
    printf(" msg_dec:");
    for (i=0; i<22; i++)
        printf("%3u,", _msg_dec[i]);
    printf("\n");
#endif
    
    return syndrome_flag;
}

